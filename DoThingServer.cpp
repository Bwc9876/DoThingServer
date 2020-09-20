// DoThingServer.cpp : Defines the entry point for the application.
//

#include "DoThingServer.h"
#define PORT 8080

using namespace std;
namespace fs = std::filesystem;
bool file_exists(const std::string name) {
    ifstream f(name.c_str());
    return f.good();
}

std::string conchartostring(char g[1024]) {
    std::string out;
    std::stringstream no;
    no << g;
    no >> out;
    return out;
}

void Send_Data(int socket, std::string message, bool java){
	std::string to_send;
	if (java == true){
		to_send = message + "\r\n";
	}
	else{
		to_send = message;
	}
	
	int n = to_send.length();
	
	send(socket, to_send.c_str(), n, 0);
}

void Write(std::string name, std::string group, int valread, int new_socket, bool java) {
    std::string fin = "DoThingData/" + name + "/" + group + ".csv";
    if (file_exists(fin) == true) {
        std::remove(fin.c_str());
    }

    ofstream File(fin.c_str());
    std::string look = "END";
    char wl[1024] = { 0 };
    char empt[1024] = { 0 };
    memset(empt, ' ', 1023);
    empt[1024] = '\0';

    while (true) 
	{
        valread = read(new_socket, wl, 1024);
        if (wl != "" && wl != empt) 
		{
            if (conchartostring(wl).find(look) != std::string::npos) 
			{
                break;
            }
            else 
			{
				std::string to_put(wl);
				if (java == true)
				{
					size_t pos;
					while ((pos = to_put.find("_")) != std::string::npos)
					{
						to_put.replace(pos, 1, " ");
					}
				}
                File << to_put << std::endl;
				std::cout << to_put << std::endl;
                Send_Data(new_socket, "GO", java);
                memset(wl, 0, 255);
            }
        }
    }
    File.close();
}

void tokenize(std::string const& str, const char delim,
    std::vector<std::string>& out)
{
    size_t start;
    size_t end = 0;

    while ((start = str.find_first_not_of(delim, end)) != std::string::npos)
    {
        end = str.find(delim, start);
        out.push_back(str.substr(start, end - start));
    }
}

void GetGroups(std::string name, int sockfd, bool java) {

    std::string data = "";


    std::string EndCode = "END";
	
    std::string fin = "DoThingData/" + name + "/";

    for (auto& p : fs::directory_iterator(fin.c_str()))
    {
        std::vector<std::string> out;

        int valread;

        std::string data = p.path().filename();

        std::string delimeter = ".";

        data = data.substr(0, data.find(delimeter));
		
		std::cout << data << std::endl;

        char tm[1024] = { 0 };

        Send_Data(sockfd, data, java);

        while (true) {
            valread = read(sockfd, tm, 1024);
            if (tm != "") {
                memset(tm, ' ', 1023);
                tm[1024] = '\0';
                break;
            }
        }


    }

    Send_Data(sockfd, EndCode, java);
}

void Proxy(std::string command, int sockfd, bool java, std::string auth_ip){
	
	int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = { 0 };
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        Send_Data(sockfd, "IE", java);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8081);

    // Convert IPv4 and IPv6 addresses from text to binary form 
    if (inet_pton(AF_INET, auth_ip.c_str(), &serv_addr.sin_addr) <= 0)
    {
        Send_Data(sockfd, "IE", java);
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        Send_Data(sockfd, "IE", java);
    }
	
    std::string fin = command;
    send(sock, fin.c_str(), strlen(fin.c_str()), 0);
    char tm[1024] = { 0 };
    valread = read(sock, tm, 1024);
    
    std::string returnCode(tm);

    close(sock);

	Send_Data(sockfd, returnCode, java);
}

bool TestAuth(std::string ip){
	
	int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = { 0 };
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        return false;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8081);

    // Convert IPv4 and IPv6 addresses from text to binary form 
    if (inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr) <= 0)
    {
        return false;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        return false;
    }
	
	send(sock, "T/Hello", 8, 0);
	char tm[1024] = { 0 };
    valread = read(sock, tm, 1024);
	
	close(sock);
	
	std::string result = conchartostring(tm);
	
	if (result == "Hello"){
		return true;
	}
	
	return false;
}

