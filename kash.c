#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "kash.h"

#define INPUT_MAX 1024
#define NORMAL 0
#define DELIMITER 1
#define ESCAPE 2

#define RESET "\x1B[0m"
#define BOLD "\x1B[1m"
#define UNDERLINE "\x1B[4m"
#define RED "\x1B[0;31m"
#define GREEN "\x1B[0;32m"
#define YELLOW "\x1B[0;33m"
#define BLUE "\x1B[0;34m"
#define MAGENTA "\x1B[0;35m"
#define CYAN "\x1B[0;36m"
#define WHITE "\x1B[0;37m"
#define BOLD_RED "\x1B[1;31m"
#define BOLD_GREEN "\x1B[1;32m"
#define BOLD_YELLOW "\x1B[1;33m"
#define BOLD_BLUE "\x1B[1;34m"
#define BOLD_MAGENTA "\x1B[1;35m"
#define BOLD_CYAN "\x1B[1;36m"
#define BOLD_WHITE "\x1B[1;37m"

// Copies contents of string between start and end to dest.
void copyBetween(char *dest, char *start, char *end) {
	while (start != end) {
		*(dest++) = *(start++);
	}

	dest = 0;
}

// Shifts all characters in a string left by offset, truncating characters
// from the beginning. 
void shift(char *str, int offset) {
	char *trail = str;
	str += offset;

	while (*str) {
		*(trail++) = *(str++);
	}

	*trail = 0;
}

// Removes all occurrences of the characters in toStrip from the start and end
// of str except those preceded by a character in escape.
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

// Returns 1 if str starts with key, otherwise 0.
int startsWith(char *str, char *key) {
	return strncmp(str, key, strlen(key)) == 0 ? 1 : 0;
}

// Custom strsep function. Returns a pointer to the first in a series of strings
// terminated by a null pointer, generated by separating str on delimiters
// specified in delim. Wrapping in escRegion or following escOne escapes a
// character.
// Trust me. It works.
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

// Takes a string and splits it on spaces with escape characters.
char **parseInput(char *input) {
	char *s = (char *) calloc(sizeof(char), strlen(input) + 1);
	strcpy(s, input);
	stripChars(s, " \n", "\\");

	char **command = splitOnChars(s, " ", "\"", "\\");
	return command;
}

// Executes input with filename as stdout in overwrite mode.
void redirStdoutWrite(char *input, char *filename) {
	umask(0000);
	int fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT, 0644);

	if (fd == -1) {
		switch (errno) {
		case ENOENT:
			printf("%s: No such file or directory\n", filename);
			break;

		default:
			printf("Error %d: %s\n", errno, strerror(errno));
			break;
		}
	} else {
		int stdout = dup(STDOUT_FILENO);
		dup2(fd, STDOUT_FILENO);
		close(fd);
		execute(input);
		dup2(stdout, STDOUT_FILENO);
		close(stdout);
	}
}

// Executes input with filename as stdout in append mode.
void redirStdoutAppend(char *input, char *filename) {
	umask(0000);
	int fd = open(filename, O_WRONLY | O_APPEND | O_CREAT, 0644);

		if (fd == -1) {
		switch (errno) {
		case ENOENT:
			printf("%s: No such file or directory\n", filename);
			break;

		default:
			printf("Error %d: %s\n", errno, strerror(errno));
			break;
		}
	} else {
		int stdout = dup(STDOUT_FILENO);
		dup2(fd, STDOUT_FILENO);
		close(fd);
		execute(input);
		dup2(stdout, STDOUT_FILENO);
		close(stdout);
	}
}

// Executes input with filename as stderr in overwrite mode.
void redirStderrWrite(char *input, char *filename) {
	umask(0000);
	int fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT, 0644);

		if (fd == -1) {
		switch (errno) {
		case ENOENT:
			printf("%s: No such file or directory\n", filename);
			break;

		default:
			printf("Error %d: %s\n", errno, strerror(errno));
			break;
		}
	} else {
		int stderr_copy = dup(STDERR_FILENO);
		dup2(fd, STDERR_FILENO);
		close(fd);
		execute(input);
		dup2(stderr_copy, STDERR_FILENO);
		close(stderr_copy);
	}
}

