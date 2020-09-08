#Makefile
CC=gcc -pipe 
CFLAGS=-Wall -g
OBJS=ssb_gen.o test_ssb.o
HEADERS=ssb_gen.h
LIBS=-lsndfile 


%.o:%.c $(HEADERS) Makefile
	$(CC) $(CFLAGS) -c $< -o $@
	
testssb:$(OBJS) $(HEADERS) test_ssb.o
	$(CC) -o testssb $(OBJS)  $(LIBS)	