std::string Validate(std::string name, std::string token, int sockfd, bool java, std::string auth_ip) {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = { 0 };
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        Send_Data(sockfd, "IE", java);
        return "IE";
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8081);

    // Convert IPv4 and IPv6 addresses from text to binary form 
    if (inet_pton(AF_INET, auth_ip.c_str(), &serv_addr.sin_addr) <= 0)
    {
        Send_Data(sockfd, "IE", java);
        return "IE";
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        Send_Data(sockfd, "IE", java);
        return "IE";
    }
	
    std::string fin = "V/" + name + "/" + token;
    send(sock, fin.c_str(), strlen(fin.c_str()), 0);
    char tm[1024] = { 0 };
    valread = read(sock, tm, 1024);
    
    std::string returnCode = conchartostring(tm);

    close(sock);

    if (returnCode == "IU") {
        Send_Data(sockfd, "IU", java);
        return "IU";
    }
    else if (returnCode == "IT") {
        Send_Data(sockfd, "IT", java);
        return "IT";
    }
    else if (returnCode == "VT") {
        Send_Data(sockfd, "VT", java);
        return "VT";
    }

	Send_Data(sockfd, "IE", java);
    return "IE";
}

void AddUser(std::string name) {
    std::string fin = "DoThingData/" + name;
    fs::create_directory(fin);
}

void TouchFile(std::string filepath) {
    fstream fsy;
    fsy.open(filepath.c_str(), ios::out);
    fsy.close();
}

void Read(std::string name, std::string group, int valread, int new_socket, bool java) {

    std::string data = "";


    std::string EndCode = "END";
    std::string fin = "DoThingData/" + name + "/" + group + ".csv";

    std::ifstream File(fin);

    if (!File) {
        Send_Data(new_socket, EndCode, java);
        return;
    }

    while (getline(File, data))
    {

        char tm[1024] = { 0 };
        Send_Data(new_socket, data, java);

        while (true) {
            valread = read(new_socket, tm, 1024);
            if (tm != "") {
                memset(tm, ' ', 1023);
                tm[1024] = '\0';
                break;
            }
        }


    }

    File.close();

    Send_Data(new_socket, EndCode, java);

}

std::vector<std::string> split(char p[1024], char s) {

    std::stringstream com;

    std::string seg;

    com << p;

    std::vector<std::string> seglist;

    while (std::getline(com, seg, s)) {
        seglist.push_back(seg);
    }

    return seglist;

}


bool DirExists(std::string dirpath){
	struct stat buffer;
	return (stat (dirpath.c_str(), &buffer) == 0);
}

int getIndex(vector<std::string> v, std::string K) 
{ 
    auto it = find(v.begin(), v.end(), K); 
    if (it != v.end()) { 
        int index = distance(v.begin(), it); 
        return index;
    } 
    else { 
        return -1;
    } 
	
	return -1;
} 


