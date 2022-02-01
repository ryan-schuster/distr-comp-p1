#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <thread>

using namespace std;

void socketThread(int newSocket) {
	char buffer[1024] = {0};
	char *hello = "Hello from server";
    int valread = read(newSocket , buffer, 1024);
    printf("%s\n",buffer );
    send(newSocket , hello , strlen(hello) , 0 );
    printf("Hello message sent\n");

}


int main(int argc, char const *argv[])
{
    if (argc < 2) { //checks if there are at least two arguments
        printf("usage: myftpserver <port>\n");
        return 0;
    }
    int port = atoi(argv[1]);
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
    address.sin_port = htons( port );
    
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
    thread tid[50];
    int i = 0;
    while (true) { //run through for every quit command

        if ((newSocket = accept(serverFd, (struct sockaddr *)&address,
                        (socklen_t*)&addrlen))<0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        while (true) { //continue to accept requests from a connection
            tid[i] = thread(socketThread, newSocket); //new thread for every request, calls socketThread function
            /*    perror("failed to make thread"); //error checking code to implement later
                exit(EXIT_FAILURE);
            }*/
            i++;
            if (i >= 50) {
                for (i = 0; i < 50; i++) {
                    tid[i].join(); //waits for thread to finish
                }
                i = 0;
            }
        
        }
          
    }
}
