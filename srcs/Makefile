NAME = ssu_ext2

CC = gcc
CFLAGS = -g
RM = rm -f

SRC_FILES = ssu_ext2.c help.c
SRC_TREES = tree.c
SRC_PRINTS = print.c
SRC_UTILS = utils_split.c debug.c parse.c validate.c
SRC_EXT2 = ext2_utils.c ext2_inode.c

SRCS = $(SRC_FILES) $(SRC_TREES) $(SRC_PRINTS) $(SRC_UTILS) $(SRC_EXT2) 
OBJS := $(SRCS:.c=.o)

all : $(NAME)

$(NAME) : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

start : $(OBJS)
	@echo "Object files created"

clean :
	$(RM) $(OBJS)

fclean :
	$(RM) $(OBJS)
	$(RM) $(NAME)

re :
	make fclean
	make all

.PHONY : all start clean fclean re