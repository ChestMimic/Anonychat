INC=-I../ac-lib/
LIB=../ac-lib
CFLAGS=		$(shell pkg-config --cflags glib-2.0)

OBJECTS= $(LIB)/base64.o $(LIB)/list.o $(LIB)/msg.o $(LIB)/enc.o
GCC=gcc $(CFLAGS) $(INC)


all: client_server test

test: test.o $(OBJECTS) key_table.o
	$(GCC) -g test.o key_table.o $(OBJECTS) -o test -lcrypto -lssl -ldl -lglib-2.0 

client_server: thread_util.o client_server.o key_table.o $(OBJECTS)
	$(GCC) -lpthread -g thread_util.o client_server.o key_table.o $(OBJECTS) -o client_server -lssl -lcrypto -lglib-2.0 -ldl

thread_util.o: thread_util.c thread_util.h
	$(GCC) -c thread_util.c

client_server.o: client_server.c thread_util.h
	$(GCC) -c client_server.c
	
test.o: test.c
	$(GCC) -c test.c

key_table.o: key_table.c key_table.h
	$(GCC) -c key_table.c

clean:
	rm  -f *.o client_server test
