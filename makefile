GCC = gcc -g

all: shell.c
	$(GCC) shell.c -o shell -lreadline

clean:
	rm -f *~ shell

run: shell
	./shell
