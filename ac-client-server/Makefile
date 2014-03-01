INC=-I../ac-lib/
LIB=../ac-lib
CFLAGS=		$(shell pkg-config --cflags glib-2.0)

OBJECTS= $(LIB)/base64.o $(LIB)/list.o $(LIB)/msg.o $(LIB)/enc.o
GCC=gcc $(CFLAGS) $(INC)


all: client_server enc-test

enc-test: enc-test.o $(OBJECTS) key_table.o
	$(GCC) -g $^ -o enc-test -lcrypto -lssl -ldl -lglib-2.0 

client_server: thread_util.o client_server.o key_table.o msg-test.o $(OBJECTS)
	$(GCC) -lpthread -g $^ -o client_server -lssl -lcrypto -lglib-2.0 -ldl

thread_util.o: thread_util.c thread_util.h
	$(GCC) -c thread_util.c

client_server.o: client_server.c thread_util.h
	$(GCC) -c client_server.c
	
enc-test.o: enc-test.c
	$(GCC) -c enc-test.c
	
msg-test.o: msg-test.c
	$(GCC) -c msg-test.c

key_table.o: key_table.c key_table.h
	$(GCC) -c key_table.c

clean:
	rm  -f *.o client_server enc-test
