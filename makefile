GCC = gcc -g

all: kash.c kash.h
	$(GCC) kash.c -o kash -lreadline

clean:
	rm -f *~ kash

run: kash
	./kash
