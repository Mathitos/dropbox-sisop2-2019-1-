############################# Makefile ##########################
all: client server
client: ./src/client.c dropbox.a ./include/util.h
	gcc -o client ./src/client.c -I./include -L./lib -ldropbox

server: ./src/server.c dropbox.a ./include/util.h ./include/fila2.h
        # O compilador faz a ligação entre os dois objetos
	gcc -o server ./src/server.c -I./include -L./lib -ldropbox -pthread
#-----> Distancia com o botão TAB ### e não com espaços

dropbox.a: fila2.o util.o
	ar crs ./lib/libdropbox.a util.o fila2.o

util.o: ./src/util.c ./include/util.h
	gcc -c -I./include ./src/util.c

fila2.o: ./src/fila2.c ./include/fila2.h
	gcc -c -I./include ./src/fila2.c

clean:
	rm *.o server client
	rm lib/*.a
