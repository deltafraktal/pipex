# PIPEX

## Description

pipex est un projet sur les pipes Unix du cursus 42, écrit entièrement en C.

Il reproduit le comportement de l'opérateur pipe du shell — lire depuis un fichier d'entrée, faire passer les données à travers deux commandes reliées par un pipe, et écrire le résultat dans un fichier de sortie. En d'autres termes, il imite cette commande shell :

```bash
< file1 cmd1 | cmd2 > file2
```

Le vrai défi est de comprendre comment les processus Unix fonctionnent réellement sous le capot : comment `fork()` duplique un processus, comment `execve()` le remplace par un nouveau programme, comment `pipe()` crée un canal de communication entre deux processus, et comment `dup2()` recâble les file descriptors pour que les commandes lisent et écrivent au bon endroit sans le savoir.

---

## Fonctionnalités

### Mandatory

- Crée un pipe entre deux processus enfants
- L'enfant 1 lit depuis `file1`, exécute `cmd1`, écrit la sortie dans le pipe
- L'enfant 2 lit depuis le pipe, exécute `cmd2`, écrit la sortie dans `file2`
- Trouve les chemins des commandes automatiquement en cherchant dans `PATH`
- Supporte les chemins absolus (`/bin/cat`) et relatifs (`./myscript`)
- Gestion des erreurs : fichier manquant, permissions insuffisantes, commande introuvable
- Aucune fuite mémoire

---

## Fonctionnement

### Le pipe

`pipe()` crée deux file descriptors :
- `fd[0]` — côté lecture
- `fd[1]` — côté écriture

Tout ce qui est écrit dans `fd[1]` peut être lu depuis `fd[0]`. C'est un canal unidirectionnel entre deux processus.

### fork()

`fork()` duplique le processus courant — mémoire, variables, file descriptors ouverts, tout. La seule différence entre le parent et l'enfant est la valeur de retour de `fork()` :

```
fork() retourne 0    → je suis l'enfant
fork() retourne > 0  → je suis le parent (valeur = pid de l'enfant)
fork() retourne < 0  → erreur
```

pipex appelle `fork()` deux fois — une fois par commande. Chaque enfant reçoit une copie des file descriptors du pipe.

### dup2()

`dup2(ancien, nouveau)` fait pointer `fd nouveau` vers la même destination que `fd ancien`. C'est comme ça que les redirections fonctionnent :

```c
dup2(fd_in, STDIN_FILENO);          // stdin lit maintenant depuis file1
dup2(pipe_fds[1], STDOUT_FILENO);   // stdout écrit maintenant dans le pipe
```

Après `dup2`, la commande ne sait pas qu'elle lit un fichier ou écrit dans un pipe — elle lit simplement depuis stdin et écrit sur stdout comme d'habitude.

### execve()

