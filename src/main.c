/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgeara <dgeara@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/30 21:43:54 by dgeara            #+#    #+#             */
/*   Updated: 2026/06/11 04:43:31 by dgeara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/pipex.h"

char    *try_path(char *dir, char *cmd)
{
    char    *tmp;
    char    *full;

    tmp = ft_strjoin(dir, "/");
    full = ft_strjoin(tmp, cmd);
    free(tmp);
    if (access(full, X_OK) == 0)
        return (full);
    free(full);
    return (NULL);
}

char    *get_path(char **envp)
{
    int i;

    i = 0;
    while (envp[i])
    {
        if (ft_strncmp(envp[i], "PATH=", 5) == 0)
            return (envp[i] + 5);   
        i++;
    }
    return (NULL);
}

char    *find_path(char *cmd, char **envp)
{
    char    **dirs;
    char    *path;
    char    *result;
    int     i;

    path = get_path(envp);
    if (!path)
        return (NULL);
    dirs = ft_split(path, ':');
    i = 0;
    while (dirs[i])
    {
        result = try_path(dirs[i], cmd);
        if (result)
        {
            i = 0;
            while (dirs[i])
                free(dirs[i++]);
            free(dirs);
            return (result);
        }
        i++;
    }
    return (NULL);
}

void    execute_cmd(char *cmd, char **envp)
{
    char    **args;
    char    *path;

    args = ft_split(cmd, ' ');
    path = find_path(args[0], envp);
    if (!path)
    {
        ft_putstr_fd("command not found\n", 2);
        exit(127);
    }
    execve(path, args, envp);
    perror("execve");
    exit(1);
}

void    child_process1(char **av, int pipe_fds[2], char **envp)
{
    int fd_in;

    fd_in = open(av[1], O_RDONLY);
    if (fd_in == -1)
        return (perror(av[1]), exit(1), (void)0);
    close(pipe_fds[0]);
    dup2(fd_in, STDIN_FILENO);
    dup2(pipe_fds[1], STDOUT_FILENO);
    close(pipe_fds[1]);
    close(fd_in);
    execute_cmd(av[2], envp);
}

void    child_process2(char **av, int pipe_fds[2], char **envp)
{
    int fd_out;
    
    fd_out = open(av[4], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_out == -1)
        return (perror(av[4]), exit(1), (void)0);
    close(pipe_fds[1]);
    dup2(pipe_fds[0], STDIN_FILENO);
    dup2(fd_out, STDOUT_FILENO);
    close(pipe_fds[0]);
    close(fd_out);
    execute_cmd(av[3], envp);
}

int main(int ac, char **av, char **envp)
{
    int     pipe_fds[2];
    pid_t   pid;

    if (ac != 5)
        return (ft_putstr_fd("Usage: ./pipex file1 cmd1 cmd2 file2\n", 1), 1);
    if (pipe(pipe_fds) == -1)
        return (perror("pipe"), 1);
    pid = fork();
    if (pid < 0)
        return (perror("fork"), 1);
    if (pid == 0)
        child_process1(av, pipe_fds, envp);
    pid = fork();
    if (pid < 0)
        return (perror("fork"), 1);
    if (pid == 0)
        child_process2(av, pipe_fds, envp);
    close(pipe_fds[0]);
    close(pipe_fds[1]);
    wait(NULL);
    wait(NULL);
    return (0);
}
