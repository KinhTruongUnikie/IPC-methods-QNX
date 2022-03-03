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

#define SHM_SIZE 4096
typedef struct option opts;

typedef enum {
	MESSAGE,
	PIPES,
	QUEUE,
	SHM,
	NONE
}methods;

typedef struct ipc_info{
	methods method;
	char *filename;
	char *argument_string;
	int argument_int;
} ipc_info;

typedef struct {
	volatile unsigned init;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	int sent;
	char buffer[SHM_SIZE];
	int end;
}shm_t;
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
void check_leading_slash(char* name);
void unlink_and_exit(char* name);
void *get_shared_memory_pointer( char *name);

#endif /* IPC_;METHODS_H_ */
