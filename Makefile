all: snake

snake: snake.o
	gcc -o $@ $^ -lncurses

snake.o: snake.c
	gcc -Wall -c -g -o $@ $<

tags: snake.c
	ctags -R .
	cscope -bU

clean:
	rm -rf snake.o snake.exe snake tags cscope.out
