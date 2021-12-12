CC = g++
CFLAGS = -Wall -std=c++1z -w -pthread -g
FLAGS = -I. -L. -L/usr/lib -lssl -lcrypto
DEPS = common.h message_server.h respond_to_request_server.h 
OBJ = server.o message_server.o respond_to_request_server.o 

%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) -I. -c -o $@ $<

tracker: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean :
	rm $(OBJ)