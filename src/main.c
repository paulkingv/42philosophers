/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pking <pking@student.42angouleme.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/19 16:01:47 by pking             #+#    #+#             */
/*   Updated: 2026/11/17 16:33:01 by pking            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philosophers.h"

// 1. main
int main (int argc, char **argv)
{
	t_data  data;

	init_data(&data);
	if (parse(argc, argv) || convert_to_ascii(&data, argc, argv))
	{
		write(2, "Error\n", 6);
		return (1);
	}
	if (init_simulation(&data))
	{
		write(2, "Error\n", 6);
		cleanup(&data);
		return (1);
	}
	monitor(&data);
	cleanup(&data);
	return (0);
}
