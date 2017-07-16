all: snake

snake: snake.o
	gcc -o $@ $^ -lncurses

snake.o: snake.c
	gcc -Wall -c -g -o $@ $<

clean:
	rm -rf snake.o snake.exe snake