// Executes input with filename as stderr in append mode.
void redirStderrAppend(char *input, char *filename) {
	umask(0000);
	int fd = open(filename, O_WRONLY | O_APPEND | O_CREAT, 0644);

		if (fd == -1) {
		switch (errno) {
		case ENOENT:
			printf("%s: No such file or directory\n", filename);
			break;

		default:
			printf("Error %d: %s\n", errno, strerror(errno));
			break;
		}
	} else {
		int stderr_copy = dup(STDERR_FILENO);
		dup2(fd, STDERR_FILENO);
		close(fd);
		execute(input);
		dup2(stderr_copy, STDERR_FILENO);
		close(stderr_copy);
	}
}

// Executes input with filename as stdout and stderr in write mode.
void redirStdoutStderrWrite(char *input, char *filename) {
	umask(0000);
	int fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT, 0644);

	if (fd == -1) {
		switch (errno) {
		case ENOENT:
			printf("%s: No such file or directory\n", filename);
			break;

		default:
			printf("Error %d: %s\n", errno, strerror(errno));
			break;
		}
	} else {
		int stdout_copy = dup(STDOUT_FILENO);
		int stderr_copy = dup(STDERR_FILENO);
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
		close(fd);
		execute(input);
		dup2(stdout_copy, STDOUT_FILENO);
		dup2(stderr_copy, STDERR_FILENO);
		close(stdout_copy);
		close(stderr_copy);
	}
}

// Executes input with filename as stdout and stderr in append mode.
void redirStdoutStderrAppend(char *input, char *filename) {
	umask(0000);
	int fd = open(filename, O_WRONLY | O_APPEND | O_CREAT, 0644);


		if (fd == -1) {
		switch (errno) {
		case ENOENT:
			printf("%s: No such file or directory\n", filename);
			break;

		default:
			printf("Error %d: %s\n", errno, strerror(errno));
			break;
		}
	} else {
		int stdout_copy = dup(STDOUT_FILENO);
		int stderr_copy = dup(STDERR_FILENO);
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
		close(fd);
		execute(input);
		dup2(stdout_copy, STDOUT_FILENO);
		dup2(stderr_copy, STDERR_FILENO);
		close(stdout_copy);
		close(stderr_copy);
	}
}

// Executes input with filename as stdin.
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

// Executes both input1 and input2 with stdin of input2 as the stdout of input1.
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

// Executes a one-line command.
void execute(char *input) {
	char *s = input;

	while (*s) {
		if (*s == '\\') {
			s++;
		} else if (startsWith(s, "2>>")) {
			*(s++) = 0;
			shift(s, 2);
			stripChars(s, " \n", "\\");
			redirStderrAppend(input, s);
			return;
		} else if (startsWith(s, "2>")) {
			*(s++) = 0;
			shift(s, 1);
			stripChars(s, " \n", "\\");
			redirStderrWrite(input, s);
			return;
		} else if (startsWith(s, "&>>")) {
			*(s++) = 0;
			shift(s, 2);
			stripChars(s, " \n", "\\");
			redirStdoutStderrAppend(input, s);
			return;
		} else if (startsWith(s, "&>")) {
			*(s++) = 0;
			shift(s, 1);
			stripChars(s, " \n", "\\");
			redirStdoutStderrWrite(input, s);
			return;			
		} else if (startsWith(s, ">>")) {
			*(s++) = 0;
			shift(s, 1);
			stripChars(s, " \n", "\\");
			redirStdoutAppend(input, s);
			return;
		} else if (*s == '>') {
			*(s++) = 0;
			stripChars(s, " \n", "\\");
			redirStdoutWrite(input, s);
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
		}

		s++;
	}

	int background = 0;
	
	if (*(s - 1) == '&' && *(s - 2) != '\\') {
		background = 1;
		*(s - 1) = 0;
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
			if (!background) {
				waitpid(pid, 0, 0);
			}
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

// Prompts the user to enter a command.
void prompt() {
	char *cwd = getcwd(NULL, 0);
	char *home = getenv("HOME");
	
	if (startsWith(cwd, home)) {
		shift(cwd, strlen(home) - 1);
		*cwd = '~';
	}
	
	char hostname[256];
	gethostname(hostname, 256);
	
	char *prefix = (char *) calloc(sizeof(char), strlen(cwd) + strlen(hostname) + 64);
	sprintf(prefix, BOLD_MAGENTA "kash " BOLD_RED "%s:" BOLD_BLUE "%s @ " RESET, hostname, cwd);

	char *input = readline(prefix);
	stripChars(input, " \n", "\\");

	if (input && *input) {
		execute(input);
		add_history(input);
		usleep(1000);
	}
}

// Initializes and runs the program.
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
