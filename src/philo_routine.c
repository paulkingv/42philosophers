/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo_routine.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pking <pking@student.42angouleme.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 18:22:50 by pking             #+#    #+#             */
/*   Updated: 2026/11/18 21:42:40 by pking            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philosophers.h"

/*
	Features of a philo
	One -- Eat
	Two -- Sleep

	Three -- Pick up Forks

	If not sleeping, and no forks, they are thinking
*/

// 9.2.1 take forks: odd philos need extra thinking to give others time to eat
// odd philos need extra thinking to give others time to eat
//
static void	take_forks(t_philo *philo)
{
	pthread_mutex_t	*first;
	pthread_mutex_t	*second;

	first = philo->lfork;
	second = philo->rfork;
	if (philo->id % 2 == 0)
	{
		first = philo->rfork;
		second = philo->lfork;
	}
	pthread_mutex_lock(first);
	print_state(philo, "has taken a fork");
	pthread_mutex_lock(second);
	print_state(philo, "has taken a fork");
}

// 9.2 take forks, set last_meal time, print, sleep, release forks
static void	eat(t_philo *philo)
{
	t_data	*data;

	data = philo->data;
	take_forks(philo);
	pthread_mutex_lock(&philo->meal_lock);
	philo->last_meal = now_ms();
	philo->meals++;
	pthread_mutex_unlock(&philo->meal_lock);
	print_state(philo, "is eating");
	ms_sleep(data, data->time_to_eat);
	pthread_mutex_unlock(philo->lfork);
	pthread_mutex_unlock(philo->rfork);
}

// 9.3: print,
// then (odd total philos only) all philos wait extra to avoid dying
// this is the extra padding needed so that each of 3 groups lines up
// for example,  (200 * 200) - 200 = 200, so in a run of 800 200 200,
// all philos have a 200ms think cycle so they line up in group of 3
static void	think(t_philo *philo)
{
	t_data	*data;
	long	extra;

	data = philo->data;
	print_state(philo, "is thinking");
	if (data->num_of_philos % 2 == 0)
		return ;
	extra = 2 * data->time_to_eat - data->time_to_sleep;
	if (extra > 0)
		ms_sleep(data, extra);
}

// 9.1: solo philo will take a fork and wait to die
void	one_philo(t_philo *philo)
{
	pthread_mutex_lock(philo->lfork);
	print_state(philo, "has taken a fork");
	while (!is_stopped(philo->data))
		usleep(500);
	pthread_mutex_unlock(philo->lfork);
}

// 9.
// this is the function that each philo thread runs once launched
// Staggers all even philos so that they can "take turns"
void	*philo_routine(void *arg)
{
	t_philo	*philo;
	t_data	*data;

	philo = (t_philo *)arg;
	data = philo->data;
	if (data->num_of_philos == 1)
	{
		one_philo(philo);
		return (NULL);
	}
	else if (philo->id % 2 == 0)
		ms_sleep(data, data->time_to_eat / 2);
	while (!is_stopped(data))
	{
		eat(philo);
		print_state(philo, "is sleeping");
		ms_sleep(data, data->time_to_sleep);
		think(philo);
	}
	return (NULL);
}
