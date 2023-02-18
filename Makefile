build/instancer: src/*.c src/*.h build
	cc -o $@ src/*.c

build:
	mkdir build
