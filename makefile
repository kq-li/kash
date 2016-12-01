GCC = gcc -g

all: shell.c shell.h
	$(GCC) shell.c -o shell -lreadline

clean:
	rm -f *~ shell

run: shell
	./shell
