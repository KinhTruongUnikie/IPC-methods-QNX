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
#include <mqueue.h>
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
			"--pipe(-p): uses name pipe(FIFO) as IPC method(argument required)\n"
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
	// ipc methods and file info
	ipc_info info;
	while ((option = getopt_long(argc, argv, "hm:p:q:s:f:", longopts, NULL)) != -1) {
		switch(option) {
			case 'h': {
				printf("<HELP>\n"
						"--message: client-server model, carries priority, QNX native API(argument <serverName>)\n"
						"--pipe: POSIX, portable, does not carry priority(argument <pipeName>)\n"
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
				info.argument_string = optarg;
				break;
			}
			case 'q': {
				qflag++;
				info.argument_string = optarg;
				break;
			}
			case 's': {
				sflag++;
				info.argument_int = atoi(optarg);
				break;
			}
			case 'f': {
				fflag++;
				info.filename = optarg;
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
				info.method = MESSAGE;
			} else if(pflag){
				printf("<pipe is used>\n");
				info.method = PIPES;
			} else if (qflag) {
				printf("<queue is used>\n");
				info.method = QUEUE;
			} else {
				printf("<shm is used>\n");
				info.method = SHM;
			}
		}
	} else {
		printf("A filename must be given as an argument for option --file/-f(only one -f is allowed)\n");
		exit(EXIT_FAILURE);
	}
	return info;
}
// This version of messageSend implemented using read()
#if 0
void messageSend(const ipc_info *info) {
	printf("Starting messageSend..\n"
			"If client specified server name does not exist, this program is stuck in an infinite loop looking for a server\n"
			);

	int fd, current, coid, status, size;
    header_t header;
    char* buffer = NULL;
    iov_t siov[2];
	// open file for reading
	fd = open(info->filename, O_RDONLY);
	if (fd == -1) {
		perror("open\n");
		exit(EXIT_FAILURE);
	}

	printf("Looking for server..\n");
    //Server search loop
    while ((coid = name_open(info->argument_string, 0)) == -1)
    {
        sleep(1);
    }
    printf("Found the server connection!\n");

    //Start message reading

    current = lseek(fd, 0, SEEK_CUR);
    size = lseek(fd, 0, SEEK_END);
    //set the file pointer back to beginning of file
    lseek(fd, current, SEEK_SET);

	header.msg_type = HEADER_IOV_TYPE;
	header.data_size = size;

    if ((buffer = (char*)malloc(size)) == NULL) {
    	perror("malloc\n");
    	free(buffer);
    	exit(EXIT_FAILURE);
    }

	//read the file
    if (read(fd, buffer, size) == -1) {
    	perror("read\n");
    	exit(EXIT_FAILURE);
    }

    close(fd);

	SETIOV(&siov[0], &header, sizeof(header));
	SETIOV(&siov[1], buffer, header.data_size);

	status = MsgSendvs(coid, siov, 2, NULL, 0);
	if (status == -1) {
		perror("MsgSendvs");
		exit(EXIT_FAILURE);
	}
	free(buffer);

	if (MsgSendPulse(coid, -1, EOF_PULSE_CODE, 0) == -1) {
		perror("MsgSendPulse\n");
		exit(EXIT_FAILURE);
	} else {
		printf("Exiting the program..\n");
	}

}
#endif

