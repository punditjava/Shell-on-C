CC = gcc
CFLAGS = -Wall -Wextra -g

FI = shell.c implementation.c

Shell: $(patsubst %.c, %.o, $(FI))
	$(CC) $(CFLAGS) -o $@ $^

%.c : shell.h

%.o : %.c
	$(CC) $(CFLAGS) -c $^

clean:
	rm -f Shell *.o
