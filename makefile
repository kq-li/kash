GCC = gcc -g

all: shell.c
	$(GCC) shell.c -o shell

clean:
	rm -f *~ shell

run: shell
	./shell
