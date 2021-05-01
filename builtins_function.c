#include "includes/minishell.h"

t_list *parse_env(char **env)
{
	t_list *lst;
	t_env *tmp;
	char *tmp_str;

	lst = NULL;

	
	while (*env)
	{
		tmp_str = *env;
		while (*tmp_str)
		{
			if (*tmp_str == '=')
			{
				tmp = (t_env*)malloc(sizeof(t_env));
				if (!tmp)
					return (NULL);
				tmp->key = ft_substr(*env, 0, tmp_str - *env);
				if (tmp->key != NULL)
					tmp->value = ft_strdup(tmp_str + 1);
				// else
				// 	tmp->value = NULL;
				if (ft_strncmp(tmp->key, "OLDPWD", 7))
					ft_lstadd_back(&lst, ft_lstnew(tmp));
				else
					{
						free(tmp->key);
						free(tmp->value);
						free(tmp);
					}
				break;
			}
			tmp_str++;
		}
		env++;
	}
	return (lst);
}

int     ft_isdigit_str(char *str)
{
    int     i;

    i = -1;
    while(str[++i])
        if (!ft_isdigit(str[i]))
            return (0);
    return(1);
}

//Реализовать удаление по-ключу
void    ft_unset(t_cmd *cmd, t_list *envp, t_list *exp)
{
    int len;
    int i;

    len = len_arr(cmd->argv);
    i = 0;
    if (cmd->argv[1] != NULL)
    {
        while(++i < len && cmd->argv[i])
            del_key(&exp, cmd->argv[i]);
            // del_key(&envp, cmd->argv[i]);
    }
}

void    del_key(t_list **lst, char *key)
{
    t_list  *tmp;
    t_env   *env;

    tmp = *lst;
    env = tmp->content;

    t_list  *tmp_next;
    tmp_next = tmp->next;
    if (!ft_strncmp(env->key, key, ft_strlen(key)))
    {
        free((t_env *)(env)->key);
        free((t_env *)(env)->value);
        free(tmp->content);
        tmp = tmp_next;
        free(tmp);
        return ;
    }
    else
    {
        while(tmp)
        {
			if (tmp->next != NULL)
			{
				env = tmp->next->content;
				if (!ft_strncmp(env->key, key, ft_strlen(key)))
				{
					printf("Gatcha!!!\n");
					free((t_env *)(env)->key);
					free((t_env *)(env)->value);
					free(tmp->next->content);
					free((tmp->next));
					tmp->next = tmp->next->next;
					return ;
				}
			}
            tmp = tmp->next;
        }
    }
}

int		is_valid_id(char *key)
{
	int i;

	i = 0;
	while (key[i])
	{
		if (!ft_isalpha(key[i]) && key[i] != '_')
			return (0);
		i++;
	}
	return (1);
}

void	ft_export_arg(t_cmd * cmd, t_list *envp, t_list *exp)
{
	int len;
    int i;
    char *key;
    char *value;
	char *tmp;

	len = len_arr(cmd->argv);
    i = 0;
	while (++i < len && cmd->argv[i])
	{
		int first_space = is_delim(cmd->argv[i]);
		if (first_space != 0)
		{
			if (ft_strlen(cmd->argv[i]) > 1 && cmd->argv[i][first_space - 1] == '+')
			{
				key = ft_substr(cmd->argv[i], 0, first_space - 1);
				tmp = ft_substr(cmd->argv[i], ++first_space, ft_strlen(cmd->argv[i]));
				value = search_key(exp, key);
				if (value == NULL)
					value = ft_strdup("");
				value = my_strjoin(value, tmp);
				add_key(envp, key, value);
				add_key(exp, key, value);
				continue;
			}
			else
				key = ft_substr(cmd->argv[i], 0, first_space);
		}
		else
			key = ft_strdup(cmd->argv[i]);
		if (!is_valid_id(key))
		{
			printf("minishell: export `%s': not a valid identifier\n", key);
			gl_fd[0] = 1;
			free(key);
		}
		// write(1,key,ft_strlen(key));
		//
		if (first_space != 0)
			value = ft_substr(cmd->argv[i], ++first_space, ft_strlen(cmd->argv[i]));
		else
			value = NULL;
		//
		//write(1, value,ft_strlen(value));
		//
		printf("1\n");
		add_key(envp, key, value);
		add_key(exp, key, value);
	}
	// free(key);
	// free(value);
// print_env(envp);
}

void    ft_export(t_cmd *cmd, t_list *envp, t_list *exp)
{
    int len;
    int i;
    char *key;
    char *value;
	char *tmp;

    len = len_arr(cmd->argv);
    i = 0;
    if (cmd->argv[1] == NULL)
        print_export(&exp);
    else
    {
        while(++i < len && cmd->argv[i])
        {
            int first_space = is_delim(cmd->argv[i]);
			if (first_space != 0)
			{
				if (ft_strlen(cmd->argv[i]) > 1 && cmd->argv[i][first_space - 1] == '+')
				{
					key = ft_substr(cmd->argv[i], 0, first_space - 1);
					tmp = ft_substr(cmd->argv[i], ++first_space, ft_strlen(cmd->argv[i]));
					value = search_key(exp, key);
					if (value == NULL)
						value = ft_strdup("");
					value = my_strjoin(value, tmp);
					add_key(envp, key, value);
            		add_key(exp, key, value);
					continue;
				}
				else
            		key = ft_substr(cmd->argv[i], 0, first_space);
			}
			else
				key = ft_strdup(cmd->argv[i]);
            if (!is_valid_id(key))
			{
				printf("minishell: export `%s': not a valid identifier\n", key);
				gl_fd[0] = 1;
				free(key);
			}
           // write(1,key,ft_strlen(key));
            //
			if (first_space != 0)
            	value = ft_substr(cmd->argv[i], ++first_space, ft_strlen(cmd->argv[i]));
			else
				value = NULL;
            //
            //write(1, value,ft_strlen(value));
            //
            add_key(envp, key, value);
            add_key(exp, key, value);
        }
        // free(key);
        // free(value);
    }
    // print_env(envp);
}
