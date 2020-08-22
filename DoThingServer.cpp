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

void Write(std::string name, std::string group, int valread, int new_socket, bool truedelete) {
    std::string fin = "/home/dev/DoThingData/" + name + "/" + group + ".csv";
    if (file_exists(fin) == true) {
        std::remove(fin.c_str());
        if (truedelete) {
            return;
        }
    }

    ofstream File(fin.c_str());
    std::string look = "END";
    char wl[1024] = { 0 };
    char empt[1024] = { 0 };
    memset(empt, ' ', 1023);
    empt[1024] = '\0';

    while (true) {
        valread = read(new_socket, wl, 1024);
        if (wl != "" && wl != empt) {
            if (conchartostring(wl).find(look) != std::string::npos) {
                break;
            }
            else {
                File << wl << std::endl;
                send(new_socket, "GO", 3, 0);
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

void GetGroups(std::string name, int sockfd) {

    std::cout << "Started Getting Groups" << std::endl;

    std::string data = "";


    const char* hello = "END";
    std::string fin = "/home/dev/DoThingData/" + name + "/";

    for (auto& p : fs::directory_iterator(fin.c_str()))
    {
        std::vector<std::string> out;

        int valread;

        std::string data = p.path().filename();

        std::string delimeter = ".";

        data = data.substr(0, data.find(delimeter));

        int n = data.length();

        std::cout << data << std::endl;

        char tm[1024] = { 0 };

        send(sockfd, data.c_str(), n, 0);

        while (true) {
            valread = read(sockfd, tm, 1024);
            if (tm != "") {
                memset(tm, ' ', 1023);
                tm[1024] = '\0';
                break;
            }
        }


    }

    send(sockfd, hello, strlen(hello), 0);
}

std::string Validate(std::string name, std::string token, int sockfd) {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = { 0 };
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        send(sockfd, "IE", 3, 0);
        return "IE";
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);

    // Convert IPv4 and IPv6 addresses from text to binary form 
    if (inet_pton(AF_INET, "192.168.86.39", &serv_addr.sin_addr) <= 0)
    {
        send(sockfd, "IE", 3, 0);
            return "IE";
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        send(sockfd, "IE", 3, 0);
            return "IE";
    }
    std::string fin = "V/" + name + "/" + token;
    send(sock, fin.c_str(), strlen(fin.c_str()), 0);
    char tm[1024] = { 0 };
    valread = read(sock, tm, 1024);
    
    std::string returnCode = conchartostring(tm);

    close(sock);

    if (returnCode == "IU") {
        send(sockfd, "IU", 3, 0);
        return "IU";
    }
    else if (returnCode == "IT") {
        send(sockfd, "IT", 3, 0);
        return "IT";
    }
    else if (returnCode == "VT") {
        send(sockfd, "VT", 3, 0);
        return "VT";
    }

    return "IE";
}

void Read(std::string name, std::string group, int valread, int new_socket) {

    std::string data = "";


    const char* hello = "END";
    std::string fin = "/home/dev/DoThingData/" + name + "/" + group + ".csv";

    std::cout << fin << std::endl;

    std::ifstream File(fin);

    if (!File) {
        send(new_socket, hello, strlen(hello), 0);
        return;
    }

    while (getline(File, data))
    {
        int n = data.length();

        std::cout << data << std::endl;

        char tm[1024] = { 0 };

        send(new_socket, data.c_str(), n, 0);

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

    send(new_socket, hello, strlen(hello), 0);

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


int main()
{
	
	std::cout << "Server started" << std::endl;


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

        std::cout << buffer << std::endl;

        std::string mode = ou[0];

        std::string code = Validate(ou[1], ou[3], new_socket);

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

        if (mode[0] == 'R') {
            Read(ou[1], ou[2], valread, new_socket);
        }
        else if (mode[0] == 'W') {
            Write(ou[1], ou[2], valread, new_socket, false);
        }
        else if (mode[0] == 'D') {
            std::string fin = "/home/dev/DoThingData/" + ou[1] + "/" + ou[2] + ".csv";
            if (file_exists(fin) == true) {
                std::remove(fin.c_str());
            }
        }
        else if (mode[0] == 'G') {
            GetGroups(ou[1], new_socket);
        }
        else if(mode[0] == 'N') {
            std::string fin = "/home/dev/DoThingData/" + ou[1] + "/" + ou[2] + ".csv";
            send(new_socket, "Same", 5, 0);
            while (true) {
                valread = read(new_socket, conf, 1024);
                if (conf != "") {
                    break;
                }
            }
            std::cout << conf << std::endl;
            std::string newstr = "/home/dev/DoThingData/" + ou[1] + "/" + string(conf) + ".csv";
            int result = rename(fin.c_str(), newstr.c_str());
            std::cout << result << std::endl;
        }
        else{
            send(new_socket, "Invalid mode!", 14, 0);
            break;
        }

        memset(buffer, '\0', 1023);

        close(new_socket);

        std::cout << "Connection Closed" << std::endl;
    }
	
	return 0;
}
