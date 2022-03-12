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
  bool guard = true;
  char buffer[1024] = {0};
  char idBuffer[100] = {0}; //initizales all to null
  string token = msg.substr(0, 4);
  string buf(msg); //constructor casts char[] into string                                                                                   
  int index = buf.find(" "); //index where first space is                                                                                   
  int index2 = buf.find(" ", index + 1);
  string token2 = buf.substr(index + 1, index2 - index);
  fstream file;

  //cout << token;
  if (token.compare("quit") == 0) {
    send(sock , msg.c_str() , msg.length() , 0 );
    cout << "\nGOODBYE" << endl;
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
  if (token.substr(0,3).compare("get")== 0 || token.substr(0,3).compare("put")== 0) { //prints out put and get command ID
    valread = read(sock, idBuffer, 100); //server should send a msg if file request exists or not
    if (valread < 1) {
      cout << "Socket read error" <<endl;
    }
    cout << idBuffer << endl; //says if there is error or no
    string existmsg(idBuffer);
    index = existmsg.find(" ");
    string errorOrExists = existmsg.substr(0,index);
    if (errorOrExists.compare("[ERROR]") == 0) {
      guard = false;
    } else {
      valread = read(sock, idBuffer, 100);
      if (valread < 1) {
        cout << "Socket read error" <<endl;
      }
      cout << " = Command ID " << endl;
      cout << idBuffer;
    }
  }

  if (guard) { //if guard is false then error occured so don't read from socket for get

    valread = read( sock , buffer, 1024); //read main msg from server
    if (valread < 1) {
        cout << "Socket read error" <<endl;
    }

    if (token.substr(0,3).compare("get")== 0) {
      cout << "HERE" << endl;
      string fileName = buf.substr(index, index2 - index);
      cout << "File name: " << fileName << endl;
      /*FILE* file = fopen(fileName.c_str(), "w");
      if (file != NULL) {
        fwrite(buffer, sizeof(buffer), 1, file);
      }
      fclose(file);*/
      //also change so a while loop keeps reading 1000 bytes at a time, the while loop exits when the last msg is something like "end of file"
      ofstream outfile(fileName.c_str()); //Code doesn't work???? maybe the buffer recieved is corrupted
      outfile.write(buffer, sizeof(buffer));
      outfile.close();
    }
    cout << endl;
    printf("%s\n",buffer );
  }
}


void terminateWorker(string msg, int term_sock) {
  int index = msg.find(" ");
  string termId = msg.substr(index +1, msg.length());
  send(term_sock , termId.c_str() , termId.length() , 0 );
}



int main(int argc, char const *argv[])
{
	if (argc < 4) { //checks if less than three arguments
		printf("Usage: myftp <machine name> <nport> <tport>\n");
		return 0;
	}
	const char *hostname = argv[1];
	int nport = atoi(argv[2]);
  int tport = atoi(argv[3]);
	int sock = 0;
  int term_sock =0;
	string msg;
	struct sockaddr_in serv_addr;
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(nport);

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

	struct sockaddr_in term_serv_addr;
	if ((term_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) //terminator socket
	{
		printf("\n Socket creation error \n");
		return -1;
	}

	term_serv_addr.sin_family = AF_INET;
	term_serv_addr.sin_port = htons(tport);

	// Convert IPv4 and IPv6 addresses from text to binary form
	if(inet_pton(AF_INET, hostname, &term_serv_addr.sin_addr)<=0)
	{
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}

	if (connect(term_sock, (struct sockaddr *)&term_serv_addr, sizeof(term_serv_addr)) < 0)
	{
		printf("\nConnection Failed \n");
		return -1;
	}


	thread tid[50];
	int i = 0;
  string firstword;
	while (true) { //loop until quit is called
		printf("myftp>");
		getline(cin, msg); //read user input
    firstword = msg.substr(0, msg.find(" "));
    if (firstword.compare("terminate") == 0) {
      tid[i] = thread(terminateWorker, msg, term_sock); 
    } else {
      if (msg.back() == '&') { //run command in seperate thread
        tid[i] = thread(worker, msg, sock);
        i++;
        if (i >= 50) {
          for (i = 0; i < 50; i++) {
            tid[i].join();
          }
          i = 0;
        }
      } else {
        worker(msg, sock);
      }
    }
	}
}