//This version of messageSend implemented using fread()
#if 1
void messageSend(const ipc_info *info) {
	printf("Starting messageSend..\n"
			"If client specified server name does not exist, this program is stuck in an infinite loop looking for a server\n"
			);

    int coid, status, size;
    header_t header;
    char* buffer = NULL;
    iov_t siov[2];
	FILE *read;
	// open file for reading
	read = fopen(info->filename, "rb");
	if (read == NULL) {
		printf("File name does not exist! Exiting the program..\n");
		exit(EXIT_FAILURE);
	}

	printf("Looking for server..\n");
    //Server search loop
    while ((coid = name_open(info->argument_string, 0)) == -1)
    {
        sleep(1);
    }
    printf("Found the server connection!\n");

    //Start message reading
	fseek(read, 0, SEEK_END);
	size = ftell(read);
	fseek(read, 0, SEEK_SET);

	header.msg_type = HEADER_IOV_TYPE;
	header.data_size = size;

	if ((buffer = (char*)malloc(size)) == NULL) {
    	perror("malloc\n");
    	exit(EXIT_FAILURE);
	}


	//read the file
    if (fread(buffer, size, 1, read) != 1) {
    	perror("fread\n");
    	exit(EXIT_FAILURE);
    }

    fclose(read);

	SETIOV(&siov[0], &header, sizeof(header));
	SETIOV(&siov[1], buffer, header.data_size);

	status = MsgSendvs(coid, siov, 2, NULL, 0);
	if (status == -1) {
		perror("MsgSendv");
		exit(EXIT_FAILURE);
	}
	free(buffer);

	if (MsgSendPulse(coid, -1, EOF_PULSE_CODE, 0) == -1) {
		perror("MsgSendPulse\n");
		exit(EXIT_FAILURE);
	} else {
		printf("Data is delivered. Exiting the program..\n");
	}
}
#endif

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
					printf("Data is received. Exiting the program..\n");
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
		} else {
			if (rbuf.type == HEADER_IOV_TYPE) {
				char *data = NULL;
				FILE *writeFile;
				// open file for writing
				writeFile = fopen(info->filename, "wb");
				if (writeFile == NULL) {
					perror("Failed to open file!");
					exit(EXIT_FAILURE);
				}
				data = malloc(rbuf.header_msg.data_size);

				if (MsgRead(rcvid, data, rbuf.header_msg.data_size, sizeof(header_t)) == -1) {
					perror("MsgRead");
					exit(EXIT_FAILURE);
				} else {
					// write into file
					fwrite(data, rbuf.header_msg.data_size, 1, writeFile);
					free(data);
					fclose(writeFile);
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
	printf("Starting pipeSend..\n");
	// remove potentially existing FIFO
	remove(info->argument_string);
	int fdp, fd, current, size;
	char* buffer = NULL;

	// create a name pipe
	if (mkfifo(info->argument_string, 0666) == -1) {
		perror("mkfifo");
		exit(EXIT_FAILURE);
	}

	fdp = open(info->argument_string, O_WRONLY);
	// open name pipe
	 if (fdp == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	 }

	 // open file
	 fd = open(info->filename, O_RDONLY);
	 if (fd == -1) {
		 perror("open");
		 exit(EXIT_FAILURE);
	 }

	 current = lseek(fd, 0, SEEK_CUR);
	 size = lseek(fd, 0, SEEK_END);
	 //set the file pointer back to beginning of file
	 lseek(fd, current, SEEK_SET);

	 // read file
	 buffer = (char*)malloc(size);
	 if (read(fd, buffer, size) == -1) {
	     perror("read\n");
	     exit(EXIT_FAILURE);
	     free(buffer);
	 }
	 close(fd);
	 //write to pipe
	 if (write(fdp, buffer, size) == -1) {
	     perror("write\n");
	     exit(EXIT_FAILURE);
	     free(buffer);
	 }

	close(fdp);
	free(buffer);
	printf("File delivered by pipe successfully, exiting the program..\n");
}

void pipeReceive(const ipc_info *info){
	printf("Starting pipeReceive..\n");

	int fdp, fd;
	char* buffer = NULL;
	int bytes;

	// open name pipe for reading
	while ((fdp = open(info->argument_string, O_RDONLY)) == -1){}

	 // open file for writing
	 fd = open(info->filename, O_WRONLY | O_CREAT, 0644);//mode(file permission should be specified if O_CREATE is used)
	 if (fd == -1) {
		 perror("open");
		 exit(EXIT_FAILURE);
	 }
	 // read from pipe and write to file
	 buffer = (char*)malloc(PIPE_BUF);
	 while ((bytes = read(fdp, buffer, PIPE_BUF)) != 0) {
		 write(fd, buffer, bytes);
	 }

	 close(fd);
	 close(fdp);
	 free(buffer);
	 //server remove the FIFO after data is written into file
	 remove(info->argument_string);
	 printf("Write to file from pipe successfully, exiting the program..\n");
}

void queueSend(const ipc_info *info){
	printf("Starting queueSend..\n");
	struct mq_attr attrs;
	struct stat st;
	int size, fd;
	char *buffer = NULL;
	mqd_t msg_queue; //message queue descriptor(file descriptor)

	// Use stat to find the size of the file
	stat(info->filename, &st);
	size = st.st_size;
	printf("stat: %d\n", size);
	buffer = (char*)malloc(size);

	memset(&attrs, 0, sizeof attrs);
	attrs.mq_maxmsg = 1;
	attrs.mq_msgsize = size;
	// open or create a mqueue if it does not exist
	msg_queue = mq_open(info->argument_string, O_WRONLY | O_CREAT, 0666, &attrs);
	if (msg_queue == -1) {
		perror("msg_open");
		exit(EXIT_FAILURE);
	}
	// open file for reading
	fd = open(info->filename, O_RDONLY);
	if (fd == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}
	// read the file into buffer
	if (read(fd, buffer, size) == -1) {
		perror("read");
		free(buffer);
		exit(EXIT_FAILURE);
	}
	// send the data into message queue
	if (mq_send(msg_queue, buffer, size, 0) == -1) {
		perror("mq_send");
		free(buffer);
		exit(EXIT_FAILURE);
	} else {
		printf("File data is sent into the queue, exiting the program..\n");
		mq_close(msg_queue);
		close(fd);
	}

}
void queueReceive(const ipc_info *info){
	printf("Starting queueReceive..\n");
	struct mq_attr attrs;
	int n;
	char *buffer = NULL;
	mqd_t msg_queue;
	FILE *writeFile;
	// open or create a mqueue if it does not exist
	printf("Looking for the message queue:\n");
	while ((msg_queue = mq_open(info->argument_string, O_RDONLY)) == -1);
	// get the queue attributes
	mq_getattr(msg_queue, &attrs);
	printf("%ld\n", attrs.mq_msgsize);
	n = attrs.mq_maxmsg;
	buffer = (char*)malloc(attrs.mq_msgsize);

	//open file for writing
	writeFile = fopen(info->filename, "wb");
	if (writeFile == NULL) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}
	printf("%ld\n", attrs.mq_msgsize);
	// receive the message from the queue
	for (int i = 0; i < n; i++) {
		mq_receive(msg_queue, buffer, attrs.mq_msgsize, NULL);
	}
	printf("%ld\n", attrs.mq_msgsize);

	if (fwrite(buffer, attrs.mq_msgsize, 1, writeFile) != 1) {
		perror("fwrite");
		exit(EXIT_FAILURE);
	} else {
		fclose(writeFile);
		mq_close(msg_queue);
		// remove the queue name, the actual removal only happens after all processes(opening the mq) close the mq after this call is made
		mq_unlink(info->argument_string);
		printf("Successfully write data into file, exiting the program..\n");
	}
}

void shmSend(const ipc_info *info){

}
void shmReceive(const ipc_info *info){

}

void run_IPC(ipc_info *info, int send) {
	if (info->method == MESSAGE) {
		(send)? messageSend(info): messageReceive(info);
	} else if (info->method == PIPES) {
		(send)? pipeSend(info): pipeReceive(info);
	} else if (info->method == QUEUE) {
		(send)? queueSend(info): queueReceive(info);
	} else if (info->method == SHM) {
		(send)? shmSend(info): shmReceive(info);
	} else {
		exit(EXIT_FAILURE);
	}
}
