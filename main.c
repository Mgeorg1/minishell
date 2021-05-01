#include "includes/minishell.h"
#include <string.h>

int		print_merror(t_all *all)
{
	printf("minishell: memmory allocate error\n");
	return (-1);
}

t_all *init_all(char **envp)
{
	t_all *all;

	all = (t_all*)malloc(sizeof(t_all));
	if (!all)
		return (NULL);
	all->hist_list = NULL;
	all->history = NULL;
	all->hist_len = 0;
	all->envp = parse_env(envp);
	all->home = getenv("HOME");
	all->file = ft_strjoin(all->home, "/.minishell_history");
	all->cmds = NULL;
	all->src = NULL;
	return (all);
}

int		ft_putint(int c)
{
	write(1, &c, 1);
	return (c);
}

char 	*get_env_val(t_list *envp, char *key)
{
	t_env *tmp;
	t_list *tmp_list;

	tmp_list = envp;
	while (tmp_list)
	{
		tmp = tmp_list->content;
		if (!ft_strncmp(tmp->key, key, ft_strlen(key) + 1 ))
			return (tmp->value);
		tmp_list = tmp_list->next;
	}
	return (NULL);
}


int		get_hist_array(t_all *all)
{
	t_list *tmp;
	int i;

	all->hist_len = ft_lstsize(all->hist_list);
	i = 0;
	all->history = (char**)malloc(sizeof(char*) * (all->hist_len + 1));
	if (!all->history)
		return (-1);
	tmp = all->hist_list;
	while(tmp)
	{
		all->history[i] = tmp->content;
		i++;
		tmp = tmp->next;
	}
	all->history[i] = NULL;
 	return (0);
}

int		read_history(t_all *all)
{
	int fd;
	int bytes;
	char *line;
	int i;

	line = NULL;
	fd = open(all->file, O_RDWR | O_CREAT, 0777);
	ft_lstclear(&all->hist_list, free);
	i = 0;
	if (all->history)
		free(all->history);
	all->hist_list = NULL;
	while ((bytes = get_next_line(fd, &line)) != 0)
	{
		if (line[0] != '\0' && line[0] != '\04')
			ft_lstadd_back(&all->hist_list, ft_lstnew(line));
		else
			free(line);
		if (all->hist_list == NULL)
		{	
			close(fd);
			return (-1);
		}
		line = NULL;
	}
	close(fd);
	if (line)
	{
		if (line[0] != '\0' && line[0] != '\04')
			ft_lstadd_back(&all->hist_list, ft_lstnew(line));
		else
			free(line);
	}
	t_list *tmp = all->hist_list;
	if (fd < 0 || bytes < 0)
		return (-1);
	return (0);
}

int		save_history(t_all *all)
{
	int fd;
	t_list *tmp;

	fd = open(all->file, O_RDWR | O_TRUNC | O_CREAT, 0777);
	if (fd < 0)
	{
		close (fd);
		return (-1);
	}
	tmp = all->hist_list;
	while(tmp)
	{
		if (ft_strlen(tmp->content) > 0)
			ft_putendl_fd(tmp->content, fd);
		tmp = tmp->next;
	}
	close(fd);
	return (0);
}

int		hist_strjoin(t_all *all, char *str)
{
	t_list *tmp;
	int i;
	char *t;

	i = 0;
	tmp = ft_lstlast(all->hist_list);
	if (str[0] == '\04')
		return (0);
	if (ft_strncmp(all->history[all->pos], all->history[all->hist_len - 1], ft_strlen(all->history[all->pos])))
	{
		t = all->history[all->hist_len - 1];
		all->history[all->hist_len - 1] = ft_strjoin(all->history[all->pos], str);
		if (all->history[all->hist_len - 1] == NULL)
			return (print_merror(all));
		free(t);
	}
	else
	{
		all->history[all->hist_len - 1] = my_strjoin(all->history[all->hist_len - 1], str);
		if (all->history[all->hist_len - 1] == NULL)
			return (print_merror(all));
	}
	tmp->content = all->history[all->hist_len - 1];
	return (0);
}

void	print_previus(t_all *all)
{
	all->pos--;
	if (all->pos < 0)
	{
		all->pos = 0;
		if (all->hist_len == 1)
			return ;
	}
	ft_putstr_fd(all->history[all->pos], 1);
}

