/*
 * ipc_methods.h
 *
 *  Created on: Feb 21, 2022
 *      Author: kt
 */


#ifndef IPC_METHODS_H_
#define IPC_METHODS_H_

#include <getopt.h>


const struct option longopts[] = {
		{"help", 0, NULL, 'h'},
		{"message", 1, NULL, 'm'},
		{"pipe", 1, NULL, 'p'},
		{"queue", 1, NULL, 'q'},
		{"shm", 1, NULL, 's'},
		{"file", 1, NULL, 'f' },
		{0, 0, 0, 0}
};

char* checkOptions (int argc, char* argv[]);
#endif /* IPC_METHODS_H_ */
