INC=-I../ac-lib/
LIB=../ac-lib
CFLAGS=		$(shell pkg-config --cflags glib-2.0)

GCC=gcc $(CFLAGS) $(INC)


all: client_server

client_server: thread_util.o client_server.o
	$(GCC) -lpthread -g thread_util.o client_server.o -o client_server

thread_util.o: thread_util.c thread_util.h
	$(GCC) -c thread_util.c

client_server.o: client_server.c thread_util.h
	$(GCC) -c client_server.c

clean:
	rm  -f *.o client_server
