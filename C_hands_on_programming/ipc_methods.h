/*
 * ipc_methods.h
 *
 *  Created on: Feb 21, 2022
 *      Author: kt
 */


#ifndef IPC_METHODS_H_
#define IPC_METHODS_H_

#include <getopt.h>
#include <sys/iomsg.h>

typedef struct option opts;

typedef enum {
	message,
	pipes,
	queue,
	shm,
	none
}methods;

typedef struct ipc_info{
	methods method;
	char *filename;
	char *argument_string;
	int argument_int;
} ipc_info;

void printInstruction();
ipc_info checkOptions (int argc, char* argv[]);
void run_IPC(ipc_info *info, int send);

void messageSend(const ipc_info *info);
void messageReceive(const ipc_info *info);

void pipeSend(const ipc_info *info);
void pipeReceive(const ipc_info *info);

void queueSend(const ipc_info *info);
void queueReceive(const ipc_info *info);

void shmSend(const ipc_info *info);
void shmReceive(const ipc_info *info);

#endif /* IPC_;METHODS_H_ */
