// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <iostream>
#include <thread>
#include <fstream>

using namespace std;

void worker(string msg, int sock) {
	int valread;
	char buffer[1024] = {0};
	string token = msg.substr(0, 4);
        string buf(buffer); //constructor casts char[] into string              
        int index = buf.find(" "); //index where first space is                 
        int index2 = buf.find(" ", index + 1);
        cout << token;
        if (token.compare("quit") == 0) {
          cout << "GOODBYE";
          exit(EXIT_SUCCESS);
        }
        send(sock , msg.c_str() , msg.length() , 0 );
        valread = read( sock , buffer, 1024);
        if (token.substr(0,3).compare("get")== 0) {
          string fileName = buf.substr(index + 1, index2 - index);
          ofstream outfile(fileName);
          outfile << buffer << endl;
        }
	//printf("%s\n",buffer );
	cout << buffer << endl; //print error or msg that needs to be printed
}

int main(int argc, char const *argv[])
{
	if (argc < 3) { //checks if less than three arguments
		printf("Usage: myftp <machine name> <port>\n");
		return 0;
	}
	const char *hostname = argv[1];
	int port = atoi(argv[2]);
	int sock = 0;
	string msg;
	struct sockaddr_in serv_addr;
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
	thread tid[50];
	int i = 0;
	while (true) { //loop until quit is called
		printf("myftp>");
		getline(cin, msg); //read user input
		tid[i] = thread(worker, msg, sock);
		i++;
		if (i >= 50) {
			for (i = 0; i < 50; i++) {
				tid[i].join();
			}
			i = 0;
		}
	}
}
