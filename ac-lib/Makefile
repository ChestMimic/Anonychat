
CC=			gcc
CFLAGS=		$(shell pkg-config --cflags glib-2.0)
LIB=		$(shell pkg-config --libs glib-2.0)


all: msg.o list.o

msg.o: msg.c msg.h
	$(CC) -c $(CFLAGS) msg.c 
	
list.o: list.c list.h
	$(CC) -c $(CFLAGS) list.c
	
clean:
	rm *.o