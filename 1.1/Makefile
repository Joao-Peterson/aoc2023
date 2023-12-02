CC=gcc
C_FLAGS=-g
I_FLAGS=-I./

SRCS=main.c

.PHONY : main

build: main 

main : $(SRCS:.c=.o)
	@$(CC) $^ -o $@
# @cat test.txt | ./main

%.o : %.c
	@$(CC) $(C_FLAGS) $(I_FLAGS) -c $^ -o $@