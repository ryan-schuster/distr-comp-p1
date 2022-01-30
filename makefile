all: myftp myftpserver

myftp: myftp.cpp
	g++ -Wall -Wextra -pthread -o myftp myftp.cpp 

myftpserver: myftpserver.cpp
	g++ -Wall -Wextra -pthread -o myftpserver myftpserver.cpp

clean:
	rm -f myftpserver myftp