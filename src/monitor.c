/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pking <pking@student.42angouleme.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 15:23:40 by pking             #+#    #+#             */
/*   Updated: 2026/11/18 21:42:08 by pking            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philosophers.h"

int	check_philo(t_data *data, t_philo *philo, int *fed)
{
	int	dead;

	pthread_mutex_lock(&philo->meal_lock);
	dead = (now_ms() - philo->last_meal > data->time_to_die);
	if (data->num_times_philos_eat < 0
		|| philo->meals < data->num_times_philos_eat)
		*fed = 0;
	pthread_mutex_unlock(&philo->meal_lock);
	return (dead);
}

// runs in main thread
// cant reaturn a function
void	monitor(t_data *data)
{
	int	i;
	int	fed;

	while (1)
	{
		i = 0;
		fed = 1;
		while (i < data->num_of_philos)
		{
			if (check_philo(data, &data->philo[i], &fed))
			{
				print_death(data, data->philo[i].id);
				return ;
			}
			i++;
		}
		if (fed)
		{
			set_stop(data);
			return ;
		}
		usleep(1000);
	}
}
