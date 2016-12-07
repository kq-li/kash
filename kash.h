static void sighandler(int signo);

void copyBetween(char *dest, char *start, char *end);

void shift(char *str, int offset);

void removeFirst(char *str, char *toRemove);

void removeAll(char *str, char *toRemove);

void stripChars(char *str, char *toStrip, char *escape);

int startsWith(char *str, char *key);

char **splitOnChars(char *str, char *delim, char *escRegion, char *escOne);

char **parseInput(char *input);

void redirStdoutWrite(char *input, char *filename);

void redirStdoutAppend(char *input, char *filename);

void redirStderrWrite(char *input, char *filename);

void redirStderrAppend(char *input, char *filename);

void redirStdoutStderrWrite(char *input, char *filename);

void redirStdoutStderrAppend(char *input, char *filename);

void redirStdin(char *input, char *filename);

void redirPipe(char *input1, char *input2);

void execute(char *input);

void prompt();
