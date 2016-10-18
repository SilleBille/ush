# Makefile for ush
#
# Vincent W. Freeh
# 

CC=gcc
CFLAGS=-g
SRC=ush.c parse.c parse.h
OBJ=ush.o parse.o

all: ush

ush:	$(OBJ)
	$(CC) -o $@ $(OBJ)


clean:
	rm *.o ush

#ush.1.ps:	ush.1
#	groff -man -T ps ush.1 > ush.1.ps
