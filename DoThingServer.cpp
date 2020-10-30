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


int getIndex(vector<string> v, string K) { 
    auto it = find(v.begin(), v.end(), K); 
	return it != v.end()? distance(v.begin(), it) : -1;
} 


bool JavaCheck(vector<string> args) {
	
	string needed("JAVA");
		
	bool java;
		
	try{
		if (!args[4].empty()){
			string javastr = args[4];
			javastr.pop_back();
			java = javastr == needed;		
		}
		else{
			java = false;
		}
	}
	catch(const out_of_range& oor){
		java = false;
	}
	catch(const logic_error& le){
		java = false;
	}
	catch(const bad_alloc& ba){
		java = false;
	}
	
	return java;	
}


void InvalidMode(vector<string> args, Connection con) {
	con.push("Invalid Mode: " + args[0]);
}


void Echo(vector<string> args, Connection con) {
	con.push(args[1]);
}


//------------------------------------------------
//END MISC. SECTION


//BEGIN STRING TOOL SECTION
//------------------------------------------------


void tokenize(string const& str, const char delim, vector<string>& argst) {
	
    size_t start;
    size_t end = 0;

    while ((start = str.find_first_not_of(delim, end)) != string::npos)
    {
        end = str.find(delim, start);
        argst.push_back(str.substr(start, end - start));
    }
}


vector<string> split(string in_str, char delimiter) {

	vector<string> out;

    string between;
	
	stringstream check1(in_str);
	
	while(getline(check1, between, delimiter)){
		out.push_back(between);
	}
	
	return out;

}


//------------------------------------------------
//END STRING TOOLS SECTION


//BEGIN FILE TOOLS SECTION
//------------------------------------------------


bool DirExists(string dirpath) {
	struct stat buffer;
	return stat (dirpath.c_str(), &buffer) == 0;
}


void TouchFile(string filepath) {
    fstream fsy;
    fsy.open(filepath.c_str(), ios::out);
    fsy.close();
}


bool file_exists(const string name) {
    ifstream f(name.c_str());
    return f.good();
}


//------------------------------------------------
//END FILE TOOLS SECTION


//BEGIN WRITE SECTION
//------------------------------------------------


void Write(vector<string> args, Connection con) {
	
    string path = "DoThingData/" + args[1] + "/" + args[2] + ".csv";
    if (file_exists(path)) {
        remove(path.c_str());
    }

    ofstream File(path.c_str());
    string look = "END";
	string data;

    while (data.find(look) != string::npos) 
	{
        data = con.WaitUntilRecv();
		
		if (con.java)
		{
			File << data;
		}
		else
		{
			File << data << endl;
		}
		
        con.push("GO");
    }
    File.close();
}


void RenameGroup(vector<string> args, Connection con) {
	
	string path = "DoThingData/" + args[1] + "/" + args[2] + ".csv";
	con.push("Same");
	string to_put = con.WaitUntilRecv();
	if (con.java) to_put.pop_back();
	string newpath = "DoThingData/" + args[1] + "/" + to_put + ".csv";
	rename(path.c_str(), newpath.c_str());
	
}


void AddUser(string name) {
	
    string path = "DoThingData/" + name;
    fs::create_directory(path);
	
}


void DeleteGroup(vector<string> args, Connection con) {
	
	string path = "DoThingData/" + args[1] + "/" + args[2] + ".csv";
	if (file_exists(path)){
		
		remove(path.c_str());
		
	}
	
}


//------------------------------------------------
//END WRITE SECTION


//BEGIN READ SECTION
//------------------------------------------------


void GetGroups(vector<string> args, Connection con) {
	
    string path = "DoThingData/" + args[1] + "/";

    for (auto& p : fs::directory_iterator(path.c_str()))
    {
        string data = p.path().filename();

        string delimeter = ".";

        data = data.substr(0, data.find(delimeter));

        con.push(data);

        con.WaitUntilRecv();
    }

    con.push("END");
}


