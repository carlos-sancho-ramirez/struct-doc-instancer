build/instancer: src/*.c src/*.h src/formatters/*.c src/formatters/*.h build
	cc -o $@ src/*.c src/formatters/*.c

build:
	mkdir build
