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
#include "parse.h"

/* Professor Code!!! Don't touch! */

static void prCmd(Cmd c) {
	int i;
	pid_t pid;
	int fd;
	pid = fork();
	if (pid < 0)
		perror("Fork error!\n");
	else if (pid > 0) {
		// parent process

		int status;
		waitpid(pid, &status, 0);
	} else if (pid == 0) {

		// child process

		if (c) {
			// printf("%s%s ", c->exec == Tamp ? "BG " : "", c->args[0]);
			if (c->in == Tin)
				printf("<(%s) ", c->infile);
			if (c->out != Tnil)
				switch (c->out) {
				case Tout:
					// printf(">(%s) ", c->outfile);

					// Create file if not exist. OVERWRITE ie. TRUNCATE if file exist
					fd = open(c->outfile, O_CREAT | O_WRONLY | O_TRUNC, 0777);
					dup2(fd, STDOUT_FILENO);
					close(fd);
					break;
				case ToutErr:
					// printf(">&(%s) ", c->outfile);
					fd = open(c->outfile, O_CREAT | O_WRONLY | O_TRUNC, 0777);
					dup2(fd, STDOUT_FILENO);
					dup2(fd, STDERR_FILENO);
					close(fd);
					break;
				case Tapp:
					// printf(">>(%s) ", c->outfile);

					// Create file if not exist. APPEND if exist
					fd = open(c->outfile, O_CREAT | O_WRONLY | O_APPEND, 0777);
					dup2(fd, STDOUT_FILENO);
					close(fd);
					break;

				case TappErr:
					// printf(">>&(%s) ", c->outfile);
					fd = open(c->outfile, O_CREAT | O_WRONLY | O_APPEND, 0777);
					dup2(fd, STDOUT_FILENO);
					dup2(fd, STDERR_FILENO);
					close(fd);
					break;
				case Tpipe:
					printf("| ");
					break;
				case TpipeErr:
					printf("|& ");
					break;
				default:
					fprintf(stderr, "Shouldn't get here\n");
					exit(-1);
				}

			if (c->nargs > 1) {
				printf("[");
				for (i = 1; c->args[i] != NULL; i++)
					printf("%d:%s,", i, c->args[i]);
				printf("\b]");
			}
			// putchar('\n');
		}
		// TODO: Define built in command and execute them if found!
		// Only if it's not built in command
		execvp(c->args[0], c->args);
	}
	// this driver understands one command
	if (!strcmp(c->args[0], "end"))
		exit(0);

}
static void prPipe(Pipe p) {
	int i = 0;
	Cmd c;

	if (p == NULL)
		return;

	// printf("Begin pipe%s\n", p->type == Pout ? "" : " Error");
	for (c = p->head; c != NULL; c = c->next) {
		// printf("  Cmd #%d: ", ++i);
		// I'm going to process commands
		prCmd(c);
	}
	// printf("End pipe\n");
	prPipe(p->next);
}

int main(int argc, char *argv[]) {
	Pipe p;
	char host[128];
	gethostname(host, sizeof(host));
	/* Ignore interactive and job-control signals.  */
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN); // handle ctrl+d
	signal(SIGTSTP, SIG_IGN); // handle ctrl+z

	while (1) {
		printf("%s%% ", host);
		p = parse();
		prPipe(p);
		freePipe(p);
	}
}

/*........................ end of main.c ....................................*/
