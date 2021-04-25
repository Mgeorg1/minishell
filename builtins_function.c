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
				tmp->value = ft_strdup(tmp_str + 1);
				ft_lstadd_back(&lst, ft_lstnew(tmp));
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

void    init_shlvl(t_list *envp, t_list *exp)
{
    char *value;
    value = search_key(exp, "SHLVL");
    printf("%s\n", value);
//     set_value(envp, "SHLVL", ft_itoa((ft_atoi(value)) + 1));
//     set_value(exp, "SHLVL", ft_itoa((ft_atoi(value)) + 1));
//     set_value(envp, "OLDPWD", "");
//     set_value(exp, "OLDPWD", "");
//     free(value);
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
        {
            // del_key(&envp, cmd->argv[i]);
            del_key(&exp, cmd->argv[i]);
        }
    }
    // write(1,"\n",1);
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

int     max_len(char *s, char *s1)
{
    int len = ft_strlen(s);
    int len1 = ft_strlen(s1);

    if (len >= len1)
        return (len);
    return (len1);
}

// ПЕРЕПИСАТЬ ФУНКЦИЮ! ВАЖНО!
char	**ft_sort(char **env, int size)
{
	int		i;
	int		j;
	char	*swap;

	i = 0;
	j = 1;
	while (j <= size - 1)
	{
		while (i <= size - 1 - j)
		{
			if (ft_strncmp(env[i], env[i + 1], is_min(is_delim(env[i]), is_delim(env[i + 1]))) > 0)
			{
				swap = env[i];
				env[i] = env[i + 1];
				env[i + 1] = swap;
			}
			i++;
		}
		i = 0;
		j++;
	}
	return (env);
}
// ПЕРЕПИСАТЬ ФУНКЦИЮ! ВАЖНО!
// export a1=123
// export a2=333 
// export a1+=$a2 // 123333
// export a1+=a2 // 123a2

// void    ft_export(t_cmd *cmd, int i, int j)
// {
//     int len;
//     int i;

//     len = len_arr(cmd->argv);
//     i = 0;
//     while(cmd->argv[i][j])
//     {
//         if (cmd->argv[i][j] == '+' && cmd->argv[i][j + 1] == '=')

//     }
// }

//Возможные варианты, 
// export abc=123
//export abc
//  если в переменная abc уже есть, ее содержимое остается без изменнений
void    ft_export(t_cmd *cmd, t_list *envp, t_list *exp)
{
    int len;
    int i;
    char *key;
    char *value;

    len = len_arr(cmd->argv);
    i = 0;
    if (cmd->argv[1] == NULL)
        print_export(&envp);
    else
    {
        while(++i < len && cmd->argv[i])
        {
            int first_space = is_delim(cmd->argv[i]);
            key = ft_substr(cmd->argv[i], 0, first_space);
            //
            write(1,key,ft_strlen(key));
            //
			if (first_space != 0)
            	value = ft_substr(cmd->argv[i], ++first_space, ft_strlen(cmd->argv[i]));
			else
				value = ft_strdup("");
            //
            write(1, value,ft_strlen(value));
            //
            add_key(envp, key, value);
            add_key(exp, key, value);
        }
        // free(key);
        // free(value);
    }
    // print_env(envp);
}