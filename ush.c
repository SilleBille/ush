/******************************************************************************
 *
 *  File Name........: main.c
 *
 *  Description......: Simple driver program for ush's parser
 *
 *  Author...........: Vincent W. Freeh
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "parse.h"
#include "builtin.h"

#define TRUE 1
#define FALSE 0

int ofd_stdin;
int ofd_stdout;
int ofd_stderr;

int shouldBuiltInFork;
int shouldRestore;

int pipefd[100][2];
int pipeRef = -1;
int writeToPipe[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int readFromPipe[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };


int isRcParsing = 0;

int shouldPrintPrompt = FALSE;
int isPromptRequired = FALSE;
int isStdinPresent = FALSE;

// You need to save the FD to recover if built in command is executed!
void saveFileDesc() {
	ofd_stdin = dup(STDIN_FILENO);
	ofd_stdout = dup(STDOUT_FILENO);
	ofd_stderr = dup(STDERR_FILENO);
}

// Now restore the FDs
void restoreFileDesc() {
	dup2(ofd_stdin, STDIN_FILENO);
	dup2(ofd_stdout, STDOUT_FILENO);
	dup2(ofd_stderr, STDERR_FILENO);
}

void disableSignals() {
	/* Ignore interactive and job-control signals.  */
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN); // handle ctrl+d
	signal(SIGTSTP, SIG_IGN); // handle ctrl+z
}

void enableSignals() {
	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
	signal(SIGTERM, SIG_DFL); // handle ctrl+d
	signal(SIGTSTP, SIG_DFL); // handle ctrl+z
}

void prSymbols(Cmd c) {
	int i;
	int fd;

	if (c) {
		// printf("%s%s ", c->exec == Tamp ? "BG " : "", c->args[0]);
		if (c->in == Tin) {
			// printf("<(%s) ", c->infile);

			// Read from stdin
			fd = open(c->infile, O_RDONLY);
			dup2(fd, STDIN_FILENO);
			close(fd);
		}
		if (c->in == Tpipe) {
			readFromPipe[pipeRef] = 1;
		}
		if (c->out != Tnil) {
			switch (c->out) {
			case Tout:
				// printf(">(%s) ", c->outfile);

				// Create file if not exist. OVERWRITE ie. TRUNCATE if file exist
				fd = open(c->outfile, O_CREAT | O_WRONLY | O_TRUNC, 0777);
				dup2(fd, STDOUT_FILENO);
				close(fd);
				shouldRestore = 1;
				break;
			case ToutErr:
				// printf(">&(%s) ", c->outfile);
				fd = open(c->outfile, O_CREAT | O_WRONLY | O_TRUNC, 0777);
				dup2(fd, STDOUT_FILENO);
				dup2(fd, STDERR_FILENO);
				close(fd);
				shouldRestore = 1;
				break;
			case Tapp:
				// printf(">>(%s) ", c->outfile);

				// Create file if not exist. APPEND if exist
				fd = open(c->outfile, O_CREAT | O_WRONLY | O_APPEND, 0777);
				dup2(fd, STDOUT_FILENO);
				close(fd);
				shouldRestore = 1;
				break;

			case TappErr:
				// printf(">>&(%s) ", c->outfile);
				fd = open(c->outfile, O_CREAT | O_WRONLY | O_APPEND, 0777);
				dup2(fd, STDOUT_FILENO);
				dup2(fd, STDERR_FILENO);
				close(fd);
				shouldRestore = 1;
				break;
			case Tpipe:
				// printf("\n| \n");
				pipeRef++;
				pipe(pipefd[pipeRef]);

				writeToPipe[pipeRef] = 1;

				shouldRestore = 0;
				// isPipeActive[pipeRef] = 1;
				break;
			case TpipeErr:
				printf("|& ");
				shouldRestore = 0;
				break;
			default:
				fprintf(stderr, "Shouldn't get here\n");
				exit(-1);
			}
		}

		/*if (c->nargs > 1) {
		 printf("[");
		 for (i = 1; c->args[i] != NULL; i++)
		 printf("%d:%s,", i, c->args[i]);
		 printf("\b]");
		 }*/
		// putchar('\n');
		// this driver understands one command
		if (!strcmp(c->args[0], "end")) {
			if (isRcParsing == FALSE) {
				// Comes in when < is given from BASH
				fprintf(stderr, "NO RC PROCESSING REQUIRED! Just QUITE!\n");
				exit(0);
			} else {
				// Comes in when .ushrc file is executed
				fprintf(stderr, "RC PROCDSSING DONE!!!\n");
				isRcParsing = FALSE;
			}
		}
	}
}

