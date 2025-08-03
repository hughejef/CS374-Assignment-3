/* This script was adapted from the skeleton code provided for assignment 
with modifications to it made to fit assignment requirements*/

/**
 * A sample program for parsing a command line. If you find it useful,
 * feel free to adapt this code for Assignment 4.
 * Do fix memory leaks and any additional issues you find.
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // chdir and getenv

#define INPUT_LENGTH 2048
#define MAX_ARGS		 512


struct command_line
{
	char *argv[MAX_ARGS + 1];
	int argc;
	char *input_file;
	char *output_file;
	bool is_bg;
};

// status variable - positive for exit, negative for signal, initialized to 0
int status = 0;

struct command_line *parse_input()
{
	char input[INPUT_LENGTH];
	struct command_line *curr_command = (struct command_line *) calloc(1, sizeof(struct command_line));

	// Get input
	printf(": ");
	fflush(stdout);
	if (!fgets(input, INPUT_LENGTH, stdin)) {
		free(curr_command);
		return NULL;
	}

	// Check if input is a comment
    if (input[0] == '#') {
        free(curr_command);
        return NULL;
    }

	// Tokenize the input
	char *token = strtok(input, " \n");
	while(token){
		if(!strcmp(token,"<")){
			curr_command->input_file = strdup(strtok(NULL," \n"));
		} else if(!strcmp(token,">")){
			curr_command->output_file = strdup(strtok(NULL," \n"));
		} else if(!strcmp(token,"&")){
			curr_command->is_bg = true;
		} else{
			curr_command->argv[curr_command->argc++] = strdup(token);
		}
		token=strtok(NULL," \n");
	}
	// confirm not blank line
	if (curr_command->argc == 0 && curr_command->input_file == NULL && curr_command->output_file == NULL 
		&& !curr_command->is_bg) {
        free(curr_command);
        return NULL;
    }
	return curr_command;
}

int main()
{
	struct command_line *curr_command;

	while(true)
	{
		curr_command = parse_input();
		// Any line that begins with the # character is a comment line and must be ignored.
		if (curr_command == NULL) {
            continue;
        }

		// Built in Commands
		// exit
		if (curr_command->argc > 0 && strcmp(curr_command->argv[0], "exit") == 0) {
            exit(0);
		}
		// cd
		else if (curr_command->argc > 0 && strcmp(curr_command->argv[0], "cd") == 0) {
            char *path = (curr_command->argc > 1) ? curr_command->argv[1] : getenv("HOME");
            if (chdir(path) != 0) {
                printf("cd: no such directory\n");
                fflush(stdout);
			}
		}
		// status
		else if (curr_command->argc > 0 && strcmp(curr_command->argv[0], "status") == 0) {
            if (status >= 0) {
                printf("exit value %d\n", status);
            } else {
                printf("terminated by signal %d\n", -status);  // Negative for signal
            }
            fflush(stdout);
		}
		else {
            // ***** TODO
            printf("TODO: %s (TODO)\n",
                   curr_command->argv[0] ? curr_command->argv[0] : "none");
            fflush(stdout);
        }
		printf("Command: %s, Args: %d, Input: %s, Output: %s, Bg: %d\n",
               curr_command->argv[0] ? curr_command->argv[0] : "none",
               curr_command->argc,
               curr_command->input_file ? curr_command->input_file : "none",
               curr_command->output_file ? curr_command->output_file : "none",
               curr_command->is_bg);

	for (int i = 0; i < curr_command->argc; i++) {
            free(curr_command->argv[i]);
        }
        free(curr_command->input_file);
        free(curr_command->output_file);
        free(curr_command);
    }
	return EXIT_SUCCESS;
}