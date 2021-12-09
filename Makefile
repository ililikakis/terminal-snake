CC = g++
CFLAGS =
INCLUDES =
LFLAGS =
LIBS = -lncurses
SRCS = snake.cxx

OBJS = $(SRCS:.c=.o)

NAME = snake

.PHONY: depend clean

all: $(NAME)
	@echo  $(NAME) has been compiled.

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(NAME) $(OBJS) $(LFLAGS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) *.o *~ $(NAME)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

