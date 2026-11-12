/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_is_digit.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pking <pking@student.42angouleme.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/20 14:15:09 by pking             #+#    #+#             */
/*   Updated: 2026/10/23 13:21:03 by pking            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "philosophers.h"

// 4. tells if its ascii number. 1 = TRUE
int ft_is_digit(char num)
{
    // int i;
    // i = 0;

    // while (num[i] != '\0')
    // {
        if (num < 48 || num > 57)
            return (0);
        // i++;
    // }
    return (1);
}