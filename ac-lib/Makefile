all: msg.o list.o

msg.o: msg.c msg.h
	gcc -c msg.c 
	
client_list.o: list.c list.h
	gcc -c list.c
	
clean:
	rm *.o