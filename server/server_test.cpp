#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string>
#include <sstream>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>

std::vector<std::string> handleRequest(std::string request) {
	std::vector<std::string> response;

	response.push_back(std::string("Pong!")); // Respond with "Pong!" 
	response.push_back(std::string("CLOSE")); // Close the connection with poison pill

	return response;
}

void handleConnection(int newsockfd, sockaddr_in* cli_addr) {
	char buffer[256]; // Initialize buffer to zeros
	bzero(buffer, 256);

	while (true) {
		int n = read(newsockfd, buffer, 255);
		if (n == 0) {
			std::cout << inet_ntoa(cli_addr->sin_addr) << ":" << ntohs(cli_addr->sin_port)
					<< " connection closed by client" << std::endl;
			return;
		}
		else if (n < 0)
			std::cerr << "ERROR reading from socket" << std::endl;

		std::stringstream stream;
		stream << buffer << std::flush;
		while (stream.good()) {
			std::string request;
			getline(stream, request); // Get and print request by lines
			if (request.length() > 0) {
				std::cout << inet_ntoa(cli_addr->sin_addr) << ":" << ntohs(cli_addr->sin_port)
						<< ": " << request << std::endl;

				std::vector<std::string> response = handleRequest(request); // Get the response

				for (int i = 0; i < response.size(); i++) {
					std::string output = response[i];
					if (output != "CLOSE") {
						n = write(newsockfd, output.c_str(), output.length()); // Write response by line
						if (n < 0)
							std::cerr << "ERROR writing to socket" << std::endl;
					}
					else {
						close(newsockfd); // Close the connection if response line == "CLOSE"
						std::cout << inet_ntoa(cli_addr->sin_addr) << ":" << ntohs(cli_addr->sin_port)
								<< " connection terminated" << std::endl;
						return;
					}
				}
			}
		}
	}
}

int main(int argc, const char *argv[]) {

	
	int sockfd; // Socket file descriptor
	int portno; // Port number

	sockaddr_in serv_addr; // Server address

	if (argc < 2) {
		std::cerr << "ERROR no port provided" << std::endl;
		exit(1);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0); // Create new socket, save file descriptor
	if (sockfd < 0) {
		std::cerr << "ERROR opening socket" << std::endl;
	}

	int reusePort = 1; // Disables default "wait time" after port is no longer in use before it is unbound
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &reusePort, sizeof(reusePort));

	bzero((char *) &serv_addr, sizeof(serv_addr)); // Initialize serv_addr to zeros
	portno = atoi(argv[1]); // Reads port number from char* array

	serv_addr.sin_family = AF_INET; // Sets the address family
	serv_addr.sin_port = htons(portno); // Converts number from host byte order to network byte order
	serv_addr.sin_addr.s_addr = INADDR_ANY; // Sets the IP address of the machine on which this server is running

	if (bind(sockfd, (sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) // Bind the socket to the address
		std::cerr << "ERROR on binding" << std::endl;

	unsigned int backlogSize = 5; // Number of connections that can be waiting while another finishes
	listen(sockfd, backlogSize);
	std::cout << "C++ server opened on port " << portno << std::endl;;

	while (true) {
		int newsockfd; // New socket file descriptor
		unsigned int clilen; // Client address size
		sockaddr_in cli_addr; // Client address

		clilen = sizeof(sockaddr_in);
		newsockfd = accept(sockfd, (sockaddr *) &cli_addr, &clilen); // Block until a client connects
		if (newsockfd < 0)
			std::cerr << "ERROR on accept" << std::endl;

		std::cout << inet_ntoa(cli_addr.sin_addr) << ":" << ntohs(cli_addr.sin_port)
				<< " connected" << std::endl;

		handleConnection(newsockfd, &cli_addr); // Handle the connection
	}
	return 0;
}