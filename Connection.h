

//BEGIN INCLUDES
//---------------------------------


#include <bits/stdc++.h>
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <string.h>


//--------------------------------
//END INCLUDES


//BEGIN CLASSES SECTION
//--------------------------------
	
	
class Connection{

	private:
		int sock;
	
	public:
	
		void push(std::string);
		std::string recieve();
		std::string WaitUntilRecv();
		void dc();
		Connection(int, std::string);
		Connection(int);
		bool java = false;
		char buffer[1024] = { 0 };
		
};


class HostConnection{
	
	private:
	
		int host_socket, server_fd, valread;
		struct sockaddr_in address;
		int opt = 1;
		int addrlen = sizeof(address);
		
	public:
	
		void MainLoop(void (*f)(Connection, std::string), std::string, bool);
		HostConnection(int);	
};


//-------------------------------
//END CLASSES SECTION

