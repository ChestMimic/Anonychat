CFLAGS=		$(shell pkg-config --cflags glib-2.0)
LIB=		$(shell pkg-config --libs glib-2.0)

CC=			gcc $(CFLAGS)


all: msg.o list.o tree.o

msg.o: msg.c msg.h
	$(CC) -c msg.c 
	
list.o: list.c list.h
	$(CC) -c list.c

tree.o: tree.c tree.h
	$(CC) -c tree.c
	
clean:
	rm *.o