void	print_next(t_all *all)
{
	all->pos++;
	if (all->pos > all->hist_len - 1)
		all->pos = all->hist_len - 1;
	ft_putstr_fd(all->history[all->pos], 1);
}

void	nocanon(t_all *all)
{
	all->term_name = get_env_val(all->envp, "TERM");
	tcgetattr(0, &all->term);
	all->term.c_lflag &= ~(ECHO);
	all->term.c_lflag &= ~(ICANON);
	all->term.c_lflag &= ~(ISIG);
	tcsetattr(0, TCSANOW, &all->term);
	tgetent(0, all->term_name);
	tputs("minishell$ ", 1, ft_putint);
	tputs(save_cursor, 1, ft_putint);
}

int		new_line(t_all *all, char *str)
{
	t_list	*tmp;

	read_history(all);
	tmp = ft_lstlast(all->hist_list);
	if ((all->hist_len == 0) || tmp == NULL)
		ft_lstadd_back(&all->hist_list, ft_lstnew(ft_strdup("")));
	else if (all->hist_len == 0 || ft_strlen(tmp->content) != 0)
		ft_lstadd_back(&all->hist_list, ft_lstnew(ft_strdup("")));
	if (get_hist_array(all) < 0)
		return (-1);
	all->pos = all->hist_len - 1;
	ft_bzero(str, 2000);
	
	return (0);
}

char	previus_char(t_line *src)
{
	char c;
	int i;

	if (src->pos != 0)
		i = src->pos - 1;
	else
		i = src->pos;
	return (src->str[i]);
}

void	check_quotes(t_all *all, t_line *src)
{
	if (src->str[src->pos] == '"' && all->val.in_dqt == 0 && all->val.in_qt == 0 && previus_char(src) != '\\')
		all->val.in_dqt = 1;
	else if (src->str[src->pos] == '\'' && all->val.in_qt == 0 && previus_char(src) != '\\')
		all->val.in_qt = 1;
	else if (src->str[src->pos] == '"' && all->val.in_dqt == 1 && all->val.in_qt == 0 && previus_char(src) != '\\')
		all->val.in_dqt = 0;
	else if (src->str[src->pos] == '\'' && all->val.in_qt == 1 && previus_char(src) != '\\')
		all->val.in_qt = 0;
}

void shield_sym(t_all *all, t_line *src)
{
	if (src->str[src->pos] == '$' && (all->val.in_qt == 1 || previus_char(src) == '\\'))
		src->str[src->pos] *= -1;
	else if (src->str[src->pos] == '#' && (all->val.in_qt == 1 || previus_char(src) == '\\' || all->val.in_dqt == 1))
		src->str[src->pos] *= -1;
	else if (src->str[src->pos] == ' ' && (all->val.in_qt == 1 || previus_char(src) == '\\' || all->val.in_dqt == 1))
		src->str[src->pos] *= -1;
	else if (src->str[src->pos] == '>' && (all->val.in_qt == 1 || previus_char(src) == '\\' || all->val.in_dqt == 1))
		src->str[src->pos] *= -1;
	else if (src->str[src->pos] == ';' && (all->val.in_qt == 1 || previus_char(src) == '\\' || all->val.in_dqt == 1))
		src->str[src->pos] *= -1;
	else if (src->str[src->pos] == '<' && (all->val.in_qt == 1 || previus_char(src) == '\\' || all->val.in_dqt == 1))
		src->str[src->pos] *= -1;
	else if (src->str[src->pos] == '&' && (all->val.in_qt == 1 || previus_char(src) == '\\' || all->val.in_dqt == 1))
		src->str[src->pos] *= -1;
	else if (src->str[src->pos] == ' ' && (all->val.in_qt == 1 || previus_char(src) == '\\' || all->val.in_dqt == 1))
		src->str[src->pos] *= -1;
	else if (src->str[src->pos] == '(' && (all->val.in_qt == 1 || previus_char(src) == '\\' || all->val.in_dqt == 1))
		src->str[src->pos] *= -1;
	else if (src->str[src->pos] == ')' && (all->val.in_qt == 1 || previus_char(src) == '\\' || all->val.in_dqt == 1))
		src->str[src->pos] *= -1;
	else if (src->str[src->pos] == '|' && (all->val.in_qt == 1 || previus_char(src) == '\\' || all->val.in_dqt == 1))
		src->str[src->pos] *= -1;
	else if (src->str[src->pos] == '"' && (all->val.in_qt == 1 || previus_char(src) == '\\' || all->val.in_dqt == 1))
		src->str[src->pos] *= -1;
	else if (src->str[src->pos] == '\\' && (all->val.in_qt == 1 || previus_char(src) == '\\' || all->val.in_dqt == 1))
		src->str[src->pos] *= -1;
	if (src->str[src->pos] == '\'' && (previus_char(src) == '\\' || all->val.in_dqt == 1))
		src->str[src->pos] *= -1;
}

