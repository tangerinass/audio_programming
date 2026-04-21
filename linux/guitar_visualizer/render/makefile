all: clean sphere wave
sphere:sphere.c
	cc sphere.c common/music.c common/parse.c -o sphere -lm
wave:wave.c
	cc wave.c common/music.c common/parse.c -o wave -lm
clean:
	rm -f sphere wave
