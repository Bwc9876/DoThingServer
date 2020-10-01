
#include "Connection.h"

using namespace std;

	void HostConnection::MainLoop( void (*f)(PartialConnection, std::string), std::string extra)
	{
		int con_sock;
		
		while ( (con_sock = accept(server_fd, (struct sockaddr*)&address,(socklen_t*)&addrlen)) ){
			
			// fcntl(con_sock, F_SETFL, O_NONBLOCK);
			
			char str[INET_ADDRSTRLEN];
			std::string client_address = std::string(inet_ntop(AF_INET, &(address.sin_addr), str, INET_ADDRSTRLEN));
			std::cout << "Connection from: " << client_address << std::endl;
			PartialConnection con(con_sock, client_address);
			(*f)(con, extra);
			close(con_sock);
			std::cout << "Connection to: " << client_address << " Closed" << std::endl;
		}
	}
	
	std::string PartialConnection::recieve(){
		int valread = read(socket, buffer, 1024);
		std::string data(buffer);
		memset(buffer, '\0', 1023);
		return data;
	}
	
	std::string PartialConnection::WaitUntilRecv(){
		std::string data;
		while (true)
		{
            data = recieve();
            if (data != "") 
			{
                break;
            }
        }
		return data;
	}
	
	void PartialConnection::push(std::string message)
	{
		std::string to_send;
		if (java == true)
		{
			to_send = message + "\r\n";
		}
		else
		{
			to_send = message;
		}
		
		int n = to_send.length();
		
		send(socket, to_send.c_str(), n, 0);
	}
	
	HostConnection::HostConnection(int port)
	{

		if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
		{
			perror("socket failed");
			exit(EXIT_FAILURE);
		}

		if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
			&opt, sizeof(opt)))
		{
			perror("setsockopt");
			exit(EXIT_FAILURE);
		}
		
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons(port);

		if (bind(server_fd, (struct sockaddr*)&address,
			sizeof(address)) < 0)
		{
			perror("bind failed");
			exit(EXIT_FAILURE);
		}
		if (listen(server_fd, 3) < 0)
		{
			perror("listen");
			exit(EXIT_FAILURE);
		}
		
	}
	
	PartialConnection::PartialConnection(int sock, std::string ip){
		socket = sock;
		client_address = ip;
	}
