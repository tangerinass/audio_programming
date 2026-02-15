all: clean main

main: source/main.c
	cc source/main.c source/fft.c -o main -lm
clean:
	rm -f main
