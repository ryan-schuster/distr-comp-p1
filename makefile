all: myftp myftpserver

myftp: myftp.cpp
	g++ -Wall -Wextra -std=c++11 -pthread -o myftp myftp.cpp 

myftpserver: myftpserver.cpp
	g++ -Wall -Wextra -std=c++11 -pthread -o myftpserver myftpserver.cpp

clean:
	rm -f myftpserver myftp