/*
 * msg.h
 *
 *  Created on: Feb 19, 2022
 *      Author: kt
 */
#include <stdint.h>
#include <sys/dispatch.h>
#include <sys/neutrino.h>

#ifndef MSG_H_
#define MSG_H_


#define SEND 1
#define RECEIVE 0
#define HEADER_IOV_TYPE (_IO_MAX + 1)
#define EOF_PULSE_CODE _PULSE_CODE_MINAVAIL + 1

//message header struct
typedef struct
{
	uint16_t msg_type;
	unsigned data_size;
} header_t;

//message receive buffer union
typedef union {
	uint16_t type;
	header_t header_msg;
	struct _pulse pulse;
} rbufs;



#endif /* MSG_H_ */
