#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <iostream>
#include <thread>
#include <fstream>
#include <mutex>

using namespace std;
int termID = -1;
mutex mtx;

void worker(string msg, int sock) {
  int valread;
  bool guard = true;
  bool notPut = true;
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
      notPut = false;
     cout << "PUTTING" << endl;
     file.open(token2, ios::in | ios::binary);
     if(file.is_open()){
       cout<<"[LOG] : File is ready to Transmit.\n";
     }
     else{
       cout<<"[ERROR] : File loading failed, Exititng.\n";
        file.open(token2, ios::in | ios::binary);
     }

     string contents((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
     if (contents.size() < 1000) {
      contents = msg + " *" + contents;
      cout << contents << endl;
      cout<<"[LOG] : Transmission Data Size "<<contents.length()<<" Bytes.\n";
      cout<<"[LOG] : Sending...\n";
      valread = read(sock, idBuffer, 100); //recieve command id
      cout << "command ID idBuffer" << idBuffer << endl;
      int bytes_sent = send(sock , contents.c_str() , contents.length() ,0);
      send(sock, "ENDOFFILE", 9, 0);
      cout<<"[LOG] : Transmitted Data Size "<<bytes_sent<<" Bytes.\n";
      cout<<"[LOG] : File Transfer Complete.\n";
     } else {
        int contentsIndex = 0;
//        string partialMsg = msg + " *" + contents.substr(contentsIndex, contentsIndex+970);
  //      contentsIndex= contentsIndex+ partialMsg.size();
    //    cout<<"[LOG] : Transmission Data Size "<< partialMsg.length()<<" Bytes.\n";
        cout<<"[LOG] : Sending...\n";
        send(sock, msg.c_str(), msg.length(), 0);
        valread = read(sock, idBuffer, 100); //recieve command id
        mtx.lock();
        cout << "command ID" << idBuffer << endl;
        mtx.unlock();
        cout << contentsIndex << "contentsIndex" << endl;
        cout << contents.size() << "contents size" << endl;
        string partialMsg = "";
        int bytes_sent = 0;
        while (termID != atoi(idBuffer) && contentsIndex + 30 < contents.size()) {
          if (contents.size() - contentsIndex > 1000) {
            partialMsg.assign(contents.substr(contentsIndex, contentsIndex+1000));
            //cout << partialMsg << "partialmsg" << endl; shows entire book
            contentsIndex = contentsIndex +1000;
            bytes_sent = send(sock, partialMsg.c_str(), partialMsg.length(), 0);
            cout<<"[LOG] : Transmission Data Size "<< partialMsg.length()<<" Bytes.\n";
          } else {
             partialMsg.assign(contents.substr(contentsIndex, contents.size()-1));
            //cout << partialMsg << "partialmsg" << endl; shows entire book
            bytes_sent = send(sock, partialMsg.c_str(), partialMsg.length(), 0);
            cout<<"[LOG] : Transmission Data "<< partialMsg.length()<<" Bytes.\n";

        }
        if (contentsIndex +30 > contents.size()) {
          send(sock, "ENDOFFILE", 9, 0);
        }
        if (termID != atoi(idBuffer)) {
          cout<<"[LOG] : File Transfer Complete.\n";
        } else { //put was terminated
          cout<<"[LOG] : File Transfer terminated.\n";
          termID = -1;
        }
     }
     }

      //while loop of 1000 bytes reading, while loop should check if termId = idBuffer, after exit delete files made

     
  } else {
    send(sock , msg.c_str() , msg.length() , 0 );
  }
  if (token.substr(0,3).compare("get")== 0 ) { //prints out put and get command ID
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

  if (guard && notPut) { //if guard is false then error occured so don't read from socket for get

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
      int index5 = 0;
      while (termID != atoi(idBuffer)) {
          valread = read( sock , buffer, 1000); //read main msg from server
          if (valread < 1) {
              cout << "Socket read error" <<endl;
          }
          string endoffile(buffer);
          index5 = endoffile.find(" ");
          string endMsg = endoffile.substr(0,index5);
          if (endMsg.compare("ENDOFFILE")) {
            break;
          }
          outfile.write(buffer, sizeof(buffer));
      }


      outfile.close();
      if (termID == atoi(idBuffer)) {
        termID = -1;
        remove(fileName.c_str());
      }
    }
    cout << endl;
    printf("%s\n",buffer );
  }
}


void terminateWorker(string msg, int term_sock) {
  int index = msg.find(" ");
  string termId = msg.substr(index +1, msg.length());
  termID = atoi(termId.c_str());
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
        msg.pop_back();
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

