#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define INPUT_MAX 1024

char **parse(char input[INPUT_MAX]) {
	char *s = input;
	s = strsep(&s, "\n");
	
	char **command = (char **) malloc(sizeof(char *) * INPUT_MAX / 2 + 1);
	int i = 0;

	while (command[i++] = strsep(&s, " "));

	command[i] = 0;
	
	return command;
}

void runNext() {
	printf("@ ");
	char input[INPUT_MAX];
	fgets(input, sizeof(char) * INPUT_MAX, stdin);
	char **command = parse(input);

	if (fork()) {
		wait(NULL);
	} else {
		execvp(command[0], command);
	} 
}

static void sighandler(int signo) {
	switch (signo) {
	case SIGINT:
		exit(-1);
		break;
	}
}

int main() {
	int isRunning = 1;
	signal(SIGINT, sighandler);
	
	while (isRunning) {
		runNext();
	}
	
	return 0;
}
