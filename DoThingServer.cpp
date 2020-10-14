//DoThingServer
//bwc9876


//BEGIN PRE-PROCCESSORS/NAMESPACES SECTION
//------------------------------------------------


#include "DoThingServer.h"
#define PORT 8080


using namespace std;
namespace fs = std::filesystem;


//------------------------------------------------
//END PRE-PROCCESSORS/NAMESPACES SECTION


//BEGIN MISC. SECTION
//------------------------------------------------


int getIndex(vector<std::string> v, std::string K) { 
    auto it = find(v.begin(), v.end(), K); 
	return it != v.end()? distance(v.begin(), it) : -1;
} 


bool JavaCheck(std::vector<std::string> args) {
	
	std::string needed("JAVA");
		
	bool java;
		
	try{
		if (!args[4].empty()){
			std::string javastr = args[4];
			javastr.pop_back();
			java = javastr == needed;		
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
	
	return java;	
}


void InvalidMode(std::vector<std::string> args, Connection con) {
	con.push("Invalid Mode: " + args[0]);
}


void Echo(std::vector<std::string> args, Connection con) {
	con.push(args[1]);
}


//------------------------------------------------
//END MISC. SECTION


//BEGIN STRING TOOL SECTION
//------------------------------------------------


void tokenize(std::string const& str, const char delim, std::vector<std::string>& argst) {
    size_t start;
    size_t end = 0;

    while ((start = str.find_first_not_of(delim, end)) != std::string::npos)
    {
        end = str.find(delim, start);
        argst.push_back(str.substr(start, end - start));
    }
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


//------------------------------------------------
//END STRING TOOLS SECTION


//BEGIN FILE TOOLS SECTION
//------------------------------------------------


bool DirExists(std::string dirpath) {
	struct stat buffer;
	return stat (dirpath.c_str(), &buffer) == 0;
}


void TouchFile(std::string filepath) {
    fstream fsy;
    fsy.open(filepath.c_str(), ios::out);
    fsy.close();
}


bool file_exists(const std::string name) {
    ifstream f(name.c_str());
    return f.good();
}


//------------------------------------------------
//END FILE TOOLS SECTION


//BEGIN WRITE SECTION
//------------------------------------------------


void Write(std::vector<std::string> args, Connection con) {
	
    std::string path = "DoThingData/" + args[1] + "/" + args[2] + ".csv";
    if (file_exists(path) == true) {
        std::remove(path.c_str());
    }

    ofstream File(path.c_str());
    std::string look = "END";

    while (true) 
	{
        std::string data = con.WaitUntilRecv();
        if (data.find(look) != std::string::npos) 
		{
            break;
        }
        else 
		{
			if (con.java)
			{
				File << data;
			}
			else
			{
				File << data << std::endl;
			}
            con.push("GO");
        }
    }
    File.close();
}


void NewGroup(std::vector<std::string> args, Connection con) {
	
	char conf[1024] = { 0 };
	std::string path = "DoThingData/" + args[1] + "/" + args[2] + ".csv";
	con.push("Same");
	con.WaitUntilRecv();
	std::string to_put(conf);
	if (con.java == true){
		to_put.pop_back();
	}
	std::string newpath = "DoThingData/" + args[1] + "/" + to_put + ".csv";
	rename(path.c_str(), newpath.c_str());
}


void AddUser(std::string name) {
    std::string path = "DoThingData/" + name;
    fs::create_directory(path);
}


void DeleteGroup(std::vector<std::string> args, Connection con) {
	std::string path = "DoThingData/" + args[1] + "/" + args[2] + ".csv";
	if (file_exists(path))
	{
		std::remove(path.c_str());
	}
}


//------------------------------------------------
//END WRITE SECTION


//BEGIN READ SECTION
//------------------------------------------------


void GetGroups(std::vector<std::string> args, Connection con) {
	
	std::string data = "";

    std::string EndCode = "END";
	
    std::string path = "DoThingData/" + args[1] + "/";

    for (auto& p : fs::directory_iterator(path.c_str()))
    {
        std::vector<std::string> argst;

        std::string data = p.path().filename();

        std::string delimeter = ".";

        data = data.substr(0, data.find(delimeter));

        char tm[1024] = { 0 };

        con.push(data);

        con.WaitUntilRecv();
    }

    con.push(EndCode);
}


void Read(std::vector<std::string> args, Connection con) {

    std::string data = "";


    std::string EndCode = "END";
    std::string path = "DoThingData/" + args[1] + "/" + args[2] + ".csv";

    std::ifstream File(path);

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


//------------------------------------------------
//END READ SECTION


//BEGIN AUTH SECTION
//------------------------------------------------

void Proxy(std::string command, Connection con, std::string auth_ip) {
	
	Connection AuthCon(8081, auth_ip);
	
    AuthCon.push(command);

    std::string returnCode = AuthCon.WaitUntilRecv();
	
	AuthCon.dc();

	con.push(returnCode);
}


bool TestAuth(std::string ip) {
	
	Connection AuthCon(8081, ip);
	
	AuthCon.push("T/Hello");
	
	std::string result = AuthCon.WaitUntilRecv();
	
	AuthCon.dc();
	
	return result == "Hello";
}


std::string Validate(std::string name, std::string token, Connection con, std::string auth_ip) {
	
    Connection AuthCon(8081, auth_ip);
	
	AuthCon.push("V/" + name + "/" + token);
    
    std::string returnCode = AuthCon.WaitUntilRecv();
	
	AuthCon.dc();

	return returnCode;
}


void Forward(std::vector<std::string> args, Connection con, std::string auth_ip) {
	std::string proxy_command;
	for (std::string part : args){
		if (getIndex(args, part) != 0){
			proxy_command += getIndex(args, part) == 1? part : "/" + part;
		}
	}
	Proxy(proxy_command, con, auth_ip);
}


void IfValid(std::vector<std::string> args, Connection con, std::string auth_ip, void (*f)(std::vector<std::string>, Connection)) {
	
	std::string returnCode = Validate(args[1], args[3], con, auth_ip);
	
	con.push(returnCode);
	
	if (returnCode == "VT")
	{	

		if (!DirExists("DoThingData/" + args[1])){
			AddUser(args[1]);
		}
		
		con.WaitUntilRecv();
		(*f)(args, con);
	}
}


//------------------------------------------------
//END AUTH SECTION


//BEGIN MAIN SECTION
//------------------------------------------------


void ConLoop(Connection con, std::string auth_ip) {
        std::string input = con.recieve();
        const std::vector<std::string> args = split(input, '/');
        const char mode = args[0][0];
		con.java = JavaCheck(args);
		
		
		switch(mode){
			case 'R': IfValid(args, con, auth_ip, Read); break;
			case 'W': IfValid(args, con, auth_ip, Write); break;
			case 'D': IfValid(args, con, auth_ip, DeleteGroup); break;
			case 'G': IfValid(args, con, auth_ip, GetGroups); break;
			case 'N': IfValid(args, con, auth_ip, NewGroup); break;
			case 'A': Forward(args, con, auth_ip); break;
			case 'T': Echo(args, con); break;
			default : InvalidMode(args, con); break;
		}
}


int main() {
	
	if (!DirExists("DoThingData")){
		std::cout << "No User Data Directory Found, Creating One" << std::endl;
		fs::create_directory("DoThingData");
	}
	
	std::string auth_ip;
	
	if (!file_exists("Server.config")){
		//Start First Time setup
		std::cout << "No config detected, entering first time set-up" << std::endl;
		std::cout << "Enter the ip address of the auth server you wish to use" << std::endl;
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
	
	std::cout << "DoThing Server Started" << std::endl;
	
	HostConnection hostCon(8080);
	hostCon.MainLoop(ConLoop, auth_ip, true);
	
	return 0;
}


//------------------------------------------------
//END MAIN SECTION




