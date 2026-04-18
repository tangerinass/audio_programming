all: clean main

main: source/main.c
	cc source/main.c source/fft.c source/music.c -o main -lm
clean:
	rm -f main main.exe
