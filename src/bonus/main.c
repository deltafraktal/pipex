/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgeara <dgeara@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/30 21:43:54 by dgeara            #+#    #+#             */
/*   Updated: 2026/06/17 23:41:10 by dgeara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/pipex_bonus.h"

void	execute_cmd(char *cmd, char **envp)
{
	char	**args;
	char	*path;

	args = ft_split_cmd(cmd, ' ');
	path = find_path(args[0], envp);
	if (!path)
	{
		free_tab(args);
		ft_putstr_fd("command not found\n", 2);
		exit(127);
	}
	execve(path, args, envp);
	free(path);
	free_tab(args);
	perror("execve");
	exit(1);
}

static void	child_mid(char *cmd, int fd_in, int fd_out, char **envp)
{
	dup2(fd_in, STDIN_FILENO);
	dup2(fd_out, STDOUT_FILENO);
	close(fd_in);
	close(fd_out);
	execute_cmd(cmd, envp);
}

static int	here_doc(char *limiter)
{
	int		pipe_fds[2];
	char	*line;

	if (pipe(pipe_fds) == -1)
		return (perror("pipe"), -1);
	while (1)
	{
		ft_putstr_fd("heredoc> ", 1);
		line = get_next_line(0);
		if (!line)
			break ;
		if (ft_strncmp(line, limiter, ft_strlen(limiter)) == 0
			&& line[ft_strlen(limiter)] == '\n')
		{
			free(line);
			break ;
		}
		ft_putstr_fd(line, pipe_fds[1]);
		free(line);
	}
	close(pipe_fds[1]);
	return (pipe_fds[0]);
}

int	main(int ac, char **av, char **envp)
{
	int	pipe_fds[2];
	int	current_fd;
	int	last_fd;
	int	i;
	int	is_heredoc;

	if (ac < 5)
		return (ft_putstr_fd("Usage: ./pipex file1 cmd1 ... cmdn file2\n", 2), 1);
	is_heredoc = (ft_strncmp(av[1], "here_doc", 8) == 0);
	if (is_heredoc)
	{
		if (ac < 6)
			return (ft_putstr_fd("Usage: ./pipex here_doc LIMITER cmd ... file\n", 2), 1);
		current_fd = here_doc(av[2]);
	}
	else
		current_fd = open(av[1], O_RDONLY);
	if (current_fd == -1)
		return (perror(av[1]), 1);
	i = 0;
	while (i < ac - is_heredoc - 3)
	{
		if (i == ac - is_heredoc - 2)
		{
			if (is_heredoc)
				last_fd = open(av[ac - 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
			else
				last_fd = open(av[ac - 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if (last_fd == -1)
				return (perror(av[ac - 1]), 1);
			if (fork() == 0)
				child_mid(av[i], current_fd, last_fd, envp);
			close(last_fd);
		}
		else
		{
			if (pipe(pipe_fds) == -1)
				return (perror("pipe"), 1);
			if (fork() == 0)
				child_mid(av[i], current_fd, pipe_fds[1], envp);
			close(pipe_fds[1]);
		}
		close(current_fd);
		current_fd = pipe_fds[0];
		i++;
	}
	close(current_fd);
	i = ac - 3;
	while (i-- > 0)
		wait(NULL);
	return (0);
}
