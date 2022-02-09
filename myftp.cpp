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
  string buf(msg); //constructor casts char[] into string                                                                                   
  int index = buf.find(" "); //index where first space is                                                                                   
  int index2 = buf.find(" ", index + 1);
  string token2 = buf.substr(index + 1, index2 - index);
  fstream file;

  cout << token;
  if (token.compare("quit") == 0) {
    send(sock , msg.c_str() , msg.length() , 0 );
    cout << "GOODBYE" << endl;
    exit(EXIT_SUCCESS);
  } else if(token.substr(0,3).compare("put")==0) {
     cout << "PUTTING" << endl;
     file.open(token2, ios::in | ios::binary);
     if(file.is_open()){
       cout<<"[LOG] : File is ready to Transmit.\n";
     }
     else{
       cout<<"[ERROR] : File loading failed, Exititng.\n";
       exit(EXIT_FAILURE);
     }

     string contents((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
     contents = msg + " *" + contents;
     cout << contents << endl;
     cout<<"[LOG] : Transmission Data Size "<<contents.length()<<" Bytes.\n";
     cout<<"[LOG] : Sending...\n";
     int bytes_sent = send(sock , contents.c_str() , contents.length() ,0);
     cout<<"[LOG] : Transmitted Data Size "<<bytes_sent<<" Bytes.\n";
     cout<<"[LOG] : File Transfer Complete.\n";
  } else {
    send(sock , msg.c_str() , msg.length() , 0 );
  }
  valread = read( sock , buffer, 1024);
  if (token.substr(0,3).compare("get")== 0) {
    cout << "HERE" << endl;
    string fileName = buf.substr(index + 1, index2 - index);
    cout << "File name: " << fileName << endl;
    ofstream outfile(fileName.c_str());
    outfile.write(buffer, sizeof(buffer));
    outfile.close();
  }
  printf("%s\n",buffer );
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
