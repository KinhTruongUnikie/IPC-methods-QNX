/*
 * ipc_sendfile.c
 *
 *  Created on: Feb 17, 2022
 *      Author: kt
*/
#include "ipc_methods.h"
#include "msg.h"

int main(int argc, char* argv[]) {
	ipc_info info;
	printInstruction();
	info = checkOptions(argc, argv);
	run_IPC(&info, SEND);
	return 0;
}