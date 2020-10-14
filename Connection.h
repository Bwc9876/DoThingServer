

//BEGIN INCLUDES
//---------------------------------


#include <bits/stdc++.h>
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <string.h>

using namespace std;


//--------------------------------
//END INCLUDES


//BEGIN CLASSES SECTION
//--------------------------------
	
	
class Connection{

	private:
		int sock;
	
	public:
	
		void push(string);
		string recieve();
		string WaitUntilRecv();
		void dc();
		Connection(int, string);
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
	
		void MainLoop(void (*f)(Connection, string), string, bool);
		HostConnection(int);	
};


//-------------------------------
//END CLASSES SECTION