int 	is_right_syntax_pipes(t_line *src, t_all *all, int *k)
{
	if ((src->str[src->pos] == '|' || src->str[src->pos] == '>'
	|| src->str[src->pos] == '<') && src->pos == src->len - 2)
	{
		printf("minishell: syntax error near unexpected token `newline'\n");
		return (-1);
	}
	if (src->str[src->pos] == '|' && src->pos == 0)
	{
		printf("minishell: syntax error near unexpected token `|'\n");
		return (-1);
	}
	if (src->str[src->pos] == '&')
	{
		printf("minishell: syntax error unexpected token `&'\n");
		return (-1);
	}
	if (src->str[src->pos] == '|')
	{
		*k += 1;
		if (*k > 1)
		{
			printf("minishell: syntax error near unexpected token `|'\n");
			return (-1);
		}
	}
	else
		*k = 0;
	return (0);
}

int		is_right_redir_syntax(t_all *all, t_line *src, int *redir, int *rev_redir)
{
	if (src->str[src->pos] == '>')
	{
		*redir += 1;
		if (*redir > 2 || *rev_redir > 0)
		{
			printf("minishell: syntax error near unexpected token `>'\n");
			return (-1);
		}
		if (src->str[src->pos + 1] == '>' && previus_char(src) == '<')
		{
			printf("minishell: syntax error near unexpected token `<'\n");
			return (-1);
		}
		if (src->str[src->pos + 1] == '<')
		{
			printf("minishell: syntax error near unexpected token `<'\n");
			return (-1);
		}
	}
	else if (src->str[src->pos] != ' ')
		*redir = 0;
	return (0);
}

int		is_right_revd_syntax(t_all *all, t_line *src, int *rev_redir, int *redir)
{
	if (src->str[src->pos] == '<')
	{
		*rev_redir += 1;
		if (*rev_redir > 2 || *redir > 0)
		{
			printf("minishell: syntax error near unexpected token `<'\n");
			return (-1);
		}

		if (src->str[src->pos + 1] == '>' && previus_char(src) == '<')
		{
			printf("minishell: syntax error near unexpected token `<'\n");
			return (-1);
		}
	}
	else if (src->str[src->pos] != ' ')
		*rev_redir = 0;
	return (0);
}

int		shield(t_all *all, t_line *src)
{
	int i;
	int k;
	int redir;
	int rev_redir;

	i = 0;
	k = 0;
	redir = 0;
	rev_redir = 0;
	while (src->str[src->pos])
	{
		check_quotes(all, src);
		shield_sym(all, src);
		if (is_right_syntax_pipes(src, all, &k) < 0)
			return (-1);
		if (is_right_redir_syntax(all, src, &redir, &rev_redir) < 0)
			return (-1);
		if (is_right_revd_syntax(all, src, &rev_redir, &redir) < 0)
			return(-1);
		src->pos++;
	}
	if (all->val.in_dqt == 1 || all->val.in_qt == 1)
	{
		printf("minishell: quote is not closed\n");
		return (-1);
	}
	return (0);
}

int 	unshield(char *str)
{
	int i;
	
	i = 0;
	while (str[i])
	{
		if (str[i] < 0)
			str[i] *= -1;
		i++;
	}
	return (0);
}

char		*ft_charjoin(char const *s1, char c)
{
	int		len;
	int		i;
	char	*a;

	if (!s1)
		return (NULL);
	len = ft_strlen((char *)s1) + 1;
	a = (char*)malloc((len + 1) * sizeof(char));
	i = 0;
	if (a == NULL)
		return (NULL);
	while (s1[i])
	{
		a[i] = s1[i];
		i++;
	}
	a[i] = c;
	a[i + 1] = '\0';
	free ((char *)s1);
	return (a);
}

