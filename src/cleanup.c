/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cleanup.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pking <pking@student.42angouleme.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/11 17:48:46 by pking             #+#    #+#             */
/*   Updated: 2026/11/12 17:38:31 by pking            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philosophers.h"

// joins every started thread and destroys mutexes
void cleanup (t_data *data)
{
	int i;

	i = 0;
	while (data->philo && i < data->num_of_philos)
		pthread_join(data->philo[i++].thread, NULL);
	i = 0;
	while (data->philo && i < data->num_of_philos)
	{
		pthread_mutex_destroy(&data->forks[i]);
		pthread_mutex_destroy(&data->philo[i].meal_lock);
		i++;
	}
	pthread_mutex_destroy(&data->print_lock);
	pthread_mutex_destroy(&data->stop_lock);
	free(data->forks);
	free(data->philo);
}