/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pking <pking@student.42angouleme.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/02 14:18:50 by pking             #+#    #+#             */
/*   Updated: 2026/11/18 21:43:21 by pking            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philosophers.h"

// time util to get the current time in MS (converts from micro)
long	now_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

//  safe read of stop flag. Used in other functs to determine 
//  if we have run set_stop
// (stop gets set in print_death, inside the main monitor)
int	is_stopped(t_data *data)
{
	int	stop_clone;

	pthread_mutex_lock(&data->stop_lock);
	stop_clone = data->stop;
	pthread_mutex_unlock(&data->stop_lock);
	return (stop_clone);
}

// uses usleep in small slices so we wake quickly when the sim stops
void	ms_sleep(t_data *data, long ms)
{
	long	end;

	end = now_ms() + ms;
	while (!is_stopped(data) && now_ms() < end)
		usleep(500);
}
