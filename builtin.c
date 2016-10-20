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
#include"builtin.h"

int isBuiltIn(Cmd c) {
	int i = 0;
	for (i = 0; i < 12; i++) {
		if (strcmp(builtin_command[i], c->args[0]) == 0)
			return 1;
	}
	return 0;
}
void cd_cmd(Cmd c) {
	if (c->args[1] == NULL) {
		// It requires to char*. WHY?!?!
		char pathToHome[300];
		strcpy(pathToHome, getenv("HOME"));
		printf("Path to home: %s\n", pathToHome);
	}
}