int		remove_ch(t_line *src)
{
	char *tmp;

	tmp = ft_strdup("");
	if (!tmp)
		return (-1);
	src->pos = 0;
	while (src->str[src->pos])
	{
		if (src->str[src->pos] != '\\' && src->str[src->pos] != '"'
		&& src->str[src->pos] != '\'' && src->str[src->pos] != '\n')
		{
			tmp = ft_charjoin(tmp, src->str[src->pos]);
			if (!tmp)
				return (-1);
		}
		src->pos++;
	}
	free(src->str);
	src->str = tmp;
	return (0);
}

char *end_var(char *s)
{
	char *s1;
	char *tmp;

	s1 = s;
	while(*s1 && (ft_isalnum(*s1) || *s1 == '_'))
		s1++;
	tmp = ft_substr(s, 0, s1 - s);
	return (tmp);
}

char 	*after_var(char *s)
{
	while(*s && (ft_isalnum(*s) || *s == '_'))
		s++;
	return (s);
}


int		argv_len(char **argv)
{
	int i;

	i = 0;
	while (argv[i])
		i++;
	return (i);
}


int		check_redir(char **argv)
{
	int i;
	int red;
	int rev_red;

	rev_red = 0;
	red = 0;
	i = 0;
	i = argv_len(argv);
	if ((!ft_strncmp(argv[0], "<", 2) || !ft_strncmp(argv[0], ">", 2)
	|| !ft_strncmp(argv[0], "<<", 3) || !ft_strncmp(argv[0], ">>", 3)) && i == 2)
		return (-1);
	i = 0;
	while (argv[i])
	{
		if (!ft_strncmp(argv[i], "<", 2) ||  !ft_strncmp(argv[i], "<<", 2))
		{
			rev_red++;
			if (rev_red > 1 || red > 0)
				{
					printf("minishell: syntax error near unexpected token `<'\n");
					return (-1);
				}
		}
		else if (!ft_strncmp(argv[i], ">", 2) || !ft_strncmp(argv[i], ">>", 3))
		{
			red++;
			if (red > 1 || rev_red > 0)
				{
					printf("minishell: syntax error near unexpected token `>'\n");
					return (-1);
				}
		}
		else
		{
			rev_red = 0;
			red = 0;
		}
		i++;
	}
	return (0);
}

int		check_sym(char *argv, int *k, char *c)
{
	int j;

	j = 0;
	while (argv[j])
	{
		if ((0 == *k || 3 == *k) && (argv[j] == '<' || argv[j] == '>'))
		{
			*c = argv[j];
			*k = 1;
		}
		if (argv[j] == ' ' && *k == 0)
			*k = 2;
		if (ft_isalnum(argv[j]) || argv[j] == '_' || '$' == argv[j])
			*k = 3;
		j++;
	}
	return (j);
}

int		check_argv(t_all *all, char **argv)
{
	int i;
	int k;
	char c;
	int j;

	i = 0;
	while (argv[i])
	{
		k = 0;
		j = check_sym(argv[i], &k, &c);
		if (k == 1)
		{	
			printf("minishell: syntax error near unexpected token `%c'\n", c);
			return(-1);
		}
		if (k == 2 || j == 0)
		{
			printf("minishell: error multiline command\n");
			return (-1);
		}
		i++;
	}
	return (0);
}

char	**realoc_argv(char **src)
{
	char **new;
	int len;
	int i;

	i = 0;
	len = argv_len(src);
	new = (char**)malloc(sizeof(char*) * (len + 2));
	char *s;
	while (i < len)
	{
		s = src[i];
		new[i] = src[i];
		i++;
	}
	new[len + 1] = NULL;
	free(src);
	return(new);
}

char	**arg_join(t_all *all, char **argv, char *str)
{
	char **tmp;
	int len;
	char *s;

	if (argv == NULL)
	{
		argv = (char**)ft_calloc(2, sizeof(char*));
		if (!argv)
			return (NULL);
		argv[0] = str;
		tmp = argv;
	}
	else
	{
		len = argv_len(argv);
		tmp = realoc_argv(argv);
		if (!tmp)
		{
			print_merror(all);
			return (NULL);
		}
		tmp[len] = str;
	}
	return (tmp);
}

