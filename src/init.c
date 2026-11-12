/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pking <pking@student.42angouleme.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/04 15:18:22 by pking             #+#    #+#             */
/*   Updated: 2026/11/12 18:03:54 by pking            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philosophers.h"

// 1.5 init_data(t_data *data): bzero for the struct
void init_data(t_data *data)
{
	memset(data, 0, sizeof(*data));
	data->num_times_philos_eat = -1;
}
//this function creates the array of philos and array of forks
static	int	init_mutexes(t_data *data)
{
	int i;

	i = 0;
	data->forks = malloc(sizeof(pthread_mutex_t) * data->num_of_philos);
	data->philo = malloc(sizeof(t_philo) * data->num_of_philos);
	if (!data->forks || !data->philo)
		return (1);
	if (pthread_mutex_init(&data->print_lock, NULL))
		return (1);
	if (pthread_mutex_init(&data->stop_lock, NULL))
		return (1);
	while (i < data->num_of_philos)
	{
		if (pthread_mutex_init(&data->philo[i].meal_lock, NULL))
			return (1);
		if (pthread_mutex_init(&data->forks[i], NULL))
			return (1);
		i++;
	}
	return (0);
}
// 8.3 init_philos: fills each philo (id, forks, last_meal, ptr to data)
void init_philos(t_data *data)
{
	int i;

	i = 0;
	while (i < data->num_of_philos)
	{
		data->philo[i].id = i + 1;
		data->philo[i].meals = 0;
		data->philo[i].last_meal = data->time_start;
		data->philo[i].lfork = &data->forks[i];
		data->philo[i].rfork = &data->forks[(i + 1) % data->num_of_philos];
		data->philo[i].data = data;
		i++;
	}

}

// 8. init_simulation: 8.1 Mutexes -> 8.2 start time -> 8.3 philos -> launch threads
// pthread_create: this is used to store a thread at each philo's thread's
// 					pthread_t. 
int	init_simulation(t_data *data)
{
	int i;

	if (init_mutexes(data))
		return (1);
	data->time_start = now_ms();
	init_philos(data);
	i = 0;
	while (i < data->num_of_philos)
	{
		if (pthread_create(&data->philo[i].thread, NULL,
				philo_routine, &data->philo[i]))
		{
			set_stop(data);
			data->num_of_philos = i;
			return (1);
		}
		i++;
	}
	return (0);
}
