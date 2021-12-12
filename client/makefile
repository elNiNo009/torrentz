CC = g++
CFLAGS = -I. -L. -g -Wall -std=c++1z -w -pthread -L/usr/lib
FLAGS =  -lssl -lcrypto 
DEPS = common.h client_database.h create_torrent.h message_client.h respond_to_request_client.h
OBJ = client.o client_database.o create_torrent.o message_client.o respond_to_request_client.o

%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<  $(FLAGS)

client: $(OBJ)
	$(CC) $(CFLAGS)  -o $@ $^  $(FLAGS)

clean :
	rm $(OBJ)