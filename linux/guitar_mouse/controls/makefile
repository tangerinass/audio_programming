all: clean main

main: source/main.c
	cc $(shell pkg-config --cflags --libs libevdev) source/main.c ../common/common.c -o main 

clean:
	rm -f main
