/*
 * ipc_sendfile.c
 *
 *  Created on: Feb 17, 2022
 *      Author: kt
 */

//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>

#include "ipc_methods.h"
#include "msg.h"




int main(int argc, char* argv[]) {
	//getopt()
	ipc_info info;
	printInstruction();

	info = checkOptions(argc, argv);
	run_IPC(&info, SEND);
	//printf("test exit failure");
	/*
	int coid, status;
	char text[10] = "succeed";
	coid = name_open(SERVER_NAME, 0);
	status = MsgSend(coid, text, sizeof(text), NULL, 0);
	if (status != -1) {
		printf("Hello\n");
	}*/
	return 0;
}



