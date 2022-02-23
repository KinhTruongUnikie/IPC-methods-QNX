/*
 * ipc_methods.c
 *
 *  Created on: Feb 21, 2022
 *      Author: kt
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ipc_methods.h"
#include "msg.h"

// struct option longopts array
opts longopts[] = {
		{"help", 0, NULL, 'h'},
		{"message", 1, NULL, 'm'},
		{"pipe", 1, NULL, 'p'},
		{"queue", 1, NULL, 'q'},
		{"shm", 1, NULL, 's'},
		{"file", 1, NULL, 'f' },
		{0, 0, 0, 0}
};

// program initial instruction
void printInstruction() {
	printf("This program is a file data transfer program in IPC environment, it is for IPC methods practice.\n"
			"The program shall receive an argument starts with \"--\"\n"
			"The options below are available for different file transfer usages(may require argument), --file option is mandatory:\n\n"
			"--help(-h): display all commands instruction and description(no arguments)\n"
			"--message(-m): uses QNX native IPC message passing method, the arguments of both client and server must be the same\n"
			"--queue(-q): uses queue as IPC method(TBD)\n"
			"--pipe(-p): uses pipe as IPC method(TBD)\n"
			"--shm(-s): uses shared memory buffer as IPC method(argument required)\n"
			"--file(-f): file used for read/write data(argument required)\n\n"
			);
}

// check command line options and arguments and return ipc_info for ipc data transfer
ipc_info checkOptions (int argc, char* argv[]) {
	int option;
	int mflag = 0;
	int pflag = 0;
	int qflag = 0;
	int sflag = 0;
	int fflag = 0;
	char *filename = NULL;
	// method id for choosing ipc method
	ipc_info info;
	while ((option = getopt_long(argc, argv, "hm:p:q:s:f:", longopts, NULL)) != -1) {
		switch(option) {
			case 'h': {
				printf("<HELP>\n"
						"--message: client-server model, carries priority, QNX native API(argument <serverName>)\n"
						"--pipe: POSIX, one directional message flow, slow, does not carry priority, portable(argument <TBD>)\n"
						"--queue: POSIX, basically pipe with extra feature(argument <TBD>)\n"
						"--shm: use shared memory region for message passing, required synchronization measure, e.g mutex, condvar(argument <bufferSize>)\n"
						"--file: add the file for data transfer (argument <fileName>)\n"
						"</HELP>\n"
						);
				break;
			}
			case 'm': {
				mflag++;
				info.argument_string = optarg;
				break;
			}
			case 'p': {
				pflag++;
				break;
			}
			case 'q': {
				qflag++;
				break;
			}
			case 's': {
				sflag++;
				info.argument_int = atoi(optarg);
				break;
			}
			case 'f': {
				fflag++;
				filename = optarg;
				info.filename = filename;
				break;
			}
			default:
				printf("Invalid options!\n");
				break;
		}

	}
	if (fflag == 1) {      // fflag > 1 is invalid option
		if (mflag + pflag + qflag + sflag != 1) {
			printf("Only one IPC method is allowed!(no duplication) e.g ./temp --message <serverName> --file <fileName>\n");
			exit(EXIT_FAILURE);
		} else {
			if(mflag) {
				printf("<message is used>\n");
				info.method = message;
			} else if(pflag){
				printf("<pipe is used>\n");
				info.method = pipes;
			} else if (qflag) {
				printf("<queue is used>\n");
				info.method = queue;
			} else {
				printf("<shm is used>\n");
				info.method = shm;
			}
		}
	} else {
		printf("A filename must be given as an argument for option --file/-f(only one -f is allowed)\n");
		exit(EXIT_FAILURE);
	}
	return info;
}

void messageSend(const ipc_info *info) {
	printf("Starting messageSend..\n"
			"If client specified server name does not exist, this program is stuck in an infinite loop looking for a server\n"
			);

	FILE *read;
	// open file for reading
	read = fopen(info->filename, "rb");
	if (read == NULL) {
		printf("File name does not exist! Exiting the program..\n");
		exit(EXIT_FAILURE);
	}

    int coid, status, size;
    header_t header;
    char* buffer = NULL;
    iov_t siov[2];

    coid = name_open(info->argument_string, 0);
    printf("Looking for server..\n");
    //Server search loop
    while (coid == -1)
    {
        sleep(1);
        coid = name_open(info->argument_string, 0);
    }
    printf("Found the server connection!\n");

    //Start message reading
	fseek(read, 0, SEEK_END);
	size = ftell(read);
	fseek(read, 0, SEEK_SET);


	header.msg_type = HEADER_IOV_TYPE;
	header.data_size = size + 1;
	buffer = (char*)malloc(header.data_size);
	//read the file
    fread(buffer, size, 1, read);

    fclose(read);

	SETIOV(&siov[0], &header, sizeof(header));
	SETIOV(&siov[1], buffer, header.data_size);

	free(buffer);

	status = MsgSendvs(coid, siov, 2, NULL, 0);
	if (status == -1) {
		perror("MsgSendv");
		exit(EXIT_FAILURE);
	}
	MsgSendPulse(coid, -1, EOF_PULSE_CODE, 0);
}
void messageReceive(const ipc_info *info) {
	printf("Starting messageReceive..\n");
	int rcvid;
	name_attach_t *attach;
	rbufs rbuf;
	int eof = 0;

	attach = name_attach(NULL, info->argument_string, 0);
	while (1) {
		rcvid = MsgReceive(attach->chid, &rbuf, sizeof(rbuf), NULL);
		if (rcvid == -1)
		{ //was there an error receiving msg?
			perror("MsgReceive"); //look up errno code and print
			exit(EXIT_FAILURE); //give up
		} else if (rcvid == 0) {
			if (rbuf.pulse.code == _PULSE_CODE_DISCONNECT) {
				printf("Client %x(scoid) disconnected\n", rbuf.pulse.scoid);
				// free up disconnected scoid
				if (ConnectDetach(rbuf.pulse.scoid) == -1) {
					perror("ConnectDetach\n");
				}
				if (eof) {
					printf("Exiting the program..\n");
					break;
				}
			} else if (rbuf.pulse.code == EOF_PULSE_CODE) {
				// client delivered all file content and notify the server to exit
				printf("Client reached end of file\n");
				eof = 1;

			} else {
				printf("Server receive a pulse code of %d and value of %x\n",
						rbuf.pulse.code, rbuf.pulse.value.sival_int);
			}
			//printf("Server receive a pulse code of %d and value of %x\n", rbuf.pulse.code, rbuf.pulse.value.sival_int);
		} else {
			if (rbuf.type == HEADER_IOV_TYPE) {
				char *data = NULL;
				FILE *write;
				// open file for writing
				write = fopen(info->filename, "wb");
				if (write == NULL) {
					perror("Failed to open file!");
					exit(EXIT_FAILURE);
				}
				data = malloc(rbuf.header_msg.data_size);

				if (MsgRead(rcvid, data, rbuf.header_msg.data_size, sizeof(header_t)) == -1) {
					perror("MsgRead");
					exit(EXIT_FAILURE);
				} else {
					// write into file
					fwrite(data, rbuf.header_msg.data_size, 1, write);
					free(data);
					fclose(write);
				}

				if (MsgReply(rcvid, EOK, NULL, 0) == -1) {
					perror("MsgReply");
					exit(EXIT_FAILURE);
				}
			} else {
				// unknown message type, unblock client with an error
				if (MsgError(rcvid, ENOSYS) == -1)
					perror("MsgError");
			}
		}
	}
}

void pipeSend(const ipc_info *info){
}

void pipeReceive(const ipc_info *info){

}

void queueSend(const ipc_info *info){

}
void queueReceive(const ipc_info *info){

}

void shmSend(const ipc_info *info){

}
void shmReceive(const ipc_info *info){

}

void run_IPC(ipc_info *info, int send) {
	if (info->method == message) {
		(send)? messageSend(info): messageReceive(info);
	} else if (info->method == pipes) {
		(send)? pipeSend(info): pipeReceive(info);
	} else if (info->method == queue) {
		(send)? queueSend(info): queueReceive(info);
	} else if (info->method == shm) {
		(send)? shmSend(info): shmReceive(info);
	} else {
		exit(EXIT_FAILURE);
	}
}