int main()
{
	
	if (!DirExists("DoThingData")){
		std::cout << "No User Data Directory Found, Creating One" << std::endl;
		fs::create_directory("DoThingData");
	}
	
	std::string auth_ip;
	
	if (!file_exists("Server.config")){
		//Start First Time setup
		std::cout << "No config detected, entering first time set-up" << std::endl;
		std::cout << "First, enter the ip address of the auth server you wish to use" << std::endl;
		std::cin >> auth_ip;
		std::cout << "Contacting Auth Server..." << std::endl;
		bool valid = TestAuth(auth_ip);
		if (valid){
			std::cout << "Succesfully validated with auth server" << std::endl;
			std::cout << "Saving Server Setup Data In Server.config" << std::endl;
			//Save Data
			ofstream File("Server.config");
			File << auth_ip << std::endl;
			File.close();
			std::cout << "Save Complete, starting the server" << std::endl;
		}
		else{
			std::cout << "Error, Auth Server didnt respond correctly" << std::endl;
			return 0;
		}
		
	}
	else {
		std::string data = "";
		std::ifstream File("Server.config");
		std::vector<std::string> lines;
		while (getline(File, data))
		{
			if (data != "")
			{
				lines.push_back(data);
			}
		}
		if (lines.size() != 1)
		{
			std::cout << "Error, cant read config file. Delete it if you wish to go through setup again" << std::endl;
			return 0;
		}
		auth_ip = lines[0];
		if (!TestAuth(auth_ip)){
			std::cout << "Error, the auth server did not respond on start (start the auth server first!)";
			return 0;
		}
	}
	
	
	std::cout << "DoThing Server started" << std::endl;


    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char conf[1024] = { 0 };
    char buffer[1024] = { 0 };

    // Creating socket file descriptor 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080 
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
        &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080 
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
    while ( (new_socket = accept(server_fd, (struct sockaddr*)&address,(socklen_t*)&addrlen)) )
    {
        std::cout << "Connection Opened" << std::endl;

        valread = read(new_socket, buffer, 1024);

        std::vector<std::string> ou = split(buffer, '/');

        std::string mode = ou[0];
		
		std::string needed("JAVA");
		
		std::cout << "BEFORE JAVA" << std::endl;
		
		bool java;
		
		try{
			if (!ou[4].empty()){
				
				std::string javastr = ou[4];
				javastr.pop_back();
				if (javastr == needed){
					java = true;
				}
				else{
					java = false;
				}
				
			}
			else{
				java = false;
			}
		}
		catch(const std::out_of_range& oor){
			java = false;
		}
		catch(const std::logic_error& le){
			java = false;
		}
		catch(const std::bad_alloc& ba){
			java = false;
		}
		
		std::cout << "BEFORE TEST" << std::endl;
		
		
		if (mode[0] == 'A'){
			//FORWARD TO AUTH SERVER
			std::string proxy_command;
			for (std::string part : ou){
				if (getIndex(ou, part) != 0){
					if (getIndex(ou, part) == 1){
						proxy_command += part;
					}
					else {
						proxy_command += "/" + part;
					}
				}
			}
			Proxy(proxy_command, new_socket, java, auth_ip);
			close(new_socket);
			std::cout << "Connection Closed" << std::endl;
			ou.clear();
			memset(buffer, '\0', 1023);
			continue;
		}
		else if (mode[0] == 'T'){
			std::string response = "Hello";
			std::cout << response << std::endl;
			send(new_socket, response.c_str(), strlen(response.c_str()), 0 );
			std::cout << "Connection Closed" << std::endl;
			memset(buffer, '\0', 1023);
			ou.clear();
			close(new_socket);
			continue;
		}
		

        std::string code = Validate(ou[1], ou[3], new_socket, java, auth_ip);

        if (code != "VT"){
            close(new_socket);
            continue;
        }

        while (true) {
            valread = read(new_socket, conf, 1024);
            if (conf != "") {
                memset(conf, '\0', 1023);
                break;
            }
        }

		if (!DirExists("DoThingData/" + ou[1])){
			AddUser(ou[1]);
		}

        if (mode[0] == 'R') {
            Read(ou[1], ou[2], valread, new_socket, java);
        }
        else if (mode[0] == 'W') {
            Write(ou[1], ou[2], valread, new_socket, java);
        }
        else if (mode[0] == 'D') {
            std::string fin = "DoThingData/" + ou[1] + "/" + ou[2] + ".csv";
            if (file_exists(fin) == true) {
                std::remove(fin.c_str());
            }
        }
        else if (mode[0] == 'G') {
            GetGroups(ou[1], new_socket, java);
        }
        else if(mode[0] == 'N') {
            std::string fin = "DoThingData/" + ou[1] + "/" + ou[2] + ".csv";
            Send_Data(new_socket, "Same", java);
            while (true) {
                valread = read(new_socket, conf, 1024);
                if (conf != "") {
                    break;
                }
            }
			std::string to_put = string(conf);
			if (java == true){
				to_put.pop_back();
			}
            std::string newstr = "DoThingData/" + ou[1] + "/" + to_put + ".csv";
            int result = rename(fin.c_str(), newstr.c_str());
        }
        else{
            Send_Data(new_socket, "Invalid mode!", java);
			std::cout << "Connection Closed" << std::endl;
			memset(buffer, '\0', 1023);
			close(new_socket);
            continue;
        }

        memset(buffer, '\0', 1023);

        close(new_socket);
		
		ou.clear();

        std::cout << "Connection Closed" << std::endl;
    }
	
	return 0;
}
