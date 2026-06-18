/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils2.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgeara <dgeara@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/17 21:53:53 by dgeara            #+#    #+#             */
/*   Updated: 2026/06/18 02:49:50 by dgeara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/pipex_bonus.h"

void	run_pipe(t_pipex *p, char **av, char **envp)
{
	if (pipe(p->pipe_fds) == -1)
		return (perror("pipe"), exit(1), (void)0);
	if (fork() == 0)
	{
		dup2(p->current_fd, STDIN_FILENO);
		dup2(p->pipe_fds[1], STDOUT_FILENO);
		close(p->pipe_fds[0]);
		close(p->pipe_fds[1]);
		execute_cmd(av[p->i], envp);
	}
	close(p->pipe_fds[1]);
}

void	run_last_pipe(t_pipex *p, int ac, char **av, char **envp)
{
	int	last_fd;

	if (p->is_heredoc)
		last_fd = open(av[ac - 1],
				O_WRONLY | O_CREAT | O_APPEND, 0644);
	else
		last_fd = open(av[ac - 1],
				O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (last_fd == -1)
		return (perror(av[ac - 1]), (void)0);
	if (fork() == 0)
	{
		dup2(p->current_fd, STDIN_FILENO);
		dup2(last_fd, STDOUT_FILENO);
		close(p->current_fd);
		close(last_fd);
		execute_cmd(av[p->i], envp);
	}
	close(last_fd);
}
