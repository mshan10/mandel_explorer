all: mandel mandelmovie

mandelmovie: mandelmovie.c
	gcc -Wall mandelmovie.c -lm -o mandelmovie

mandel: mandel.o bitmap.o
	gcc mandel.o bitmap.o -o mandel -lpthread -lm

mandel.o: mandel.c
	gcc -Wall -g -c mandel.c -o mandel.o

bitmap.o: bitmap.c
	gcc -Wall -g -c bitmap.c -o bitmap.o

clean:
	rm -f mandel.o bitmap.o mandel mandel.mpg
	ls | grep -P "mandel[0-9]+.bmp" | xargs -d"\n" rm
