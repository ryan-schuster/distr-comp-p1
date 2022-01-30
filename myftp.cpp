// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <iostream>

using namespace std;

int main(int argc, char const *argv[])
{
	if (argc < 3) { //checks if less than three arguments
		printf("Usage: myftp <machine name> <port>\n");
		return 0;
	}
	const char *hostname = argv[1];
	int port = atoi(argv[2]);
	int sock = 0, valread;
	string msg;
	struct sockaddr_in serv_addr;
	char buffer[1024] = {0};
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

	// Convert IPv4 and IPv6 addresses from text to binary form
	if(inet_pton(AF_INET, hostname, &serv_addr.sin_addr)<=0)
	{
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\nConnection Failed \n");
		return -1;
	}
	while (true) { //loop until quit is called
		printf("myftp>");
		getline(cin, msg); //read user input
		send(sock , msg.c_str() , msg.length() , 0 ); 
		printf("Message sent\n");
		valread = read( sock , buffer, 1024);
		printf("%s\n",buffer );
	}
}
