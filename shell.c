#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#define INPUT_MAX 1024

static void sighandler(int signo) {
	switch (signo) {
	case SIGINT:
		exit(-1);
		break;
	}
}

char **split(char *s, char d) {
	char **ret = (char **) calloc(sizeof(char *), strlen(s));
	int state = 0;
	int i = 0;
	char *trail = s;
	printf("*%s*\n", s);

	// state 1 if currently looking through delimiters, 0 otherwise
	while (*s) {
		if (state) {
			if (*s != d) {
				state = 0;
				trail = s;
			} else {
				*s = 0;
			}
		} else {
			if (*s == d) {
				state = 1;
				*s = 0;
				ret[i++] = trail;
			}
		}

		s++;
	}

	ret[i] = trail;

	char **cp = ret;

	printf("*");
	while (*cp) {
		printf("%s_", *(cp++));
	}

	printf("*\n");
	
	return ret;
}

char **parse(char input[INPUT_MAX]) {
	char *s = input;
	//strip terminating newline
	*(strrchr(input, '\n')) = 0;
	
	char **command = split(s, ' ');
	return command;
}

void runNext() {
	char cwd[128];
	getcwd(cwd, 128);
	printf("%s@ ", cwd);
	char input[INPUT_MAX];
	fgets(input, sizeof(char) * INPUT_MAX, stdin);
	char **command = parse(input);

	char **s = command;
	printf("%s", *(s++));

	while (*s) {
		printf("_%s", *(s++));
	}

	printf("\n");

	if (strcmp(command[0], "cd") == 0) {
		if (command[1]) {
			if (chdir(command[1]) != 0) {
				printf("Error %d: %s\n", errno, strerror(errno));
			}
		} else {
			printf("%s\n", getenv("HOME"));
			if (chdir(getenv("HOME")) != 0) {
				printf("Error %d: %s\n", errno, strerror(errno));
			}
		}
	} else if (strcmp(command[0], "exit") == 0) {
		printf("exit\n");
	} else if (fork()) {
		wait(NULL);
	} else {
		execvp(command[0], command);
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
