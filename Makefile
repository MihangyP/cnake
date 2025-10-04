NAME = cnake
SRC_FILES := cnake.c itoa.c trace_log.c
SRC_FILES := $(addprefix src/, $(SRC_FILES))
OBJ_FILES = $(SRC_FILES:.c=.o)
HEADER_FILES := cnake.h
HEADER_FILES := $(addprefix include/, $(HEADER_FILES))
CC = cc -g
CFLAGS = -Wall -Wextra # -Werror
INCLUDING = -I./include
LINKING = 
RM = rm -rf
BUILD_FOLDER = ./build

all: $(NAME)

$(NAME): $(SRC_FILES) $(HEADER_FILES)
	mkdir -p $(BUILD_FOLDER)
	$(CC) $(CFLAGS) $(INCLUDING) $(SRC_FILES) -o $(BUILD_FOLDER)/$(NAME) $(LINKING)

clean:
	$(RM) $(OBJ_FILES)

fclean: clean
	$(RM) $(BUILD_FOLDER)

re: fclean all

.PHONY: all clean fclean re
