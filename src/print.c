/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pking <pking@student.42angouleme.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 15:38:46 by pking             #+#    #+#             */
/*   Updated: 2026/11/17 16:17:56 by pking            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philosophers.h"

// 8.4 sets the stop bool
// mutex here is for preventing race condition read and writing to the stop bool 
void set_stop(t_data *data)
{
    pthread_mutex_lock(&data->stop_lock);
    data->stop = 1;
    pthread_mutex_unlock(&data->stop_lock);
}

// 9.4: locks, then prints timestamped *msg
void print_state(t_philo *philo, char *msg)
{
    t_data *data;

    data = philo->data;
    pthread_mutex_lock(&data->print_lock);
    if (!is_stopped(data))
        printf("%ld %d %s\n", now_ms() - data->time_start, philo->id, msg);
    pthread_mutex_unlock(&data->print_lock);
}

// 10.2
// this is used only when a philo in the monitor is detected as having died
void print_death(t_data *data, int id)
{
    pthread_mutex_lock(&data->print_lock);
    set_stop(data);
    printf("%ld %d died\n", now_ms() - data->time_start, id);
    pthread_mutex_unlock(&data->print_lock);
}