/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgeara <dgeara@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/30 21:45:28 by dgeara            #+#    #+#             */
/*   Updated: 2026/06/03 00:41:49 by dgeara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PIPEX_H
# define PIPEX_H

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

// function
// main
int	    main(int ac, char **av, char **envp);

//checks
void	check_ac(int ac);

#endif