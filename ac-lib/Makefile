CFLAGS=		$(shell pkg-config --cflags glib-2.0)
LIB=		$(shell pkg-config --libs glib-2.0)

CC=			gcc $(CFLAGS)


all: msg.o list.o enc.o base64.o tree.o

msg.o: msg.c msg.h
	$(CC) -c msg.c 
	
list.o: list.c list.h
	$(CC) -c list.c

tree.o: tree.c tree.h
	$(CC) -c tree.c -lm
	
enc.o: enc.c enc.h msg.h
	$(CC) -c enc.c

base64.o: base64.c base64.h
	$(CC) -c base64.c

clean:
	rm *.o
