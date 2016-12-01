#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "shell.h"

#define INPUT_MAX 1024
#define NORMAL 0
#define DELIMITER 1
#define ESCAPE 2

void copyBetween(char *dest, char *start, char *end) {
	while (start != end) {
		*(dest++) = *(start++);
	}

	dest = 0;
}

void shift(char *str, int offset) {
	char *trail = str;
	str += offset;

	while (*str) {
		*(trail++) = *(str++);
	}

	*trail = 0;
}

void removeFirst(char *str, char *toRemove) {
	char *trail = str;

	while (*str) {
		if (strchr(toRemove, *str)) {
			shift(str, 1);
			break;
		}
	}
}

void removeAll(char *str, char *toRemove) {
	char *trail = str;

	while (*str) {
		if (!strchr(toRemove, *str)) {
			*(trail++) = *str;
		}

		str++;
	}

	*trail = 0;
}

int isEmpty(char *s) {
	while (*s) {
		if (!strchr(" \n", *(s++))) {
			return 0;
		}
	}

	return 1;
}

void stripChars(char *str, char *toStrip, char *escape) {
	if (!str || !(*str)) {
		return;
	}
	
	while (strchr(toStrip, *str)) {
		//printf("str: \"%s\"\n", str);
		shift(str, 1);
	}

	char *end = str + strlen(str) - 1;

	while (strchr(toStrip, *end) && !strchr(escape, *(end - 1))) {
		//printf("str: \"%s\"\n", str);
		*(end--) = 0;
	}

	//printf("str: \"%s\"\n", str);
}

int startsWith(char *str, char *key) {
	return strncmp(str, key, strlen(key)) == 0 ? 1 : 0;
}

// just trust me
char **splitOnChars(char *str, char *delim, char *escRegion, char *escOne) {
	char **ret = (char **) calloc(sizeof(char *), strlen(str));
	char *trail = str;
	int state = 0;
	int i = 0;
	char *escape = (char *) calloc(sizeof(char), strlen(escRegion) + strlen(escOne) + 1);
	strcpy(escape, escRegion);
	strcat(escape, escOne);
	//	printf("*%s*\n", str);

	while (*str) {
		switch (state) {
		case NORMAL:
			if (startsWith(str, escOne)) {
				shift(str, 1);
			} else if (startsWith(str, escRegion)) {
				state = ESCAPE;
				shift(str--, 1);
			} else if (strchr(delim, *str)) {
				state = DELIMITER;
				*str = 0;
				ret[i++] = trail;
				trail = str + 1;
			}

			break;

		case DELIMITER:
			if (strchr(delim, *str)) {
				*str = 0;
				trail = str;
			} else if (startsWith(str, escRegion)) {
				state = ESCAPE;
				shift(str--, 1);
			} else {
				state = NORMAL;
				trail = str--;
			}

			break;

		case ESCAPE:
			if (startsWith(str, escOne)) {
				shift(str, 1);
			} else if (startsWith(str, escRegion)) {
				state = NORMAL;
				shift(str--, 1);
			}
			
			break;
		}

		str++;
	}

	ret[i] = trail;

	//char **cp = ret;

	//printf("*");

	//if (*cp) {
		//printf("%s", *(cp++));
	//}
	
	//while (*cp) {
		//printf("_%s", *(cp++));
	//}

	//printf("*\n");
	
	return ret;
}

char **parseInput(char *input) {
	//strip newline
	char *s = (char *) calloc(sizeof(char), strlen(input));
	strcpy(s, input);
	stripChars(s, " \n", "\\");

	char **command = splitOnChars(s, " ", "\"", "\\");
	return command;
}

void redirStdout(char *input, char *filename) {
	umask(0000);
	int fd = open(filename, O_WRONLY | O_CREAT, 0644);

	if (fd == -1) {
		printf("Error %d: %s\n", errno, strerror(errno));
	} else {
		int stdout = dup(STDOUT_FILENO);
		dup2(fd, STDOUT_FILENO);
		close(fd);
		execute(input);
		dup2(stdout, STDOUT_FILENO);
		close(stdout);
	}
}

