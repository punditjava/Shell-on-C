CC = gcc
FL = -Werror -g -o
FI = shell.c implementation.c
Shell: shell.c implementation.c
	${CC} ${FI} ${FL} $@                