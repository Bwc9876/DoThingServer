#include <bits/stdc++.h>
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <string.h>


class PartialConnection
{
	private:
		int socket;
	public:
		std::string client_address;
		void push(std::string);
		std::string recieve();
		std::string WaitUntilRecv();
		PartialConnection(int, std::string);
		bool java;
		char buffer[1024] = { 0 };
};

class HostConnection
{
	private:
		int connection_socket, host_socket, server_fd, valread;
		struct sockaddr_in address;
		int opt = 1;
		int addrlen = sizeof(address);
	public:
		void MainLoop(void (*f)(PartialConnection, std::string), std::string);
		HostConnection(int);
};