void Read(vector<string> args, Connection con) {

    string data = "";


    string EndCode = "END";
    string path = "DoThingData/" + args[1] + "/" + args[2] + ".csv";

    ifstream File(path);

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

void Proxy(string command, Connection con, string auth_ip) {
	
	Connection AuthCon(8081, auth_ip);
	
    AuthCon.push(command);

    string returnCode = AuthCon.WaitUntilRecv();
	
	AuthCon.dc();

	con.push(returnCode);
}


bool TestAuth(string ip) {
	
	try{
	
		Connection AuthCon(8081, ip);
		
		AuthCon.push("T/Hello");
	
		string result = AuthCon.WaitUntilRecv();
		
		AuthCon.dc();
		
		return result == "Hello";
	
	}
	catch (const char* ConError) {
		
		return false;
		
	}
	
}


string Validate(string name, string token, Connection con, string auth_ip) {
	
    Connection AuthCon(8081, auth_ip);
	
	AuthCon.push("V/" + name + "/" + token);
    
    string returnCode = AuthCon.WaitUntilRecv();
	
	AuthCon.dc();

	return returnCode;
}


void Forward(vector<string> args, Connection con, string auth_ip) {
	
	if (args[0] != "V"){
		string proxy_command;
		for (string part : args){
			if (getIndex(args, part) != 0){
				proxy_command += getIndex(args, part) == 1? part : "/" + part;
			}
		}
		Proxy(proxy_command, con, auth_ip);
	}
	else {
		
		con.push("Validate is for main server only!");
		
	}
}


void IfValid(vector<string> args, Connection con, string auth_ip, void (*f)(vector<string>, Connection)) {
	
	string returnCode = Validate(args[1], args[3], con, auth_ip);
	
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


void ConLoop(Connection con, string auth_ip) {
        string input = con.recieve();
        const vector<string> args = split(input, '/');
        const char mode = args[0][0];
		con.java = JavaCheck(args);
		
		
		switch(mode){
			case 'R': IfValid(args, con, auth_ip, Read); 		break;
			case 'W': IfValid(args, con, auth_ip, Write); 		break;
			case 'D': IfValid(args, con, auth_ip, DeleteGroup); break;
			case 'G': IfValid(args, con, auth_ip, GetGroups); 	break;
			case 'N': IfValid(args, con, auth_ip, RenameGroup); break;
			case 'A': Forward(args, con, auth_ip); 				break;
			case 'T': Echo(args, con); 							break;
			default : InvalidMode(args, con); 					break;
		}
}


int main() {
	
	if (!DirExists("DoThingData")){
		cout << "No User Data Directory Found, Creating One" << endl;
		fs::create_directory("DoThingData");
	}
	
	string auth_ip;
	
	if (!file_exists("Server.config")){
		cout << "No config detected, entering first time set-up" << endl;
		cout << "Enter the ip address of the auth server you wish to use: ";
		cin >> auth_ip;
		cout << "Contacting Auth Server..." << endl;
		bool valid = TestAuth(auth_ip);
		if (valid){
			cout << "Succesfully validated with auth server" << endl;
			cout << "Saving Server Setup Data In Server.config" << endl;
			ofstream File("Server.config");
			File << auth_ip << endl;
			File.close();
			cout << "Save Complete, starting the server" << endl;
		}
		else{
			cout << "Error, Auth Server didnt respond correctly" << endl;
			return 0;
		}
		
	}
	else {
		string data = "";
		ifstream File("Server.config");
		vector<string> lines;
		while (getline(File, data))
		{
			if (data != "")
			{
				lines.push_back(data);
			}
		}
		if (lines.size() != 1)
		{
			cout << "Error, cant read config file. Delete it if you wish to go through setup again" << endl;
			return 0;
		}
		auth_ip = lines[0];
		if (!TestAuth(auth_ip)){
			cout << "Error, the auth server did not respond on start (start the auth server first!)" << endl;
			return 0;
		}
	}
	
	cout << "DoThing Server Started" << endl;
	
	HostConnection hostCon(8080);
	hostCon.MainLoop(ConLoop, auth_ip, true);
	
	return 0;
}


//------------------------------------------------
//END MAIN SECTION




