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


char* checkOptions (int argc, char* argv[]) {
	int option;
	int mflag = 0;
	int pflag = 0;
	int qflag = 0;
	int sflag = 0;
	int fflag = 0;
	char *filename = NULL;
	while ((option = getopt_long(argc, argv, "hm:p:q:s:f:", longopts, NULL)) != -1) {
		switch(option) {
			case 'h': {
				printf("help\n");
				break;
			}

			case 'm': {
				mflag++;
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
				break;
			}

			case 'f': {
				fflag++;
				filename = optarg;
				break;
			}

			default:
				printf("Invalid options!\n");
				break;
		}

	}
	if (fflag == 1) {      // fflag > 1 is invalid option
		if (mflag + pflag + qflag + sflag != 1) {
			printf("Only one IPC method is allowed! e.g ./temp --message\n");
			//exit(EXIT_FAILURE);
		} else {
			if(mflag) {
				printf("m is used\n");
			} else if(pflag){
				printf("p is used\n");
			} else if (qflag) {
				printf("q is used\n");
			} else {
				printf("s is used\n");
			}
		}
	} else {
		printf("A filename must be given as an argument for option --file/-f(only one -f is allowed)\n");
		//exit(EXIT_FAILURE);
	}
	return filename;
}
