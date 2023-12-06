CC=gcc
C_FLAGS=-g
I_FLAGS=-I./
L_FLAGS=-lpthread

SRCS=main.c
SRCS+=string+.c

.PHONY : main

build: main 

main : $(SRCS:.c=.o)
	@$(CC) $(L_FLAGS) $^ -o $@
# @cat input.txt | ./main

%.o : %.c
	@$(CC) $(C_FLAGS) $(I_FLAGS) -c $^ -o $@

mem : main
	valgrind -s --leak-check=full --show-leak-kinds=all --track-origins=yes ./$< input.txt

clear :
	@rm -vf *.o 
	@rm -vf main 