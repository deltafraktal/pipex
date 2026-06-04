/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgeara <dgeara@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/30 21:43:54 by dgeara            #+#    #+#             */
/*   Updated: 2026/06/04 04:43:20 by dgeara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/pipex.h"

/* int	main(int ac, char **av, char **envp)
{
    (void)*argv;

    if (argc != 5)
    {
        ft_putstr_fd("Usage: ./pipex file1 cmd1 cmd2 file2\n", 2);
        return (1);
    }
    return (0);
} */


int	main(int ac, char **av, char **envp)
{
    (void)*av;
    (void)ac;
    (void)*envp;
    int     pipe_fds[2];
    pid_t   pid;

    if (ac != 5)
		return (ft_putstr_fd("Usage: ./pipex file1 cmd1 cmd2 file2\n", 1), 1);
    if (pipe(pipe_fds) == -1)
        return (perror("pipe"), 1);
    //ft_printf("pipe créé ! pipe_fd[0]=%d  pipe_fd[1]=%d\n", pipe_fds[0], pipe_fds[1]);
    pid = fork();
    if (pid < 0)
        return (perror("fork"), 1);
    if (pid == 0)
    {
        // ENFANT : redirige stdout vers le pipe
        close(pipe_fds[0]);                    // on n'a pas besoin de lire
        dup2(pipe_fds[1], STDOUT_FILENO);      // stdout → pipe
        close(pipe_fds[1]);                    // fd original plus nécessaire
        char *args[] = {"/bin/ls", NULL};
        execve("/bin/ls", args, envp);
        perror("execve");
    }
    else
    {
        close(pipe_fds[1]);
        dup2(pipe_fds[0], STDIN_FILENO);   // stdin ← pipe
        close(pipe_fds[0]);
        char *args[] = {"/usr/bin/ls", "a", NULL};
        execve("/usr/bin/grep", args, envp);
        perror("execve");
        exit(1);
    }
    
    close(pipe_fds[1]);                    // on n'a pas besoin d'écrire
    close(pipe_fds[0]);
    waitpid(pid, NULL, 0);
    return (0);
}
