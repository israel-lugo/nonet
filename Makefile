
CC ?= gcc
CFLAGS ?= -DDEBUG=1 -g -W -Wall

all: nonet

nonet: nonet.o

clean:
	rm -f nonet nonet.o
