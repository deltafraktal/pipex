/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgeara <dgeara@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/30 21:43:54 by dgeara            #+#    #+#             */
/*   Updated: 2026/06/18 02:49:03 by dgeara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/pipex_bonus.h"

void	execute_cmd(char *cmd, char **envp)
{
	char	**args;
	char	*path;

	args = ft_split_cmd(cmd, ' ');
	if (!args)
		exit(1);
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

int	here_doc(char *limiter)
{
	int		pipe_fds[2];
	char	*line;

	if (pipe(pipe_fds) == -1)
		return (perror("pipe"), -1);
	while (1)
	{
		ft_putstr_fd("pipe heredoc> ", STDOUT_FILENO);
		line = get_next_line(STDIN_FILENO);
		if (!line)
			break ;
		if (ft_strncmp(line, limiter, ft_strlen(limiter)) == 0
			&& line[ft_strlen(limiter)] == '\n')
			return (free(line), close(pipe_fds[1]), pipe_fds[0]);
		ft_putstr_fd(line, pipe_fds[1]);
		free(line);
	}
	close(pipe_fds[1]);
	return (pipe_fds[0]);
}

int	parse_and_init(t_pipex *p, int ac, char **av)
{
	if (ac < 5)
	{
		ft_putstr_fd("Usage: ./pipex file1 cmd1 ... cmdn file2\n", 2);
		return (0);
	}
	p->is_heredoc = (ft_strncmp(av[1], "here_doc", 8) == 0);
	if (p->is_heredoc)
	{
		if (ac < 6)
		{
			ft_putstr_fd("Usage: ./pipex here_doc LIMITER cmd ... file\n", 2);
			return (0);
		}
		p->current_fd = here_doc(av[2]);
	}
	else
		p->current_fd = open(av[1], O_RDONLY);
	if (p->current_fd == -1)
		return (perror(av[1]), 0);
	return (1);
}

int	main(int ac, char **av, char **envp)
{
	t_pipex	p;

	if (!parse_and_init(&p, ac, av))
		return (1);
	p.i = 2 + p.is_heredoc;
	while (p.i < ac - 1)
	{
		if (p.i != ac - 2)
			run_pipe(&p, av, envp);
		else
			run_last_pipe(&p, ac, av, envp);
		close(p.current_fd);
		p.current_fd = p.pipe_fds[0];
		p.i++;
	}
	close(p.current_fd);
	while (ac-- > 3)
		wait(NULL);
	return (0);
}