static void execute_builtin(Cmd c) {
	if (strcmp(c->args[0], "cd") == 0) {
		cd_cmd(c);
	} else if (strcmp(c->args[0], "echo") == 0) {
		echo_cmd(c);
	} else if (strcmp(c->args[0], "logout") == 0) {
		logout_cmd();
	} else if (strcmp(c->args[0], "pwd") == 0) {
		pwd_cmd();
	} else if (strcmp(c->args[0], "where") == 0) {
		where_cmd(c);
	}
}

static void prCmd(Cmd c) {
	int i;
	pid_t pid;
	printf("trying to execute: %s\n", c->args[0]);
	// this driver understands one command
	if (isBuiltIn(c->args[0]) != -1 && shouldBuiltInFork == 0) {
		enableSignals();
		execute_builtin(c);
		disableSignals();
	} else {

		// Fork only if it's not a built in
		pid = fork();
		if (pid < 0)
			perror("Fork error!\n");
		else if (pid > 0) {
			// parent process

			int status;

			waitpid(pid, &status, 0); // Wait for the child to complete its execution

			if (writeToPipe[pipeRef] == TRUE) {
				close(pipefd[pipeRef][1]);
				writeToPipe[pipeRef] = FALSE;
			}

		} else if (pid == 0) {
			int whichPipeToReadFrom = pipeRef;
			if (writeToPipe[pipeRef] == TRUE) {
				dup2(pipefd[pipeRef][1], 1);
				close(pipefd[pipeRef][0]);
				whichPipeToReadFrom = pipeRef - 1;
			}
			if (readFromPipe[whichPipeToReadFrom] == TRUE) {
				dup2(pipefd[whichPipeToReadFrom][0], 0);
				close(pipefd[whichPipeToReadFrom][1]);
			}
			// child process

			if (isBuiltIn(c->args[0]) != -1 && shouldBuiltInFork == 1) {
				enableSignals();
				execute_builtin(c);
				disableSignals();
			} else {
				// Only if it's not built in command
				// fprintf(stderr, "%s I'm not built-in\n", c->args[0]);
				enableSignals();

				execvp(c->args[0], c->args);

				// Shouldn't return. If returns, them some error occured
				switch (errno) {
				case EISDIR:
				case EACCES:
					fprintf(stderr, "permission denied\n");
					break;
				case ENOENT:
					fprintf(stderr, "command not found\n");
					break;
				default:
					break;
				}
			}

			exit(0);
		}

	}

}
static void prPipe(Pipe p) {
	int i = 0;
	Cmd c;

	if (p == NULL) {
		// restoreFileDesc();
		// doneProcessingInit = 1;
		return;
	}
	//printf("should the builtin be forked? %d\n", shouldBuiltInFork);

	// printf("Begin pipe%s\n", p->type == Pout ? "" : " Error");
	for (c = p->head; c != NULL; c = c->next) {
		// printf("  Cmd #%d: ", ++i);
		// Check if there is a pipe. If so, the builtIn should fork
		shouldBuiltInFork = (c->next != NULL) ? 1 : 0;

		prSymbols(c);
		prCmd(c);
		// If current cmd is the last, you MUST restore the pipelines
		if (c->next == NULL) {
			shouldRestore = 1;
		}

		if (shouldRestore == 1 && isRcParsing == FALSE) {
			restoreFileDesc();
		}
		/*if ( c->nargs > 1 ) {
		 printf("[");
		 for ( i = 1; c->args[i] != NULL; i++ )
		 printf("%d:%s,", i, c->args[i]);
		 printf("\b]");
		 }*/
	}
	// printf("End pipe\n");
	prPipe(p->next);
}

void initshell() {
	Pipe p1;
	char *pathToUshrc = (char *) malloc((sizeof(char) * 2000));
	strcpy(pathToUshrc, getenv("HOME"));
	strcat(pathToUshrc, "/.ushrc");
	int ret = access(pathToUshrc, F_OK | R_OK);
	int fd;
	if (ret == 0) {
		fd = open(pathToUshrc, O_RDONLY);
		dup2(fd, STDIN_FILENO);
		close(fd);

		isRcParsing = TRUE;

		isPromptRequired = FALSE;
		//fflush(stdout);
	}
}
void setupPrompt() {
	char host[128];
	gethostname(host, sizeof(host));
	if (!isStdinPresent) {
		printf("%s%% ", host);
		fflush(stdout);
	}
}

int main(int argc, char *argv[]) {
	Pipe p;
	int num;

	// I'm going to save the file desc, process symbols and then commands
	saveFileDesc();

	// int tempfd;
	// dup2(stdin, tempfd);
	// fseek (tempfd, 0, SEEK_END);

	num = ftell (stdin);

	printf("Is the stdin presend? %d\n", num);

	if(num != -1)
		isStdinPresent = TRUE;
	initshell();

	while (1) {

		setupPrompt();
		disableSignals();
		pipeRef = -1;

		p = parse();
		prPipe(p);
		freePipe(p);
	}
}

/*........................ end of main.c ....................................*/
