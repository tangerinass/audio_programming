.PHONY:read_input controls

all: clean read_input controls move

read_input:
	$(MAKE) -C ./read_input/
	
controls:
	$(MAKE) -C ./controls/

move:
	mkdir -p build
	mv ./read_input/main ./build/read_input
	mv ./controls/main ./build/controls

clean:
	rm -f build/controls build/read_input
