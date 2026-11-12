/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_atoi.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pking <pking@student.42angouleme.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 16:56:00 by pking             #+#    #+#             */
/*   Updated: 2026/10/23 15:15:26 by pking            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philosophers.h"

// 6. ft_atoi: negative condition will never hit.
int	ft_atoi(const char *str)
{
	int	    i;
	int	    sign;
	size_t	num_storage;

	i = 0;
	sign = 1;
	num_storage = 0;
	while (str[i] == ' ' || (str[i] >= 9 && str[i] <= 13))
		i++;
	if ((str[i] == '-') || (str[i] == '+'))
	{
		if (str[i] == '-')
			sign *= -1;
		i++;
	}
    if (str[0] == '\0')
        return (-1);
    while (ft_is_digit(str[i]))
	{
		num_storage = num_storage * 10 + (str[i] - '0');
        if (num_storage > 2147483647)
            return (-1);
		i++;
	}
	return (num_storage * sign);
}
