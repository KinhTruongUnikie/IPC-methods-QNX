
#
#	Makefile for ipc methods project
#

DEBUG = -g
CC = qcc
LD = qcc

TARGET = -Vgcc_ntox86_64
#TARGET = -Vgcc_ntoarmv7le
#TARGET = -Vgcc_ntoaarch64le

CFLAGS += $(DEBUG) $(TARGET) -Wall
LDFLAGS+= $(DEBUG) $(TARGET)

BINS = ipc_sendfile ipc_receivefile 

all:	$(BINS)

ipc_sendfile: ipc_sendfile.c ipc_methods.o
ipc_receivefile: ipc_receivefile.c ipc_methods.o

clean:
	rm -f *.o $(BINS)