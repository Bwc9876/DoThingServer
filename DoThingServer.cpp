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

void Write(std::string name, std::string group, PartialConnection con) {
	
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
        std::string data = con.WaitUntilRecv();
        if (data.find(look) != std::string::npos) 
		{
            break;
        }
        else 
		{
			std::string to_put(wl);
			if (con.java)
			{
				File << data;
				std::cout << data;
			}
			else
			{
				File << data << std::endl;
				std::cout << data << std::endl;
			}
            con.push("GO");
        }
    }
    File.close();
}

void tokenize(std::string const& str, const char delim,
    std::vector<std::string>& argst)
{
    size_t start;
    size_t end = 0;

    while ((start = str.find_first_not_of(delim, end)) != std::string::npos)
    {
        end = str.find(delim, start);
        argst.push_back(str.substr(start, end - start));
    }
}

void GetGroups(std::string name, PartialConnection con) {
	
	std::string data = "";

    std::string EndCode = "END";
	
    std::string fin = "DoThingData/" + name + "/";

    for (auto& p : fs::directory_iterator(fin.c_str()))
    {
        std::vector<std::string> argst;

        std::string data = p.path().filename();

        std::string delimeter = ".";

        data = data.substr(0, data.find(delimeter));
		
		std::cout << data << std::endl;

        char tm[1024] = { 0 };

        con.push(data);

        con.WaitUntilRecv();
    }

    con.push(EndCode);
}

void Proxy(std::string command, PartialConnection con, std::string auth_ip){
	
	int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = { 0 };
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        con.push("IE");
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8081);

    // Convert IPv4 and IPv6 addresses from text to binary form 
    if (inet_pton(AF_INET, auth_ip.c_str(), &serv_addr.sin_addr) <= 0)
    {
        con.push("IE");
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        con.push("IE");
    }
	
    std::string fin = command;
    send(sock, fin.c_str(), strlen(fin.c_str()), 0);
    char tm[1024] = { 0 };
    valread = read(sock, tm, 1024);
    
    std::string returnCode(tm);

    close(sock);

	con.push(returnCode);
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
	
	std::string result(tm);
	
	if (result == "Hello"){
		return true;
	}
	
	return false;
}

std::string Validate(std::string name, std::string token, PartialConnection con, std::string auth_ip) {
	
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = { 0 };
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        return "IE";
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8081);

    // Convert IPv4 and IPv6 addresses from text to binary form 
    if (inet_pton(AF_INET, auth_ip.c_str(), &serv_addr.sin_addr) <= 0)
    {
        return "IE";
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        return "IE";
    }
	
    std::string fin = "V/" + name + "/" + token;
    send(sock, fin.c_str(), strlen(fin.c_str()), 0);
    char tm[1024] = { 0 };
    valread = read(sock, tm, 1024);
    
    std::string returnCode(tm);

    close(sock);

	return returnCode;
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

void Read(std::string name, std::string group, PartialConnection con) {

    std::string data = "";


    std::string EndCode = "END";
    std::string fin = "DoThingData/" + name + "/" + group + ".csv";

    std::ifstream File(fin);

    if (!File) {
        con.push("END");
        return;
    }

    while (getline(File, data))
    {
        con.push(data);
        con.WaitUntilRecv();
    }

    File.close();

    con.push("END");

}

std::vector<std::string> split(std::string in_str, char delimiter) {

	std::vector<std::string> out;

    std::string between;
	
	std::stringstream check1(in_str);
	
	while(getline(check1, between, delimiter)){
		out.push_back(between);
	}
	
	return out;

}


bool DirExists(std::string dirpath){
	struct stat buffer;
	return (stat (dirpath.c_str(), &buffer) == 0);
}

