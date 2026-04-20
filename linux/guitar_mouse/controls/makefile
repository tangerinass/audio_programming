all: clean main

main: main.c
	cc $(shell pkg-config --cflags --libs libevdev) main.c ../common/common.c -o main 

clean:
	rm -f main
