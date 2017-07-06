all: snake

snake: snake.o getkey.o
	gcc -o $@ $^

snake.o: snake.c getkey.h
	gcc -o $@ -c $< -Wall

getkey.o: getkey.c getkey.h
	gcc -o $@ -c $< -Wall

clean:
	rm -rf snake.o getkey.o snake.exe snake
