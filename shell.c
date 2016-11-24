#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#define INPUT_MAX 1024
#define NORMAL 0
#define DELIMITER 1
#define ESCAPE 2

static void sighandler(int signo) {
	switch (signo) {
	case SIGINT:
		exit(-1);
		break;
	}
}

char *removeChar(char *s, char c) {
	int pos = 0;

	while (*(s + pos++) != c);
	pos--;
	
	if (*(s + pos)) {
		char *ret = (char *) calloc(sizeof(char), strlen(s) - 1);
		strncpy(ret, s, pos++);
		strcat(ret, s + pos);
		return ret;
	} else {
		return s;
	}	
}

char **splitOnChar(char *str, char delim, char escRegion, char escOne) {
	char **ret = (char **) calloc(sizeof(char *), strlen(str));
	int state = 0;
	int i = 0;
	char *trail = str;
//	printf("*%s*\n", str);

	while (*str) {
		switch (state) {
		case NORMAL:
			if (*str == escOne) {
				trail = removeChar(trail, escOne);
				str++;
			} else if (*str == escRegion) {
				state = ESCAPE;
			} else if (*str == delim) {
				state = DELIMITER;
				*str = 0;
				ret[i++] = trail;
			}

			break;

		case DELIMITER:
			if (*str == delim) {
				*str = 0;
			} else if (*str == escRegion) {
				state = ESCAPE;
				*str = 0;
				trail = str + 1;
			} else {
				state = NORMAL;
				trail = str;
			}

			break;

		case ESCAPE:
			if (*str == escOne) {
				str++;
			} else if (*str == escRegion) {
				state = NORMAL;
				*str = 0;
			}
			
			break;
		}

		str++;
	}

	ret[i] = trail;

//	char **cp = ret;

//	printf("*");

//	if (*cp) {
//		printf("%s", *(cp++));
//	}
	
//	while (*cp) {
//		printf("_%s", *(cp++));
//	}

//	printf("*\n");
	
	return ret;
}

char **parseInput(char input[INPUT_MAX]) {
	char *s = input;
	//strip terminating newline
	*(strrchr(input, '\n')) = 0;
	
	char **command = splitOnChar(s, ' ', '"', '\\');
	return command;
}

void runNext() {
	char cwd[128];
	getcwd(cwd, 128);
	printf("%s@ ", cwd);
	char input[INPUT_MAX];
	fgets(input, sizeof(char) * INPUT_MAX, stdin);
	char **command = parseInput(input);

//	char **s = command;
//	printf("%s", *(s++));

//	while (*s) {
//		printf("_%s", *(s++));
//	}

//	printf("\n");

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
