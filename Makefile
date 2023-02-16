build/instancer: src/instancer.c build
	cc -o $@ $<

build:
	mkdir build
