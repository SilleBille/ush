/*
 * builtin.c
 *
 *  Created on: Oct 20, 2016
 *      Author: dinesh
 */

#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<sys/stat.h>
#include<errno.h>
#include<sys/resource.h>
#include<sys/time.h>
#include"builtin.h"

/*int isDirectory(char * path) {
 struct stat sb;

 return (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode));
 }*/
int isBuiltIn(char *c) {
	int i = 0;
	for (i = 0; i < 12; i++) {
		if (strcmp(builtin_command[i], c) == 0)
			return i;
	}
	return -1;
}
void cd_cmd(Cmd c) {

	char pathToChange[300];
	if (c->args[1] == NULL || c->args[1][0] == '~') {
		strcpy(pathToChange, getenv("HOME"));
	} else {
		strcpy(pathToChange, c->args[1]);
	}
	if (chdir(pathToChange) == -1) {
		switch (errno) {
		case EACCES:
			fprintf(stderr, "Permission Denied!\n");
			break;
		case ENOTDIR:
			fprintf(stderr, "Not a directory!\n");
			break;
		case ENOENT:
			fprintf(stderr, "Directory does not exist!\n");
			break;
		}
	}
}

void echo_cmd(Cmd c) {
	int i;
	if (c->args[1] == NULL) {
		fprintf(stdout, "\n");
		return;
	}
	for (i = 1; c->args[i] != NULL; i++) {
		fprintf(stdout, "%s", c->args[i]);

		if (c->args[i + 1] != NULL)
			fprintf(stdout, " ");
	}
	fprintf(stdout, "\n");
}

void logout_cmd() {
	exit(0);
}

void pwd_cmd() {
	char *cwd = (char *) malloc(2000 * sizeof(char));
	cwd = get_current_dir_name();
	fprintf(stdout, "%s\n", cwd);
}

void where_cmd(Cmd c) {
	char *origPath = getenv("PATH");
	char path[1000];
	strcpy(path, origPath);
	char delimiter[1];
	delimiter[0] = ':'; // The path is separated using :
	if (isBuiltIn(c->args[1]) != -1) {
		printf("%s: shell built-in command.\n", c->args[1]);
	}
	char *p = strtok(path, delimiter);
	char *pathToCommand = (char *) malloc(sizeof(char) * 150);

	while (p != NULL) {
		strcpy(pathToCommand, p); // Need to generate <path>/<command>
		strcat(pathToCommand, "/");
		strcat(pathToCommand, c->args[1]);
		if (checkIsCommandPath(pathToCommand))
			printf("%s\n", pathToCommand);

		p = strtok(NULL, delimiter);
	}

}

void setenv_cmd(Cmd c) {
	extern char **environ;
	char **availableEnviron = environ;

	if (c->args[1] == NULL) {
		// If there are no arguments, Print all available paths
		for (; *availableEnviron != NULL; *availableEnviron++)
			fprintf(stdout, "%s\n", *availableEnviron);
	} else {
		// Some arguments are there.
		// args[1] = NAME,
		// args[2] = VALUE
		if (c->args[2] == NULL) {
			// If no word
			setenv(c->args[1], "", 1);
		} else {
			setenv(c->args[1], c->args[2], 1);
		}
	}
}

void unsetenv_cmd(Cmd c) {
	if (c->args[1] != NULL) {
		unsetenv(c->args[1]);
	} else {
		fprintf(stderr, "unsetenv: Too few arguments.");
	}
}

void nice_cmd(Cmd c) {
	int which, who, prio = 4;
	pid_t pid;
	which = PRIO_PROCESS;
	if (c->args[1] == NULL) {
		who = getpid();
		setpriority(which, who, prio);

	} else {
		prio = atoi(c->args[1]);
		// printf("The priority: %d\n", prio);
		if (c->args[2] == NULL) {
			who = getpid();
			setpriority(which, who, prio);

		} else {
			Cmd tempCmd = (Cmd) malloc(sizeof(Cmd) * sizeof(c));
			tempCmd->args = malloc(sizeof(c->args) * sizeof(char *));/*
			 tempCmd->args[0] = malloc(strlen(c->args[2]));
			 strcpy(tempCmd->args[0],c->args[2]);
			 printf("This: %s\n", tempCmd->args[0]);*/
			/*memcpy(tempCmd->args, c->args + 1, sizeof(c->args) * sizeof(c->args));*/
			int i;
			i = 0;
			while (c->args[i + 2]) {
				tempCmd->args[i] = (char *) malloc(
						sizeof(c->args[i + 2]) * sizeof(char));
				strcpy(tempCmd->args[i], c->args[i + 2]);
				i++;
			}
			tempCmd->args[i] = NULL;
			if (isBuiltIn(tempCmd->args[0]) != -1) {
				who = getpid();
				setpriority(which, who, prio);
				if (strcmp(tempCmd->args[0], "cd") == 0) {
					cd_cmd(tempCmd);
				} else if (strcmp(tempCmd->args[0], "echo") == 0) {
					echo_cmd(tempCmd);
				} else if (strcmp(tempCmd->args[0], "logout") == 0) {
					logout_cmd();
				} else if (strcmp(tempCmd->args[0], "pwd") == 0) {
					pwd_cmd();
				} else if (strcmp(tempCmd->args[0], "where") == 0) {
					where_cmd(tempCmd);
				} else if (strcmp(tempCmd->args[0], "setenv") == 0) {
					setenv_cmd(tempCmd);
				} else if (strcmp(tempCmd->args[0], "unsetenv") == 0) {
					unsetenv_cmd(tempCmd);
				}
			} else {

				who = fork();
				setpriority(which, who, prio);
				if (who == 0) {
					execvp(tempCmd->args[0], tempCmd->args);
				} else if (who > 0) {
					wait(NULL);
				}
			}
		}
	}

}
int checkIsCommandPath(char *path) {
	struct stat sb;

	if (stat(path, &sb) == 0 && S_ISREG(sb.st_mode)) {
		return 1;
	}
	return 0;
}

/*int hasExecPermission(char *pathToCmd) {
 if(checkIsCommandPath(pathToCmd) && access(pathToCmd, R_OK|X_OK) == 0)
 return 1;
 return 0;
 }*/
