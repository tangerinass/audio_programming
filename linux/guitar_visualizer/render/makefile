all: clean sphere
sphere:source/sphere.c
	cc source/sphere.c ../common/common.c source/read_fifo.c -o sphere -lm
clean:
	rm -f sphere
