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

	// Check if its a comment
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