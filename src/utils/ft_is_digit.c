/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_is_digit.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pking <pking@student.42angouleme.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 14:15:09 by pking             #+#    #+#             */
/*   Updated: 2026/11/18 21:44:43 by pking            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philosophers.h"

// 4. tells if its ascii number
int	ft_is_digit(char num)
{
	if (num < 48 || num > 57)
		return (0);
	return (1);
}
