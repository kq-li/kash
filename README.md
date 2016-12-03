# kash$ - an almost-fully functional bash clone  
Written by Kenneth Li  
Systems - Fall 2016 - Period 10  
  
## Features
- Executes commands in a bash-style terminal
	- Tokens in a command can be delimited by any number of spaces
	- Escape characters allow spaces within tokens
		- \ escapes the next character
		- "" escapes all text between quotes
			- \" for quoteception
	- Supported commands
		- Most bash commands
		- "kash" (try it!)
- Processes operators from left to right
	- ; between commands executes the first, then the second
		- Multiple ; supported
	- < redirects stdin to read from a file
	- > redirects stdout to overwrite a file
	- >> redirects stdout to append to a file
	- 2> redirects stderr to overwrite a file
	- 2>> redirects stderr to append to a file
	- &> redirects stdout and stderr to overwrite a file
	- &>> redirects stdout and stderr to append to a file
	- | redirects stdout of the first command to stdin of the second command
		- Multiple pipes supported
	- & backgrounds a process
- Aesthetically pleasing prompt geared towards maximum user satisfaction
	- Supports arrow keys, emacs shortcut navigation, history, and tab completion
	- Prompt contains hostname and current directory
	- Colored using ANSI escape codes and crayons
- State-of-the-art memory plumbing - no leaks here

## Attempted/missing features
- Current Git status in prompt
- Colored commands and output
- Second prompt if escaped newline
- ~ expansion outside cd
- World domination

## Bugs
- ^C and ^Z exit kash entirely instead of terminating/backgrounding the current process
- Only one redirection operator is supported at a time except pipes
	- "ls > test 2> test2" results in the output of ls being written to a file named "test 2> test2"
- Not mosquitoes, thankfully

## Additional information
- One C file to rule them all, one C file to find them
- Sleeps for 1 ms between prompts
	- Ensures user input field is clear of (most) background output (e.g. ls &)
- Uses the GNU readline library

## Function headers
kash.c - Everything
* void copyBetween(char *dest, char *start, char *end);
 	* Copies contents of string between start and end to dest.
 	* Modifies dest.

* void shift(char *str, int offset);
	* Shifts all characters in str leftwards by offset.
	* Truncates characters as they pass the first character.
	* Modifies str.

* void stripChars(char *str, char *toStrip, char *escape)
	* Removes all occurrences of the characters in toStrip from the start and end of str except those preceded by a character in escape.
	* Modifies str.

* int startsWith(char *str, char *key);
	* Returns 1 if str starts with key, otherwise 0.

* char **splitOnChars(char *str, char *delim, char *escRegion, char *escOne);
	* Custom strsep function. Generates a series of strings by splitting str on delimiter characters specified in delim.
	* Delimiter characters wrapped in escRegion or preceded by escOne are escaped.
	* Modifies str.
	* ret needs to be freed.
	* Trust me. It works.

* char **parseInput(char *input);
	* Splits input on spaces, escaped by quotes (") and backslashes ("\").
	* Returns the parsed input.
	* Both command and s (command[0]) need to be freed.

* void redirStdoutWrite(char *input, char *filename);
	* Executes input with filename as stdout in overwrite mode.

* void redirStdoutAppend(char *input, char *filename);
	* Executes input with filename as stdout in append mode.

* void redirStderrWrite(char *input, char *filename);
	* Executes input with filename as stderr in overwrite mode.

* void redirStderrAppend(char *input, char *filename);
	* Executes input with filename as stderr in append mode.

* void redirStdoutStderrWrite(char *input, char *filename);
	* Executes input with filename as stdout and stderr in write mode.

* void redirStdoutStderrAppend(char *input, char *filename);
	* Executes input with filename as stdout and stderr in append mode.

* void redirStdin(char *input, char *filename);
	* Executes input with filename as stdin.

* void redirPipe(char *input1, char *input2);
	* Executes both input1 and input2 with stdin of input2 as the stdout of input1.

* void execute(char *input);
	* Parses and executes input as line containing one or more commands.
	* Modifies input.

* void prompt();
	* Prompts the user to enter a command.

* int main();
	* Initializes and runs the program.
