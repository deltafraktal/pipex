/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex_bonus.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgeara <dgeara@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/30 21:45:28 by dgeara            #+#    #+#             */
/*   Updated: 2026/06/17 20:58:44 by dgeara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PIPEX_BONUS_H
# define PIPEX_BONUS_H

# include "../libft/inc/libft.h"
# include <fcntl.h>
# include <stdio.h>
# include <unistd.h>
# include <stdlib.h>
# include <sys/wait.h>

# define BOLD "\033[1m"
# define PURPLE "\033[35m"
# define YELLOW "\033[33m"
# define END "\033[0m"

// main
int		main(int ac, char **av, char **envp);
void	child_process1(char **av, int pipe_fds[2], char **envp);
void	child_process2(char **av, int pipe_fds[2], char **envp);
void	execute_cmd(char *cmd, char **envp);

//checks and free
char	*find_path(char *cmd, char **envp);
char	*get_path(char **envp);
char	*try_path(char *dir, char *cmd);
void	free_tab(char **tab);

// split_cmd
char	**ft_split_cmd(char const *s, char c);
void	free_tab_split(char **tab, int i);

#endif