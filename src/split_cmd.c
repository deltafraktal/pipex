/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   split_cmd.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgeara <dgeara@student.42lausanne.ch>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/30 21:43:54 by dgeara            #+#    #+#             */
/*   Updated: 2026/06/17 17:17:27 by dgeara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/pipex.h"

static int	count_words(char const *s, char c)
{
	int	i;
	int	in_quote;

	i = 0;
	in_quote = 0;
	while (*s)
	{
		if (*s == '\'')
			in_quote = !in_quote;
		if (*s != c && !in_quote && (*(s + 1) == c || *(s + 1) == '\0'))
			i++;
		s++;
	}
	return (i);
}

static int	word_len(char const *s, char c)
{
	int	len;

	len = 0;
	while (*s && *s != c)
	{
		if (*s == '\'')
		{
			s++;
			while (*s && *s != '\'')
			{
				len++;
				s++;
			}
			if (*s == '\'')
				s++;
		}
		else
		{
			len++;
			s++;
		}
	}
	return (len);
}

static void	fill_word(char *dst, char const **s, char c)
{
	int	j;

	j = 0;
	while (**s && **s != c)
	{
		if (**s == '\'')
		{
			(*s)++;
			while (**s && **s != '\'')
				dst[j++] = *(*s)++;
			if (**s == '\'')
				(*s)++;
		}
		else
			dst[j++] = *(*s)++;
	}
	dst[j] = '\0';
}

static int	fill_tab(char **tab, char const *s, char c)
{
	int	i;
	int	len;

	i = 0;
	while (*s)
	{
		while (*s == c)
			s++;
		len = word_len(s, c);
		if (len > 0)
		{
			tab[i] = malloc(sizeof(char) * (len + 1));
			if (!tab[i])
				return (free_tab_split(tab, i), 0);
			fill_word(tab[i], &s, c);
			i++;
		}
	}
	tab[i] = NULL;
	return (1);
}

char	**ft_split_cmd(char const *s, char c)
{
	char	**tab;

	if (!s)
		return (NULL);
	tab = malloc(sizeof(char *) * (count_words(s, c) + 1));
	if (!tab)
		return (NULL);
	if (fill_tab(tab, s, c) == 0)
		return (NULL);
	return (tab);
}
