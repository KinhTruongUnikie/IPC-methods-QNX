/*
 * ipc_receivefile.c
 *
 *  Created on: Feb 17, 2022
 *      Author: kt

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <process.h>
#include <sys/dispatch.h>
#include <sys/siginfo.h>
#include <unistd.h>
*/
#include "ipc_methods.h"
#include "msg.h"


int main(int argc, char* argv[]) {
	ipc_info info;
	printInstruction();

	info = checkOptions(argc, argv);
	run_IPC(&info, RECEIVE);

	return 0;
}