void redirStdin(char *input, char *filename) {
	umask(0000);
	int fd = open(filename, O_RDONLY, 0644);

	if (fd == -1) {
		printf("Error %d: %s\n", errno, strerror(errno));
	} else {
		int stdin = dup(STDIN_FILENO);
		dup2(fd, STDIN_FILENO);
		execute(input);
		dup2(stdin, STDIN_FILENO);
		close(stdin);
		close(fd);
	}
}

void redirPipe(char *input1, char *input2) {
	int stdin_copy = dup(STDIN_FILENO);
	int stdout_copy = dup(STDOUT_FILENO);
	int pipeEnds[2];

	if (pipe(pipeEnds) == -1) {
		printf("Error %d: %s\n", errno, strerror(errno));
		return;
	}

	//printf("in: %d, out: %d\n", pipeEnds[0], pipeEnds[1]);
	
	//printf("Redirecting stdout to pipe read\n");
	dup2(pipeEnds[1], STDOUT_FILENO);

	close(pipeEnds[1]);
	
	//dprintf(stdout_copy, "Executing %s\n", input1);
	execute(input1);

	//dprintf(stdout_copy, "Restoring stdout\n");
	dup2(stdout_copy, STDOUT_FILENO);

	//printf("Redirecting stdin to pipe write\n");
	dup2(pipeEnds[0], STDIN_FILENO);

	close(pipeEnds[0]);

	//printf("Executing %s\n", input2);
	execute(input2);

	//printf("Restoring stdin\n");
	dup2(stdin_copy, STDIN_FILENO);

	close(stdin_copy);
	close(stdout_copy);
}

void execute(char *input) {
	char *s = input;

	while (*s) {
		if (startsWith(s, ">>")) {

		} else if (startsWith(s, "2>")) {

		} else if (startsWith(s, "2>>")) {

		} else if (startsWith(s, "&>")) {
			
		} else if (*s == '>') {
			*(s++) = 0;
			stripChars(s, " \n", "\\");
			redirStdout(input, s);
			return;
		} else if (*s == '<') {
			*(s++) = 0;
			stripChars(s, " \n", "\\");
			redirStdin(input, s);
			return;
		} else if (*s == '|') {
			*(s++) = 0;
			stripChars(s, " \n", "\\");
			redirPipe(input, s);
			return;
		} else if (*s == ';') {
			*(s++) = 0;
			execute(input);
			execute(s);
			return;
		} else {
			s++;
		}
	}
	
	char **command = parseInput(input);
	
	if (strcmp(command[0], "cd") == 0) {
		if (command[1]) {
			if (chdir(command[1]) != 0) {
				switch (errno) {
				case ENOTDIR:
					printf("%s: %s: Not a directory\n", command[0], command[1]);
					break;
					
				case ENOENT:
					printf("%s: %s: No such file or directory\n", command[0], command[1]);
					break;

				default:
					printf("Error %d: %s\n", errno, strerror(errno));
					break;
				}
			}
		} else {
			printf("%s\n", getenv("HOME"));
			
			if (chdir(getenv("HOME")) != 0) {
				printf("Error %d: %s\n", errno, strerror(errno));
			}
		}
	} else if (strcmp(command[0], "exit") == 0) {
		exit(0);
	} else {
		int pid = fork();

		if (pid == -1) {
			printf("Error %d: %s\n", errno, strerror(errno));
		} else if (pid) { //parent
			wait(&pid);
		} else { //child
			if (execvp(command[0], command)) {
				switch (errno) {
				case ENOENT:
					printf("%s: command not found\n", command[0]);
					break;
				default:
					printf("Error %d: %s\n", errno, strerror(errno));
					break;
				}
			}
		}
	} 
}

void prompt() {
	char *cwd = getcwd(NULL, 0);
	char *prefix = (char *) calloc(sizeof(char), strlen(cwd) + 1);
	sprintf(prefix, "%s @ ", cwd);
	char *input = readline(prefix);
	stripChars(input, " \n", "\\");

	if (input && *input) {
		execute(input);
		add_history(input);
	}
}

int main() {
	//char input[32];
	//strcpy(input, " *Hello World! * ");
	//stripChars(input, " *", "\\");
	//printf("\n%s_\n", input);
	
	int isRunning = 1;
	
	while (isRunning) {
		prompt();
	}

	return 0;
}
