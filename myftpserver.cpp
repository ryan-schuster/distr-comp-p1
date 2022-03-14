#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <thread>
#include <sstream>
#include <iostream>
#include <sys/wait.h>
#include <dirent.h>
#include <fstream>
#include <mutex>

using namespace std;
bool guard = true;
mutex mtx;
int idCounter = 0;
int termId = -1; //id to be terminated

void socketThread(int newSocket) {
	char buffer[1024] = {0};
    int valread = read(newSocket , buffer, 1024);
    if (valread < 1) {
      cout << "Socket read error" <<endl;
    }
    string buf(buffer); //constructor casts char[] into string
    int index = buf.find(" "); //index where first space is
    int index2 = buf.find(" ", index + 1);
    string token = buf.substr(0, index); //first word
    string token2 = buf.substr(index + 1, index2 - index); //second word
    const char* arg = token.c_str(); //first word but in char* format
    const char * arg2 = token2.c_str(); //second word but in char* format
    int pid;
    int status;
    fstream file;

    char * args[3]; //used for execvp
    args[0] = const_cast<char*>(token.c_str());
    args[1] = const_cast<char*>(token2.c_str());
    args[2] = NULL;

    //mutex lock to make sure not two threads get same id
    mtx.lock();
    int id = idCounter; //critical section
    idCounter++;
    mtx.unlock();
    string cmdId = to_string(id);

	if (token.compare("get") == 0) {
      file.open(token2, ios::in | ios::binary);
      if(file.is_open()){
        cout<<"[LOG] : File is ready to Transmit.\n";
      }
      else{
	cout<<"[ERROR] : File loading failed, Exititng.\n";
        exit(EXIT_FAILURE);
      }
      string contents((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
      cout<<"[LOG] : Transmission Data Size "<<contents.length()<<" Bytes.\n";
      cout<<"[LOG] : Sending...\n";
      string exists = "File exists\n";
      send(newSocket, exists.c_str(), exists.length(),0);
      send(newSocket, cmdId.c_str(), cmdId.length(), 0);
      int bytes_sent = send(newSocket , contents.c_str() , contents.length() ,0);
      cout<<"[LOG] : Transmitted Data Size "<<bytes_sent<<" Bytes.\n";
      cout<<"[LOG] : File Transfer Complete.\n";
        } else {
            cout<<"[ERROR] : File does not exist\n";
            string error = "[ERROR] : File does not exist\n";
            send(newSocket, error.c_str(), error.length(), 0);
        }
                
    } else if (token.compare("put") == 0) {
        int index3 = buf.find("*");
        string token3 = buf.substr(index3+1);
        cout << token3 << endl;
        //char buffer2[1024] = {0};
        printf("Put received\n");
        cout << "HERE" << endl;
        string fileName = token2;
        cout << "File name: " << fileName << endl;
        //same deal as get, check every 1000 bytes if id = termId and if so stop reading and cleanup created files
        //need a way for the client to know to stop sending bytes
        ofstream outfile(fileName.c_str());
        outfile.write(token3.c_str(), sizeof(token3));
        outfile.close();
    } else if (token.compare("delete") == 0) {
        string stError = "An error occured while trying to delete this file";
		if(remove(token2.c_str())!=0){
            send(newSocket, stError.c_str(), stError.length(), 0);
        }

    } else if (token.compare("ls") == 0) {
        struct dirent *dr;
    	string s = "";
        DIR *directory = opendir(".");
        for(dr=readdir(directory); dr!=NULL; dr=readdir(directory)){
           s = s + ", " + dr->d_name;
        }
        send(newSocket, s.c_str(), s.length(), 0);
        closedir(directory);
    } else if (token.compare("cd") == 0) {
        if (chdir(arg2) != 0) { //changes into directory or the if statments triggers
            string cdError = token + ": " + token2 + ": No such file or directory";
            send(newSocket, cdError.c_str(), cdError.length(), 0);
        }
    } else if (token.compare("mkdir") == 0) {
        pid = fork();
        if (pid == 0) { //forks and executes command
            execvp(arg, args); 
            exit(0);
        }
        waitpid(pid, &status, 0);
        if (status != 0) { //if execvp fails
            string mkdirError = token + ": cannot create directory " + token2  + ": File exists";
            send(newSocket, mkdirError.c_str(), mkdirError.length(), 0);
        }
    } else if (token.compare("pwd") == 0) {
        char st[256];
        string cwd = getcwd(st, 256);
        send(newSocket, cwd.c_str(), cwd.length(), 0);

    } else if (token.compare("quit") == 0) {
        guard = false;
        close(newSocket);
    }
}


void clientThread(int newSocket) {
    guard = true;
    int i = 0;
    thread tid[10];
    while (guard) { //continue to accept requests from a connection
        tid[i] = thread(socketThread, newSocket); //one client able to do multiple cmds
        i++;
        if (i >= 10) {
            for (i=0; i<10; i++) {
                tid[i].join();
            }
            i = 0;
        }
    }
}
 

void listenerThread(int nport) {

	int serverFd, newSocket;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);


    // Creating socket file descriptor
    if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    // optional code that prevents errors such as 'address already in use'
    // lets socket be bound to an address already in use.
    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( nport );
    
    // Forcefully attaching socket to the port 8080
    if (bind(serverFd, (struct sockaddr *)&address,
                                sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(serverFd, 50) < 0) //allows for 3 pending connections. More than 3? 
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    thread ctid[10]; //allow ten client threads to connect at a time
    int i = 0;
    while (true) { //run through for every quit command

        if ((newSocket = accept(serverFd, (struct sockaddr *)&address,
                        (socklen_t*)&addrlen))<0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        ctid[i] = thread(clientThread, newSocket); //multiple client threading code
        i++;
        if (i >= 10) {
            for (i=0; i<10; i++) {
                ctid[i].join();
            }
            i = 0;
        }
    }
}

void terminatorThread(int tport) {
	int serverFd, newSocket;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
    // Creating socket file descriptor
    if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    // optional code that prevents errors such as 'address already in use'
    // lets socket be bound to an address already in use.
    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( tport );
    
    // Forcefully attaching socket to the port 8080
    if (bind(serverFd, (struct sockaddr *)&address,
                                sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(serverFd, 50) < 0)  
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    while (true) {

        if ((newSocket = accept(serverFd, (struct sockaddr *)&address,
                        (socklen_t*)&addrlen))<0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        char buffer[1024] = {0};
        int valread = read(newSocket , buffer, 1024);
        if (valread < 1) {
            cout << "Socket read error" <<endl;
        }
        string buf(buffer); //constructor casts char[] into string
        int index = buf.find(" "); //index where first space is
        string token = buf.substr(0, index); //first word, which should be term id
        termId = stoi(token); //set string to int and then set to termId
        //terminate process id which should be token and then go back to top of while loop to wait for next connection
       
          
    }
}

int main(int argc, char const *argv[]) {
    if (argc < 3) { //checks if there are at least two arguments
        printf("usage: myftpserver <nport> <tport>\n");
        return 0;
    }
    int nport = atoi(argv[1]);
    int tport = atoi(argv[2]);
    thread listener = thread(listenerThread, nport);
    thread terminator = thread(terminatorThread, tport);
    listener.join();
    terminator.join();


}