int		if_space(t_tok *tok, t_all *all, int *i)
{
	tok->res = arg_join(all, tok->res, tok->tmp);
	if (!tok->tmp)
		return (print_merror(all));
	tok->tmp = ft_strdup("");
	while (tok->str_tmp[*i] && tok->str_tmp[*i] == ' ')
		*i += 1;
	return (0);
}

int if_redir(t_all *all, t_tok *tok, int *i)
{
	if (tok->str_tmp[*i] == '<')
	{
		if (tok->str_tmp[*i + 1] == '<')
		{
			tok->tmp_redir = ft_strdup("<<");
			if (!tok->tmp_redir)
				return (print_merror(all));
			*i += 1;
		}
		else
		{
			tok->tmp_redir = ft_strdup("<");
			if (!tok->tmp_redir)
				return (print_merror(all));
		}
		if (!ft_strncmp(tok->tmp, "", 2))
			tok->res = arg_join(all, tok->res, tok->tmp_redir);
		else
		{
			tok->res = arg_join(all, tok->res, tok->tmp);
			tok->res = arg_join(all, tok->res, tok->tmp_redir);
			tok->tmp = ft_strdup("");
		}
	}
	return (0);
}

int if_char(t_all *all, t_tok *tok, int *i)
{
	if (tok->str_tmp[*i] != ' ' && tok->str_tmp[*i] != '<'
	&& tok->str_tmp[*i] != '>')
	{
		tok->tmp = ft_charjoin(tok->tmp, tok->str_tmp[*i]);
		if (!tok->tmp)
			return (print_merror(all));
	}
	if (tok->str_tmp[*i + 1] == '\0' && tok->str_tmp[*i] != '<' 
	&& tok->str_tmp[*i] != '>' && tok->str_tmp[*i] != ' ')
	{
		tok->res = arg_join(all, tok->res, tok->tmp);
		if (!tok->tmp)
			return (print_merror(all));
	}
	return (0);
}

int		if_rev_redir(t_all *all, t_tok *tok, int *i)
{
	if (tok->str_tmp[*i] == '>')
	{
		if (tok->str_tmp[*i + 1] == '>')
		{
			tok->tmp_redir = ft_strdup(">>");
			if (!tok->tmp_redir)
				return (print_merror(all));
			*i += 1;
		}
		else
		{
			tok->tmp_redir = ft_strdup(">");
			if (!tok->tmp_redir)
				return (print_merror(all));
		}
		if (!ft_strncmp(tok->tmp, "", 2))
			tok->res = arg_join(all, tok->res, tok->tmp_redir);
		else
		{
			tok->res = arg_join(all, tok->res, tok->tmp);
			tok->res = arg_join(all, tok->res, tok->tmp_redir);
			tok->tmp = ft_strdup("");
		}
	}
	return (0);
}

char**	tokenize(char *str, t_all *all)
{
	int i;
	t_tok tok;

	i = 0;
	tok.res = NULL;
	tok.str_tmp = ft_strtrim(str, " ");
	tok.tmp = ft_strdup("");
	while(tok.str_tmp[i])
	{
		tok.tmp_redir = NULL;
		if (!tok.tmp)
		{
			print_merror(all);
			return (NULL);
		}
		if (tok.str_tmp[i] == ' ')
		{
			if (i > 0 && tok.str_tmp[i - 1] != ' ' && tok.str_tmp[i - 1] != '<'
			&& tok.str_tmp[i - 1] != '>')
			{
				if (if_space(&tok, all, &i) < 0)
					return (NULL);
				continue ;
			}
		}
		if (if_char(all, &tok, &i) < 0 || if_redir(all, &tok, &i) < 0
		|| if_rev_redir(all, &tok, &i) < 0)
			return (NULL);
		i++;
	}
	free(tok.str_tmp);
	free(str);;
	i = 0;
	return (tok.res);
}

int		m_struct(t_all *all, char ***argv)
{
	int i;

	all->cmds = (t_cmd*)malloc(sizeof(t_cmd) * all->cmds_num);
	if (!all->cmds)
		return(print_merror(all));
	i = 0;
	while(i < all->cmds_num)
	{
		all->cmds[i].args_num = argv_len(argv[i]);
		all->cmds[i].argv = argv[i];
		i++;
	}
	return (0);
}

