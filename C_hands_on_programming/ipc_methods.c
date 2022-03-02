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

#include <fcntl.h>
#include <mqueue.h>
#include <sys/mman.h>

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
			"--queue(-q): uses queue as IPC method(argument required)\n"
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
						"--queue: POSIX, basically pipe with extra feature(argument <queueName>)\n"
						"--shm: use shared memory region for message passing, required synchronization measure, e.g mutex, condvar(argument <shmName>)\n"
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
				info.argument_string = optarg;
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
	     close(fd);
	     free(buffer);
	     exit(EXIT_FAILURE);
	 }
	 close(fd);
	 //write to pipe
	 if (write(fdp, buffer, size) == -1) {
	     perror("write\n");
	     free(buffer);
	     exit(EXIT_FAILURE);
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
	while ((fdp = open(info->argument_string, O_RDONLY)) == -1) {
		sleep(1);
	}

	 // open file for writing
	 fd = open(info->filename, O_WRONLY | O_CREAT, 0644);//mode(file permission should be specified if O_CREATE is used)
	 if (fd == -1) {
		 perror("open");
		 exit(EXIT_FAILURE);
	 }
	 // read from pipe and write to file
	 buffer = (char*)malloc(PIPE_BUF);
	 while ((bytes = read(fdp, buffer, PIPE_BUF)) != 0) {
		 if (write(fd, buffer, bytes) == -1) {
			 perror("write");
			 free(buffer);
			 exit(EXIT_FAILURE);
		 }
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

	buffer = (char*)malloc(size);
	// read the file into buffer
	if (read(fd, buffer, size) == -1) {
		perror("read");
		free(buffer);
		exit(EXIT_FAILURE);
	}
	printf("Messages in the queue before sent: %ld\n", attrs.mq_curmsgs);
	// send the data into message queue
	if (mq_send(msg_queue, buffer, size, 0) == -1) {
		perror("mq_send");
		free(buffer);
		exit(EXIT_FAILURE);
	} else {
		mq_getattr(msg_queue, &attrs);
		printf("Messages in the queue after sent: %ld\n", attrs.mq_curmsgs);
		printf("File data is sent into the queue, exiting the program..\n");
		free(buffer);
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
	// open the message queue in a loop until it is created by the other process
	printf("Looking for the message queue..\n");
	while ((msg_queue = mq_open(info->argument_string, O_RDONLY)) == -1) {
		sleep(1);
	}
	printf("Found the requested queue\n");
	// get the queue attributes
	mq_getattr(msg_queue, &attrs);
	n = attrs.mq_maxmsg;

	//open file for writing
	writeFile = fopen(info->filename, "wb");
	if (writeFile == NULL) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}

	buffer = (char*)malloc(attrs.mq_msgsize);
	// receive the message from the queue

	for (int i = 0; i < n; i++) {
		mq_receive(msg_queue, buffer, attrs.mq_msgsize, NULL);
	}
	mq_getattr(msg_queue, &attrs);
	printf("Messages in the queue after receive: %ld\n", attrs.mq_curmsgs);
	if (fwrite(buffer, attrs.mq_msgsize, 1, writeFile) != 1) {
		perror("fwrite");
		free(buffer);
		exit(EXIT_FAILURE);
	} else {
		fclose(writeFile);
		free(buffer);
		//mq_getattr(msg_queue, &attrs);
		mq_close(msg_queue);
		// remove the queue name, the actual removal only happens after all processes(opening the mq) close the mq after this call is made
		mq_unlink(info->argument_string);
		printf("Successfully write data into file, exiting the program..\n");
	}
}

void unlink_and_exit(char* shm_name) {
	(void)shm_unlink(shm_name);
	exit(EXIT_FAILURE);
}

void *get_shared_memory_pointer( char *name)
{
	shm_t *ptr;
	int fd;

	printf("Getting access to shared memory region..(Program would stuck in a loop for non-exist shm name\n");

	while ((fd = shm_open(name, O_RDWR, 0)) == -1) {
		/* wait one second then try again */
		sleep(1);
	}

	while ((ptr = mmap(0, sizeof(shm_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
		sleep(1);
	}
	/* no longer need fd */
	(void)close(fd);

	while (!ptr->init) {
		sleep(1);
	}
	return ptr;
}

void check_leading_slash(char* name) {
	if (*name != '/') {
		perror("Shared memory name should start with leading '/' character");
		exit(EXIT_FAILURE);
	}
}

void shmSend(const ipc_info *info){
	printf("Starting shmSend..\n");
	check_leading_slash(info->argument_string);

	int fd, fd1, size, bytes_read, total;
	shm_t *shm_ptr;
	pthread_mutexattr_t m_attr;
	pthread_condattr_t c_attr;
	struct stat st;

	// Use stat to find the size of the file
	stat(info->filename, &st);
	size = st.st_size;
	//open the file for reading
	fd1 = open(info->filename, O_RDONLY);
	if (fd1 == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	// create shared memory object
	fd = shm_open(info->argument_string, O_RDWR | O_CREAT, 0666);
	if (fd == -1) {
		perror("shm_open");
		unlink_and_exit(info->argument_string);
	}
	// allocate the size of the shared memory object
	if (ftruncate(fd, sizeof(shm_t)) == -1) {
		perror("ftruncate");
		unlink_and_exit(info->argument_string);
	}
	// get a pointer to shared memory region
	shm_ptr = mmap(NULL, sizeof(shm_t), PROT_WRITE, MAP_SHARED, fd, 0);
	if (shm_ptr == MAP_FAILED) {
		perror("mmap");
		unlink_and_exit(info->argument_string);
	}
	//close fd
	close(fd);

	pthread_mutexattr_init(&m_attr);
	pthread_mutexattr_setpshared(&m_attr, PTHREAD_PROCESS_SHARED);
	if (pthread_mutex_init(&shm_ptr->mutex, &m_attr) != EOK) {
		perror("pthread_mutex_init");
		unlink_and_exit(info->argument_string);
	}

	pthread_condattr_init(&c_attr);
	pthread_condattr_setpshared(&c_attr, PTHREAD_PROCESS_SHARED);
	if (pthread_cond_init(&shm_ptr->cond, &c_attr) != EOK) {
		perror("pthread_cond_init");
		unlink_and_exit(info->argument_string);
	}
	// initialized shared memory object member values
	shm_ptr->init = 1;
	shm_ptr->sent = 0;
	shm_ptr->end = 0;
	// enter critical section, lock the mutex
	total = 0;
	while (total < size) {
		if (pthread_mutex_lock(&shm_ptr->mutex) != EOK) {
			perror("pthread_mutex_lock");
			unlink_and_exit(info->argument_string);
		}
		// check sent status and blocked if data still has not been retrieved 
		while (shm_ptr->sent) {
			pthread_cond_wait(&shm_ptr->cond, &shm_ptr->mutex);
		}
		//read the file into shared buffer;
		if ((bytes_read = read(fd1, shm_ptr->buffer, SHM_SIZE)) == -1) {
			perror("read\n");
			exit(EXIT_FAILURE);
		}
		total += bytes_read;
		if (total == size) {
			shm_ptr->end = 1;
		}
		// update shared memory object
		shm_ptr->sent = 1;
		shm_ptr->data_size = bytes_read;
		// unlock the mutex
		if (pthread_mutex_unlock(&shm_ptr->mutex) != EOK) {
			perror("pthread_mutex_unlock");
			unlink_and_exit(info->argument_string);
		}
		//wake up process which is currently condvar blocked
		if (pthread_cond_broadcast(&shm_ptr->cond) != EOK) {
			perror("pthread_cond_broadcast");
			unlink_and_exit(info->argument_string);
		}
	}
	close(fd);
	close(fd1);
	if (munmap(shm_ptr, sizeof(shm_t)) == -1)
	{
		perror("munmap");
	}
	printf("Data successfully sent\n");
}

void shmReceive(const ipc_info *info){
	printf("Starting shmReceive..\n");
	check_leading_slash(info->argument_string);

	shm_t *shm_p;
	int fd, done;
	done = 0;
	// get shared memory object pointer
	shm_p = get_shared_memory_pointer(info->argument_string);
	//open file for writing
	fd = open(info->filename, O_WRONLY | O_CREAT, 0666);
	if (fd == -1) {
		perror("open");
		unlink_and_exit(info->argument_string);
		exit(EXIT_FAILURE);
	}

	while (!done) {
		// enter critical section, lock the mutex
		if (pthread_mutex_lock(&shm_p->mutex) != EOK) {
			perror("pthread_mutex_lock");
			unlink_and_exit(info->argument_string);
		}
		// check end status 
		if (shm_p->end) {
			done = shm_p->end;
		}
		// check sent status and blocked if shmSend still has not sent new data at this point
		while (!shm_p->sent) {
			if (pthread_cond_wait(&shm_p->cond, &shm_p->mutex) != EOK) {
				perror("pthread_cond_wait");
				unlink_and_exit(info->argument_string);
			}
		}
		// write into file
		if (write(fd, shm_p->buffer, shm_p->data_size) == -1) {
			perror("fwrite");
			unlink_and_exit(info->argument_string);
		}
		// reset sent status
		shm_p->sent = 0;
		// unlock the mutex
		if (pthread_mutex_unlock(&shm_p->mutex) != EOK) {
			perror("pthread_mutex_unlock");
			unlink_and_exit(info->argument_string);
		}
		//wake up process which is currently condvar blocked
		if (pthread_cond_broadcast(&shm_p->cond) != EOK) {
			perror("pthread_cond_broadcast");
			unlink_and_exit(info->argument_string);
		}
	}
	if (shm_unlink(info->argument_string) == -1) {
		perror("shm_unlink");
	}
	printf("Successfully write data into file, exiting the program..\n");
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
