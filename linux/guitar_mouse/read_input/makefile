all: clean main

main: source/main.c
	cc source/main.c source/fft.c source/music.c ../common/common.c -o main -lm
clean:
	rm -f main main.exe
