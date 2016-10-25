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

		if(c->args[i+1] != NULL) fprintf(stdout, " ");
	}
	fprintf(stdout, "\n");
}

void logout_cmd() {
	exit(0);
}

void pwd_cmd() {
	char *cwd = (char *) malloc(2000 * sizeof(char));
	cwd = get_current_dir_name();
	printf("%s\n", cwd);
}

void where_cmd(Cmd c) {
	char *origPath = getenv("PATH");
	char path[1000];
	strcpy(path, origPath);
	char delimiter[1];
	delimiter[0] = ':'; // The path is separated using :
	if (isBuiltIn(c->args[1]) != -1) {
		printf("%s\n", c->args[1]);
	}
	char *p = strtok(path, delimiter);
	char *pathToCommand = (char *)malloc(sizeof(char) * 150);

	while (p != NULL) {
		strcpy(pathToCommand, p); // Need to generate <path>/<command>
		strcat(pathToCommand, "/");
		strcat(pathToCommand, c->args[1]);
		if(checkIsCommandPath(pathToCommand))
			printf("%s\n", pathToCommand);

		p = strtok(NULL, delimiter);
	}


}

int checkIsCommandPath(char *path) {
	struct stat sb;

	if (stat(path, &sb) == 0 && S_ISREG(sb.st_mode))
	{
	    return 1;
	}
	return 0;
}

/*int hasExecPermission(char *pathToCmd) {
	if(checkIsCommandPath(pathToCmd) && access(pathToCmd, R_OK|X_OK) == 0)
		return 1;
	return 0;
}*/