int getIndex(vector<std::string> v, std::string K) { 
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

void DeleteGroup(std::string username, std::string group){
	std::string fin = "DoThingData/" + username + "/" + group + ".csv";
	if (file_exists(fin) == true)
	{
		std::remove(fin.c_str());
	}
}

bool JavaCheck(std::vector<std::string> args){
	
	std::string needed("JAVA");
		
	bool java;
		
	try{
		if (!args[4].empty()){
				
			std::string javastr = args[4];
			javastr.pop_back();
			if (javastr == needed){
				java = true;
				std::cout << "Java Is True" << std::endl;
			}
			else{
				std::cout << "Java Is False, not equal" << std::endl;
				java = false;
			}
				
		}
		else{
			java = false;
			std::cout << "Java Is False, empty" << std::endl;
		}
	}
	catch(const std::out_of_range& oor){
		java = false;
		std::cout << "Java Is False, oor" << std::endl;
	}
	catch(const std::logic_error& le){
		java = false;
		std::cout << "Java Is False, le" << std::endl;
	}
	catch(const std::bad_alloc& ba){
		java = false;
		std::cout << "Java Is False, ba" << std::endl;
	}
	
	return java;	
}

void NewGroup(std::string username, std::string group, PartialConnection con){
	
	char conf[1024] = { 0 };
	std::string fin = "DoThingData/" + username + "/" + group + ".csv";
	con.push("Same");
	con.WaitUntilRecv();
	std::string to_put(conf);
	if (con.java == true){
		to_put.pop_back();
	}
	std::string newstr = "DoThingData/" + username + "/" + to_put + ".csv";
	rename(fin.c_str(), newstr.c_str());
}

void InvalidMode(PartialConnection con){
	con.push("Invalid Mode!");
}

void ConLoop(PartialConnection con, std::string extra_string)
{
	
        std::string input = con.recieve();

		std::cout << "AH" << std::endl;

        const std::vector<std::string> args = split(input, '/');
		
		std::cout << input << std::endl;

        const char mode = args[0][0];
		
		con.java = JavaCheck(args);
		
		std::cout << con.java << std::endl;
		
		if (mode == 'A'){
			//FORWARD TO AUTH SERVER
			std::string proxy_command;
			for (std::string part : args){
				if (getIndex(args, part) != 0){
					if (getIndex(args, part) == 1){
						proxy_command += part;
					}
					else {
						proxy_command += "/" + part;
					}
				}
			}
			Proxy(proxy_command, con, extra_string);
			return;
		}
		else if (mode == 'T'){
			std::string response = "Hello";
			std::cout << response << std::endl;
			con.push(response);
			return;
		}
		

        std::string code = Validate(args[1], args[3], con, extra_string);
		
		con.push(code);

        if (code != "VT"){return;}

        con.WaitUntilRecv();

		if (!DirExists("DoThingData/" + args[1])){
			AddUser(args[1]);
		}

		switch(mode){
			case 'R': Read(args[1], args[2], con); break;
			case 'W': Write(args[1], args[2], con); break;
			case 'D': DeleteGroup(args[1], args[2]); break;
			case 'G': GetGroups(args[1], con); break;
			case 'N': NewGroup(args[1], args[2], con); break;
			default : InvalidMode(con); break;
		}
		
}


int main(){
	
	if (!DirExists("DoThingData")){
		std::cout << "No User Data Directory Fargsnd, Creating One" << std::endl;
		fs::create_directory("DoThingData");
	}
	
	std::string auth_ip;
	
	if (!file_exists("Server.config")){
		//Start First Time setup
		std::cout << "No config detected, entering first time set-up" << std::endl;
		std::cout << "First, enter the ip address of the auth server yargs wish to use" << std::endl;
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
			std::cout << "Error, cant read config file. Delete it if yargs wish to go thrargsgh setup again" << std::endl;
			return 0;
		}
		auth_ip = lines[0];
		if (!TestAuth(auth_ip)){
			std::cout << "Error, the auth server did not respond on start (start the auth server first!)";
			return 0;
		}
	}
	
	HostConnection hostCon(8080);
	hostCon.MainLoop(ConLoop, auth_ip);
	
	return 0;
}
