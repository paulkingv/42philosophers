NAME	= philo

SRC_DIR	= ./src
INC_DIR	= ./inc
OBJ_DIR	= ./obj

SRC		= main.c \
		  parse.c \
		  init.c \
		  time.c \
		  print.c \
		  philo_routine.c \
		  monitor.c \
		  cleanup.c \
		  utils/ft_is_digit.c \
		  utils/ft_atoi.c

OBJ		= $(addprefix $(OBJ_DIR)/, $(SRC:.c=.o))
DEP		= $(OBJ:.o=.d)

CC		= cc
CFLAGS	= -Wall -Werror -Wextra -g -pthread -I$(INC_DIR) -MMD -MP
LDFLAGS	= -pthread

# Debug helper: make re SAN=thread   (data races)
#               make re SAN=address  (leaks / memory errors)
# Never combine both, and run `make fclean` when switching.
ifdef SAN
CFLAGS	+= -fsanitize=$(SAN)
LDFLAGS	+= -fsanitize=$(SAN)
endif

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

-include $(DEP)

.PHONY: all clean fclean re