void	free_cmd(t_all *all)
{
	int i;
	int j;

	i = 0;
	if (all->cmds)
	{
		while (i < all->cmds_num)
		{
			j = 0;
			while(all->cmds[i].argv[j])
			{
				if (all->cmds[i].argv[j])
					free(all->cmds[i].argv[j]);
				j++;
			}
			free(all->cmds[i].argv);
			i++;
		}
		free(all->cmds);
		all->cmds = NULL;
	}
}

void free_argv(char **argv)
{
	int i;

	i = 0;
	while (argv[i])
	{
		free(argv[i]);
		i++;
	}
	free(argv);
}

void free_struct(t_all *all, char **argv, char ***cmds)
{
	free(argv);
	free(cmds);
	free_cmd(all);
}

int		make_struct(t_all *all, char *str)
{
	char **argv;
	int i;
	char **new_argv;
	char ***cmds;

	i = 0;
	argv = ft_split(str, '|');
	if (!argv)
		return (print_merror(all));
	all->cmds_num = argv_len(argv);
	if (check_argv(all, argv) < 0)
	{
		free_argv(argv);
		return (-1);
	}
	cmds = (char***)malloc(sizeof(char**) * (all->cmds_num + 1));
	if (!cmds)
		return(print_merror(all));
	cmds[all->cmds_num] = NULL;
	while (argv[i])
	{
		new_argv = tokenize(argv[i], all);
		if (!new_argv)
			return (-1);
		if (check_redir(new_argv) < 0)
		{
			free(argv);
			free_argv(new_argv);
			free(cmds);
			return (-1);
		}
		cmds[i] = new_argv;
		i++;
	}
	i = 0;
	if (m_struct(all, cmds) < 0)
		return (print_merror(all));
	execute_cmd(all);
	free_struct(all, argv, cmds);
	return (0);
}

char	*var_replace(t_all *all, char *str, char *d_pointer, char *end)
{
	char *tmp1;
	char *tmp2;

	if ((tmp2 = get_env_val(all->envp, end)))
	{
		tmp1 = ft_substr(str, 0, d_pointer - str);
		tmp1 = my_strjoin(tmp1, tmp2);
		tmp1 = my_strjoin(tmp1, after_var(d_pointer + 1));
		if (!tmp1 || !end)
		{
			print_merror(all);
			return (NULL);
		}
		free(str);
		str = tmp1;
	}
	return (str);
}

char	*if_no_var(t_all *all, char *str, char *d_pointer, char *end)
{
	char *tmp1;
	char *tmp2;

	tmp2 = get_env_val(all->envp, end);

	if (!tmp2)
	{
		tmp1 = ft_substr(str, 0, d_pointer - str);
		if (!tmp1)
		{
			print_merror(all);
			return (NULL);
		}
		if (ft_isalnum((int)*(d_pointer + 1)) || (char)*(d_pointer + 1) == '_')
			tmp1 = my_strjoin(tmp1, after_var(d_pointer + 1));
		else
		{
			*d_pointer *= -1;
			tmp1 = my_strjoin(tmp1, d_pointer);
		}
		if (!tmp1)
		{
			print_merror(all);
			return (NULL);
		}
		free(str);
		str = tmp1;
	}
	return (str);
}

int		make_cmd(t_all *all)
{
	char **cmds;
	int i;
	char *d_pointer;
	char *end;

	i = 0;
	d_pointer = NULL;
	cmds = ft_split(all->src->str, ';');
	while(cmds[i])
	{
		while ((d_pointer = ft_strchr(cmds[i], '$')))
		{
			end = end_var(d_pointer + 1);
			if (!end)
				return(print_merror(all));
			cmds[i] = var_replace(all, cmds[i], d_pointer, end);
			cmds[i] = if_no_var(all, cmds[i], d_pointer, end);
			free(end);
		}
		make_struct(all, cmds[i]);
		free(cmds[i]);
		i++;
	}
	free(cmds);
	return (0);
}

int		parser(t_all *all)
{
	t_line src;
	int		ret;
	
	if (all->history == NULL || all->hist_len <= 0)
		return (0);
	ft_bzero(&all->val, sizeof(all->val));
	src.str = ft_strdup(all->history[all->hist_len - 1]);
	src.len = ft_strlen(src.str);
	src.pos = 0;
	ret = shield(all, &src);
	if (ret < 0)
	{
		free(src.str);
		return (0);
	}
	ret = remove_ch(&src);
	if (ret < 0)
	{
		free(src.str);
		print_merror(all);
	}
	all->src = &src;
	make_cmd(all);
	free(src.str);
	return (ret);
}	

