*This project has been created as part of the 42 curriculum by dgeara.*

# pipex

> Reproducing the shell pipe operator in C — `< file1 cmd1 | cmd2 > file2`

---

## Overview

**pipex** is a 42 curriculum project about Unix pipes, written entirely in C.

It mimics the behavior of the shell pipe operator by reading from an input file, passing data through two commands connected by a pipe, and writing the result to an output file — exactly like:

```bash
< file1 cmd1 | cmd2 > file2
```

The real challenge is understanding how Unix processes work under the hood: how `fork()` duplicates a process, how `execve()` replaces it with a new program, how `pipe()` creates a communication channel between two processes, and how `dup2()` rewires file descriptors so commands read and write to the right place without knowing it.

---

## Usage

```bash
./pipex file1 "cmd1" "cmd2" file2
```

### Examples

```bash
./pipex input.txt "cat" "grep hello" output.txt
./pipex input.txt "ls -la" "wc -l" output.txt
./pipex input.txt "/bin/cat" "grep hello" output.txt
```

---

## Compilation

```bash
make
make clean    # remove object files
make fclean   # remove object files + binary
make re       # fclean + make
```

---

## Features

- Creates a pipe between two child processes
- Child 1 reads from `file1`, executes `cmd1`, writes output into the pipe
- Child 2 reads from the pipe, executes `cmd2`, writes output to `file2`
- Automatically resolves command paths by searching through `PATH`
- Supports absolute (`/bin/cat`) and relative (`./myscript`) paths
- Error handling: missing file, insufficient permissions, command not found
- No memory leaks

---

## How It Works

### `pipe()`

Creates two file descriptors:
- `fd[0]` — read end
- `fd[1]` — write end

Anything written to `fd[1]` can be read from `fd[0]`. It's a one-way channel between two processes.

### `fork()`

Duplicates the current process — memory, variables, open file descriptors, everything. The only difference between parent and child is the return value:

```
fork() returns 0    → I am the child
fork() returns > 0  → I am the parent (value = child's pid)
fork() returns < 0  → error
```

pipex calls `fork()` twice — once per command. Each child receives a copy of the pipe's file descriptors.

### `dup2()`

`dup2(old, new)` makes `fd new` point to the same destination as `fd old`. This is how redirections work:

```c
dup2(fd_in, STDIN_FILENO);          // stdin now reads from file1
dup2(pipe_fds[1], STDOUT_FILENO);   // stdout now writes into the pipe
```

After `dup2`, the command doesn't know it's reading a file or writing to a pipe — it simply uses stdin and stdout as usual.

### `execve()`

Doesn't launch a program alongside you — it **replaces** your process entirely. Code, stack, memory — everything disappears, replaced by the new program. File descriptors survive (which is why `dup2` must be called before it).

If `execve` returns, it failed.

### Why `fork` and not just `execve` directly?

Without `fork`, the first `execve` would replace your process and the second command would never run. And even if that weren't the case, you'd end up in a **deadlock**: `cmd1` would fill the pipe buffer (~64KB) and block waiting for someone to read, but `cmd2` hasn't started yet.

`fork` solves both problems — it creates a disposable copy of your process that `execve` can replace, and it lets both commands run in parallel so one fills the pipe while the other drains it.

---

## Key Concepts

### File Descriptors

Every file, pipe, or device gets a number from the OS. By default:

| fd | destination |
|----|-------------|
| 0  | stdin (keyboard) |
| 1  | stdout (screen) |
| 2  | stderr (screen) |

`dup2` lets you rewire these numbers so commands read and write wherever you want.

### PATH Resolution

When you type `grep` in a terminal, the shell searches through each directory listed in the `PATH` environment variable:

```
/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin
```

pipex does the same — splits `PATH` on `:` and tests each directory with `access()` until it finds a matching executable.

### Exit Codes

| Code | Meaning |
|------|---------|
| 0 | success |
| 1 | general error |
| 127 | command not found |

### File Permissions

When creating `file2`, pipex uses `0644`:

| Who | Permission |
|-----|------------|
| owner | read + write |
| group | read only |
| other | read only |

---

## Error Handling

The program prints an error to stderr and exits cleanly if:

- Not enough arguments
- `file1` is missing or unreadable
- `file2` cannot be created or written to
- A command is not found in `PATH`
- `pipe()` or `fork()` fails

---

## Tests

```bash
# Setup
echo "hello world\nhi there\ntest" > input.txt

# Basic test
./pipex input.txt "cat" "grep hello" output.txt
cat output.txt

# Compare with bash
< input.txt cat | grep hello > output_shell.txt
diff output_shell.txt output.txt   # should be empty

# Absolute path
./pipex input.txt "/bin/cat" "grep hello" output.txt

# Missing input file
./pipex no_such_file "cat" "grep hello" output.txt

# Command not found
./pipex input.txt "no_such_cmd" "grep hello" output.txt

# Output file with no write permission
chmod 000 output.txt
./pipex input.txt "cat" "grep hello" output.txt
chmod 644 output.txt

# Empty input file
touch empty.txt
./pipex empty.txt "cat" "grep hello" output.txt

# grep finds nothing
./pipex input.txt "cat" "grep zzz" output.txt
cat output.txt   # should be empty

# Memory leaks
valgrind --leak-check=full ./pipex input.txt "cat" "grep hello" output.txt
```

---

## Resources

**Processes & pipes**
- [fork, pipe, execve, dup2 — CodeVault playlist](https://youtube.com/playlist?list=PLfqABt5AS4FkW5mOn2Tn9ZZLLDwA3kZUY&si=miCvpSIJp5lIC69y)
- [The exec family — GeeksForGeeks](https://www.geeksforgeeks.org/c/exec-family-of-functions-in-c/)
- [fork() — man page](https://man7.org/linux/man-pages/man2/fork.2.html)
- [pipe() — man page](https://man7.org/linux/man-pages/man2/pipe.2.html)
- [dup2() — man page](https://man7.org/linux/man-pages/man2/dup.2.html)

**File descriptors & redirections**
- [File descriptors — Wikipedia](https://en.wikipedia.org/wiki/File_descriptor)
- [Understanding file descriptors — bottomupcs.com](https://www.bottomupcs.com/file_descriptors.xhtml)

**Exit codes**
- [Exit codes in C — GeeksForGeeks](https://www.geeksforgeeks.org/cpp/exit-codes-in-c-c-with-examples/)

**Environment variables**
- [envp and PATH — Linux man pages](https://man7.org/linux/man-pages/man7/environ.7.html)
- [access() — man page](https://man7.org/linux/man-pages/man2/access.2.html)

**Memory**
- [Valgrind quick-start guide](https://valgrind.org/docs/manual/quick-start.html)

---

## AI Usage

Claude (Anthropic) was used during this project for:

- **fork / execve / pipe** — understanding how processes are duplicated and replaced, how data flows between them, and what exactly happens in memory at each step
- **dup2 and file descriptors** — understanding how stdin/stdout are rewired before execve, why the order of `close`/`dup2` calls matters, and why rewiring stdout in the child doesn't affect the parent
- **PATH resolution** — building the logic to find command paths by splitting PATH and testing each directory with `access()`, including handling absolute paths
- **Debugging** — tracing why commands weren't found, why pipes were deadlocking, and identifying double-free bugs
- **Memory management** — understanding why there are no leaks when execve succeeds (the OS cleans everything up), and where to free in error paths