/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pking <pking@student.42angouleme.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 14:01:42 by pking             #+#    #+#             */
/*   Updated: 2026/11/18 21:45:47 by pking            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philosophers.h"

// 7. fill_data: depending on which arg we are, we know which to fill
int	fill_data(t_data *data, int arg, int i)
{
	if (!data)
		return (1);
	if (i == 0)
		data->num_of_philos = arg;
	if (i == 1)
		data->time_to_die = arg;
	if (i == 2)
		data->time_to_eat = arg;
	if (i == 3)
		data->time_to_sleep = arg;
	if (i == 4)
		data->num_times_philos_eat = arg;
	return (0);
}

// 5. convert_to_ascii: atoi, then fills data struct's corresponding variable
int	convert_to_ascii(t_data *data, int argc, char **argv)
{
	int	args[argc - 1];
	int	i;

	i = 1;
	while (argv[i])
	{
		args[i - 1] = ft_atoi(argv[i]);
		if (args[i - 1] == -1)
			return (1);
		if (fill_data(data, args[i - 1], i - 1))
			return (1);
		i++;
	}
	return (0);
}

// 3. validate_arg: check if
int	validate_arg(char *arg)
{
	int	j;

	j = 0;
	if (!arg)
		return (1);
	if (arg[0] == '-' || arg[0] == '\0')
		return (1);
	if (arg[0] == '+' && ft_is_digit(arg[1]))
		j = 1;
	while (arg[j] != '\0')
	{
		if (!ft_is_digit(arg[j]))
			return (1);
		j++;
	}
	return (0);
}

// 2. parse: check wrong argc, check argv
int	parse(int argc, char **argv)
{
	int	i;

	i = 1;
	if (argc < 5 || argc > 6)
		return (1);
	while (i <= argc - 1)
	{
		if (validate_arg(argv[i]))
			return (1);
		i++;
	}
	return (0);
}
