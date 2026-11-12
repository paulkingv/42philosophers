/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philosophers.h                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pking <pking@student.42angouleme.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/19 16:07:17 by pking             #+#    #+#             */
/*   Updated: 2026/11/12 17:31:58 by pking            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PHILOSOPHERS_H
# define PHILOSOPHERS_H

#include <stdio.h> // printf
#include <stdlib.h> // malloc
#include <unistd.h> // usleep, write, fork
#include <string.h> // memset
#include <sys/time.h> // gettimeofday 
#include <pthread.h> // pthread stuff
#include <semaphore.h> // IDK sem_ stuff , maybe used


//**********************************STRUCTS***********************************//

typedef	struct	s_philo	t_philo;
typedef	struct	s_data	t_data;

struct	s_data {
	ssize_t			time_start;
	int				num_of_philos;
	int				time_to_die;
	int				time_to_eat;
	int				time_to_sleep;
	int				num_times_philos_eat;
	int				stop;
	pthread_mutex_t print_lock;
	pthread_mutex_t stop_lock;
	pthread_mutex_t *forks;		
	t_philo		*philo;
};

struct	s_philo {
	int					id;
	int					meals;
	ssize_t				last_meal;
	pthread_t			thread;
	pthread_mutex_t		meal_lock;
	pthread_mutex_t		*lfork;
	pthread_mutex_t		*rfork;
	t_data				*data;
};




//******************************FOLDERS/FUNCTIONS*****************************//
//------------/SRC/PARSE.C------------//


int fill_data(t_data *data, int arg, int i);
int convert_to_ascii(t_data *data, int argc, char **argv);
int validate_arg(char *arg);
int parse(int argc, char **argv);

//-------------/SRC/UTILS/FT_ATOI.C---------------//
int	ft_atoi(const char *str);

//------------/SRC/UTILS/FT_IS_DIGIT.C------------//
int ft_is_digit(char num);

//------------/SRC/INIT.C------------//
void init_data(t_data *data);
int init_simulation(t_data *data);

//-----------------/SRC/TIME.C-------------------//
long now_ms(void);
void ms_sleep(t_data *data, long ms);
int is_stopped(t_data *data);

//-----------------/SRC/PHILO_ROUTINE.C-------------------//
void	one_philo(t_philo *philo);
void	*philo_routine(void *arg);
//-----------------/SRC/PRINT.C-------------------//
void 	set_stop(t_data *data);
void 	print_state(t_philo *philo, char *msg);
void	print_death(t_data *data, int id);

//-----------------/SRC/TIME.C-------------------//
long	now_ms(void);
int		is_stopped(t_data *data);
void	ms_sleep(t_data *data, long ms);

//-----------------/SRC/MONITOR.C-------------------//
void	monitor(t_data *data);

//-----------------/SRC/CLEANUP.C-------------------//
void	cleanup(t_data *data);

#endif