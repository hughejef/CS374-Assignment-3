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
#include <unistd.h> // chdir, getenv
#include <sys/wait.h> // waitpid, status macros
#include <sys/types.h> // pid_t
#include <fcntl.h> // open, O_RDONLY, O_WRONLY, O_CREAT, O_TRUNC

#define INPUT_LENGTH 2048
#define MAX_ARGS	 512
#define MAX_BG_PROCS 100 // Max number of background processes


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
// Array of background PIDs
pid_t bg_pids[MAX_BG_PROCS];
int bg_count = 0;

struct command_line *parse_input()
{
	char input[INPUT_LENGTH];
	struct command_line *curr_command = (struct command_line *) calloc(1, sizeof(struct command_line));

	// Get input
	printf(": ");
	fflush(stdout);
	if (!fgets(input, INPUT_LENGTH, stdin)) {
		free(curr_command);
		exit(0);
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
		// reprompt if blank
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
            // Whenever a non-built-in command is received, the parent will fork
            pid_t childPid = fork();
            if (childPid == -1) {
                // If fork fails, indicate error and set status to 1
                perror("fork");
                status = 1;
            } else if (childPid == 0) {
                // Support input redirection - Section 5
                // An input file redirected via stdin must be opened for reading only
                if (curr_command->input_file != NULL) {
                    int in_fd = open(curr_command->input_file, O_RDONLY);
                    // If your shell cannot open the file for reading, it must print an error message and set the exit status to 1
                    if (in_fd == -1) {
                        printf("cannot open %s for input\n", curr_command->input_file);
                        fflush(stdout);
                        exit(1);
                    }
                    // The redirection must be done before using exec() to run the command
                    dup2(in_fd, 0); 
                    close(in_fd); 
                }
                // Handle output redirection (>)
                // An output file redirected via stdout must be opened for writing only; it must be truncated if it already exists or created if it does not exist
                if (curr_command->output_file != NULL) {
                    int out_fd = open(curr_command->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    // If your shell cannot open the output file it must print an error message and set the exit status to 1
                    if (out_fd == -1) {
                        printf("cannot open %s for output\n", curr_command->output_file);
                        fflush(stdout);
                        exit(1);
                    }
                    dup2(out_fd, 1);
                    close(out_fd); 
                }
                // If the user doesn't redirect stdin for a background command, redirect to /dev/null
                if (curr_command->is_bg && curr_command->input_file == NULL) {
                    int in_fd = open("/dev/null", O_RDONLY);
                    if (in_fd == -1) {
                        printf("cannot open /dev/null for input\n");
                        fflush(stdout);
                        exit(1);
                    }
                    dup2(in_fd, 0);
                    close(in_fd);
                }
                // If the user doesn't redirect stdout for a background command, redirect to /dev/null
                if (curr_command->is_bg && curr_command->output_file == NULL) {
                    int out_fd = open("/dev/null", O_WRONLY);
                    if (out_fd == -1) {
                        printf("cannot open /dev/null for output\n");
                        fflush(stdout);
                        exit(1);
                    }
                    dup2(out_fd, 1);
                    close(out_fd);
                }
                // The child will use a function from the exec() family to run the command
                // Use execvp to search PATH for non-built-in commands and allow shell scripts
                if (execvp(curr_command->argv[0], curr_command->argv) == -1) {
                    // If a command fails because it could not be found, print error and set status to 1
                    perror("execvp");
                    // A child process must terminate after running a command (successful or failed)
                    exit(1);
                }
            } 
			else {
                // Any non-built-in command with & at the end run as background, shell doesn't wait
                if (curr_command->is_bg) {
                    // Print the process id of a background process when it begins
                    printf("background pid is %d\n", childPid);
                    fflush(stdout);
                    // Store PID in array
                    if (bg_count < MAX_BG_PROCS) {
                        bg_pids[bg_count++] = childPid;
                    }
                } else {
                // Parent waits for child to complete
                int childStatus;
                waitpid(childPid, &childStatus, 0);
                // Set status to exit value if child exited normally and... 
                if (WIFEXITED(childStatus)) {
                    status = WEXITSTATUS(childStatus);
				// ... negative signal if terminated!
                } else if (WIFSIGNALED(childStatus)) {
                    status = -WTERMSIG(childStatus);
                }
            }
		}
        }
		/* printf("Command: %s, Args: %d, Input: %s, Output: %s, Bg: %d\n",
               curr_command->argv[0] ? curr_command->argv[0] : "none",
               curr_command->argc,
               curr_command->input_file ? curr_command->input_file : "none",
               curr_command->output_file ? curr_command->output_file : "none",
               curr_command->is_bg);
		*/

	for (int i = 0; i < curr_command->argc; i++) {
            free(curr_command->argv[i]);
        }
        free(curr_command->input_file);
        free(curr_command->output_file);
        free(curr_command);
    }
	return EXIT_SUCCESS;
}