`execve()` ne lance pas un programme à côté de toi — il remplace ton processus entièrement. Code, pile, mémoire — tout disparaît, remplacé par le nouveau programme. Les file descriptors survivent (d'où l'importance de faire `dup2` avant de l'appeler).

Si `execve` retourne, c'est qu'il a échoué.

### Pourquoi fork et pas juste execve directement ?

Sans `fork`, le premier `execve` remplacerait ton processus et la deuxième commande ne tournerait jamais. Et même si ce n'était pas le cas, tu te retrouverais dans un deadlock : `cmd1` remplirait le buffer du pipe (~64ko) et se bloquerait en attendant que quelqu'un lise, mais `cmd2` n'a pas encore démarré.

`fork` résout les deux problèmes — il crée une copie jetable de ton processus qu'`execve` peut remplacer, et il permet aux deux commandes de tourner en parallèle pour que l'une remplisse le pipe pendant que l'autre le vide.

---

## Compilation

```bash
make
```

```bash
make clean    # supprime les fichiers objets
make fclean   # supprime les fichiers objets + le binaire
make re       # fclean + make
```

---

## Utilisation

```bash
./pipex file1 "cmd1" "cmd2" file2
```

Équivalent à :

```bash
< file1 cmd1 | cmd2 > file2
```

### Exemples

```bash
./pipex input.txt "cat" "grep bonjour" output.txt
./pipex input.txt "ls -la" "wc -l" output.txt
./pipex input.txt "/bin/cat" "grep hello" output.txt
```

---

## Gestion des erreurs

Le programme affiche une erreur sur stderr et quitte proprement si :

- Pas assez d'arguments
- `file1` est manquant ou illisible
- `file2` ne peut pas être créé ou écrit
- Une commande est introuvable dans PATH
- `pipe()` ou `fork()` échoue

---

## Tests

```bash
# préparation
echo "bonjour monde\nhello world\ntest" > input.txt

# test basique
./pipex input.txt "cat" "grep bonjour" output.txt
cat output.txt

# comparaison avec bash
< input.txt cat | grep bonjour > output_shell.txt
diff output_shell.txt output.txt   # doit être vide

# chemin absolu
./pipex input.txt "/bin/cat" "grep bonjour" output.txt

# fichier d'entrée inexistant
./pipex fichier_nope "cat" "grep bonjour" output.txt

# commande introuvable
./pipex input.txt "cmd_nope" "grep bonjour" output.txt

# fichier de sortie sans permission d'écriture
chmod 000 output.txt
./pipex input.txt "cat" "grep bonjour" output.txt
chmod 644 output.txt

# fichier d'entrée vide
touch empty.txt
./pipex empty.txt "cat" "grep bonjour" output.txt

# grep ne trouve rien
./pipex input.txt "cat" "grep zzz" output.txt
cat output.txt   # doit être vide

# fuites mémoire
valgrind --leak-check=full ./pipex input.txt "cat" "grep bonjour" output.txt
```

---

## Concepts clés

### File descriptors

Pour chaque fichier, pipe ou périphérique, l'OS donne à ton processus un numéro — un file descriptor. Par défaut :

```
fd 0  →  stdin  (clavier)
fd 1  →  stdout (écran)
fd 2  →  stderr (écran)
```

`dup2` permet de recâbler ces numéros pour que les commandes lisent et écrivent où tu veux.

### PATH

Quand tu tapes `grep` dans un terminal, le shell ne sait pas magiquement où il se trouve. Il cherche dans chaque dossier listé dans la variable d'environnement `PATH` :

```
/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin
```

pipex fait la même chose — il split `PATH` sur `:` et teste chaque dossier jusqu'à trouver un exécutable correspondant.

### Codes de sortie

```
0    →  succès
1    →  erreur générale
127  →  commande introuvable
```

### Permissions fichier

Lors de la création de `file2`, pipex utilise `0644` :

```
propriétaire  →  lecture + écriture  (6)
groupe        →  lecture seulement   (4)
autres        →  lecture seulement   (4)
```

---

## Ressources

**Processus et pipes**
- [fork, pipe, execve, dup2 — playlist CodeVault](https://youtube.com/playlist?list=PLfqABt5AS4FkW5mOn2Tn9ZZLLDwA3kZUY&si=miCvpSIJp5lIC69y)
- [La famille exec — GeeksForGeeks](https://www.geeksforgeeks.org/c/exec-family-of-functions-in-c/)
- [fork() — man page](https://man7.org/linux/man-pages/man2/fork.2.html)
- [pipe() — man page](https://man7.org/linux/man-pages/man2/pipe.2.html)
- [dup2() — man page](https://man7.org/linux/man-pages/man2/dup.2.html)

**File descriptors et redirections**
- [File descriptors — Wikipedia](https://en.wikipedia.org/wiki/File_descriptor)
- [Tout comprendre sur les fd — bottomupcs.com](https://www.bottomupcs.com/file_descriptors.xhtml)

**Codes de sortie**
- [Exit codes en C — GeeksForGeeks](https://www.geeksforgeeks.org/cpp/exit-codes-in-c-c-with-examples/)

**Variables d'environnement**
- [envp et PATH — Linux man pages](https://man7.org/linux/man-pages/man7/environ.7.html)
- [access() — man page](https://man7.org/linux/man-pages/man2/access.2.html)

**Mémoire et valgrind**
- [Guide valgrind](https://valgrind.org/docs/manual/quick-start.html)

---

## Utilisation de l'IA

Claude (Anthropic) a été utilisé pendant ce projet pour :

- **fork / execve / pipe** — comprendre comment les processus sont dupliqués, remplacés, et comment les données circulent entre eux ; clarifier pourquoi fork est nécessaire et ce qui se passe exactement en mémoire à chaque étape

- **dup2 et file descriptors** — comprendre comment stdin et stdout sont recâblés avant execve, pourquoi l'ordre des appels `close` / `dup2` est important, et pourquoi recâbler stdout dans l'enfant n'affecte pas le parent

- **Résolution de PATH** — construire la logique pour trouver les chemins des commandes en splittant PATH et en testant chaque dossier avec `access()`, et gérer le cas des chemins absolus

- **Débogage** — tracer pourquoi des commandes n'étaient pas trouvées, pourquoi les pipes se bloquaient, et identifier des bugs de double free

- **Gestion mémoire** — comprendre pourquoi il n'y a pas de fuite quand execve réussit (l'OS nettoie tout), et où libérer dans les chemins d'erreur