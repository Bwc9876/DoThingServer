

//BEGIN PRE-PROCCESSORS/NAMESPACES SECTION
//------------------------------------------------


#include "Connection.h"


using namespace std;


//------------------------------------------------
//END PRE-PROCCESSORS/NAMESPACES SECTION


//BEGIN HOSTCONNECTION SECTION
//---------------------------------------


	void HostConnection::MainLoop( void (*f)(PartialConnection, std::string), std::string extra, bool LogConnects){
		int con_sock;	
		while ( (con_sock = accept(server_fd, (struct sockaddr*)&address,(socklen_t*)&addrlen)) ){
			char str[INET_ADDRSTRLEN];
			std::string client_address = std::string(inet_ntop(AF_INET, &(address.sin_addr), str, INET_ADDRSTRLEN));
			if (LogConnects){std::cout << "Connection from: " << client_address << std::endl;}
			PartialConnection con(con_sock, client_address);
			(*f)(con, extra);
			close(con_sock);
			if (LogConnects){std::cout << "Connection to: " << client_address << " Closed" << std::endl;}
		}
	}
	
	
	HostConnection::HostConnection(int port){
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
	
	
//---------------------------------------
//END HOSTCONNECTION SECTION


//START PARTIALCONNECTION SECTION
//---------------------------------------


	std::string PartialConnection::recieve(){
		int valread = read(sock, buffer, 1024);
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
	
	
	void PartialConnection::push(std::string message){
		std::string to_send = java? message + "\r\n": message;
		send(sock, to_send.c_str(), to_send.length(), 0);
	}
	
	
	PartialConnection::PartialConnection(int sock_in, std::string ip){
		sock = sock_in;
		client_address = ip;
	}
	

//--------------------------------------
//END PARTIALCONNECTION SECTION


//BEGIN CONNECTION SECTION
//--------------------------------------


	Connection::Connection(int port, std::string ip){
		
		struct sockaddr_in serv_addr;
		
		if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			std::cout << "Error Connecting" << std::endl;
		}

		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(port);

		if (inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr) <= 0)
		{
			std::cout << "Error Connecting" << std::endl;
		}

		if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
		{
			std::cout << "Error Connecting" << std::endl;
		}
		
	}	
	
	std::string Connection::recieve(){
		int valread = read(sock, buffer, 1024);
		std::string data(buffer);
		memset(buffer, '\0', 1023);
		return data;
	}
	
	
	std::string Connection::WaitUntilRecv(){
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
	
	
	void Connection::push(std::string message){
		std::string to_send = java? message + "\r\n": message;
		send(sock, to_send.c_str(), to_send.length(), 0);
	}
	
	void Connection::dc(){
		close(sock);
	}
	
	
//----------------------------------
//END CONNECTION SECTION
