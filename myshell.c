//Erik Roeckel (EFR11)

#define _GNU_SOURCE
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/times.h>

char input[300];
const char* delim = " \t\n";
int num_tokens = 0;
char* tokens[150];


int main()
{
	while(1)
	{
		printf("myshell> ");
		fgets(input, sizeof(input), stdin); // gets input string from user
		char* in;
		char* out;
		char* token = strtok(input, delim); //Gives us tokens from user input starting at first token, tokens found with delim
		int in_counter = 0;
		int out_counter = 0;
		int argument_counter = 0;
		FILE *file_in;
		FILE *file_out;

		// loops through each token until it finds a NULL token which means this is the end of the user input
		for(argument_counter= 0; token != NULL; argument_counter++)
		{
			num_tokens = argument_counter;
			tokens[argument_counter] = token;
			// checks for output redirection token
			if(strcmp(">", token) == 0)
			{
				tokens[argument_counter] = NULL;
				if(out_counter == 0)
				{
					out = strtok(NULL, delim);
					token = out;
				}
				else
					token = strtok(NULL, delim);
				out_counter++;
			}
			// checks for input redirection token
			else if(strcmp("<", token) == 0)
			{
				tokens[argument_counter] = NULL;
				if(in_counter == 0)
				{
					in = strtok(NULL, delim);
					token = in;
				}
				else
					token = strtok(NULL, delim);
				in_counter++;
			}
			else
				token = strtok(NULL, delim);

		}
		
		//Ensures that the there is no more than 1 input/output redirections at a time
		if(in_counter > 1 || out_counter > 1)
			{
				printf("Too many input/output arguments\n");
				continue;
			}
		// null terminatior
		tokens[argument_counter] = NULL;
		
		// handles the user pressing just the enter key(\n) sends it to new iteration of loop
		if(tokens[0] == NULL)
			continue;

		//enters if the token string is equal to exit
		if(strcmp("exit", tokens[0]) == 0)
		{
			// enters if the token after 'exit' is not NULL
			if(tokens[1] != NULL)
			{
				//this parses that token into an integer to be used as 'exit' number
				int exit_num = atoi(tokens[1]);
				exit(exit_num);
			}
				exit(0);
		}
		//enters if the token string is equal to cd
		else if(strcmp("cd", tokens[0]) == 0)
		{
			//enters if there is no argument after cd or if the argument after cd is not a current file or directory
			if(tokens[1] == NULL || chdir(tokens[1]) == -1)
				printf("\nNo such file or directory\n");
			//enters if there is a valid argument(existing file) after cd
			else
			{
				chdir(tokens[1]);
			}
			continue;
		}

		// executes if fork is succesfull, 0 is returned to child process
		if(fork() == 0)
		{
			signal(SIGINT, SIG_DFL);
			
			//executes if 1 output redirection tokenhas been detected
			if(out_counter == 1)
			{
				//opens a file for redirecting stdout, opens as write-only
				file_out = freopen(out, "w", stdout);
				//prints appropriate error stored in errno if freopen fails and returns NULL
				if(file_out == NULL)
					perror("Failed redirect output: \n");
			}
			//executes if 1 input redirection token has been detected
			if(in_counter == 1)
			{
				//opens a file for redirecting stdin, opens as read-only
				file_in = freopen(in, "r", stdin);
				//prints appropriate error stored in errno if freopen fails and returns NULL
				if(file_in == NULL)
					perror("Failed redirect input: \n");

			}
			
			//provides an array of pointers to null-terminated strings that represent the argument list available to program
			execvp(tokens[0], &tokens[0]);
			int fd = open(tokens[0], O_RDONLY);
			//executes if execvp() returns an error, can't open file
			if(fd < 0)
			{
				perror("Couldn't open file: ");
				exit(1);
			}

		}

		// executes if fork() is not successful, no child process is created, and errno is set
		else
		{
			signal(SIGINT, SIG_IGN);
			int status;
			int childpid = waitpid(-1, &status, 0);
			
			// enters if waitpid returns an error (-1)
			if(childpid == -1)
				perror("Error running program: \n"); //prints that error to the user
			
			// enters if child terminated due to a signal this prints out the signal which killed it
			else if(WIFSIGNALED(status))
				printf(" Program terminated due to signal: %s\n", strsignal(status));
			// otherwise lets the user know the it terminated abnormally
			else if(status != 0 && !WIFEXITED(status))
				printf("Program terminated abnormally!\n");

		}
	}
	
	return 0;

}