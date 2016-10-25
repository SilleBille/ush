/*
 * builin.h
 *
 *  Created on: Oct 20, 2016
 *      Author: dinesh
 */

#ifndef BUILTIN_H_
#define BUILTIN_H_

#include"parse.h"

int isBuiltIn(char *c);
/*int hasExecPermission(char *pathToCmd);*/

void cd_cmd(Cmd c);
void echo_cmd(Cmd c);
void kill_cmd(Cmd c);
void logout_cmd();
void pwd_cmd();
void where(Cmd c);

static char *builtin_command[] = {
		"bg", "cd", "fg", "echo", "jobs", "kill", "logout",
		"nice", "pwd", "setenv", "unsetenv", "where"
};

#endif /* BUILTIN_H_ */