void up_arrow(t_all *all)
{
	if (all->hist_len > 1)
	{
		tputs(restore_cursor, 1, ft_putint);
		print_previus(all);
		tputs(tigetstr("ed"), 1, ft_putint);
	}
}

void	down_arrow(t_all *all)
{
	tputs(restore_cursor, 1, ft_putint);
	print_next(all);
	tputs(tigetstr("ed"), 1, ft_putint);
}

void	backspace_key(t_all *all)
{
	if (ft_strlen(all->history[all->pos]) > 0)
	{
		tputs(cursor_left, 1, ft_putint);
		all->history[all->pos][ft_strlen(all->history[all->pos]) - 1] = '\0';
		tputs(tgetstr("dc", 0), 1, ft_putint);
	}
}

void 	canon(t_all *all)
{
	all->term.c_lflag |= ECHO;
	all->term.c_lflag |= ICANON;
	all->term.c_lflag |= ISIG;
	tcsetattr(0, TCSANOW, &all->term);
}

void	new_env(t_all *all)
{
	t_list *tmp_list;
	t_env *env;
	char **new_env;
	int i;

	tmp_list = all->envp;
	i = 0;
	new_env = calloc(ft_lstsize(tmp_list) + 1, sizeof(char*));
	while (tmp_list)
	{
		env = tmp_list->content;
		new_env[i] = ft_strjoin(env->key, "=");
		new_env[i] = my_strjoin(new_env[i], env->value);
		tmp_list = tmp_list->next;
		i++;
	}
	all->env_strs = new_env;
}

void	shlvl_ini(t_all *all)
{
	char *shlvl;
	int val;
	t_list *tmp;
	t_env *env;

	shlvl = NULL;
	shlvl = get_env_val(all->envp, "SHLVL");
	if (shlvl != NULL)
		val = ft_atoi(shlvl);
	else
		return ;
	if (val < 0)
		add_key(all->envp, "SHLVL", "0");
	else if (val >= 1000)
	{
		printf("minishell: warning: shell level (%d) too high, resetting to 1\n", val + 1);
		add_key(all->envp, "SHLVL", "1");
	}
	else
	{	
		val++;
		add_key(all->envp, "SHLVL", ft_itoa(val));
	}
	if (shlvl)
		free(shlvl);
}

void ctrl_d_term(t_all *all)
{
	canon(all);
	save_history(all);
	printf("exit\n");
	exit(0);
}

int read_line(t_all *all, char *str)
{
	int len;

	while (ft_strncmp(str, "\n", 2))
	{
		len = read(0, str, 1000);
		str[len] = 0;
		if (!ft_strncmp(str, "\e[A", 5))
			up_arrow(all);
		else if (!ft_strncmp(str, "\e[B", 5))
			down_arrow(all);
		else if (!ft_strncmp(str, "\177", 5))
			backspace_key(all);
		else if (!ft_strncmp(str, "\e[D", 5))
			continue;
		else if (!ft_strncmp(str, "\e[C", 5))
			continue;
		else if (!ft_strncmp(str, "\4", 2) && ft_strlen(all->history[all->pos]))
			continue;
		else if (!ft_strncmp(str, "\4", 2) && !ft_strlen(all->history[all->pos]))
			ctrl_d_term(all);
		else if (!ft_strncmp(str, "\03", 2))
		{
			canon(all);
			write(1, "\n", 1);
			break;
		}
		else
		{
			if (hist_strjoin(all, str) < 0)
				return (-1);
			write(1, str, len);
		}
	}
	return (0);
}

int		main(int ac, char **av, char **envp)
{
	t_all *all;
	int len;
	char str[2000];

	all = init_all(envp);
	shlvl_ini(all);;
	while(1) 
	{
		new_line(all, str);
		nocanon(all);
		read_line(all, str);
		if (!ft_strncmp(str, "\n", 2))
		{
			canon(all);
			parser(all);
			save_history(all);
		}
	}
	ft_putchar_fd('\n', 1);
	return (0);
}