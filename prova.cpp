#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <csignal>

#define RED "\e[1;31m"
#define GRE "\e[1;32m"
#define WHI "\e[0;37m"

class Server {
private:
    int Port;
    int SerSocketFd;
    static bool Signal;
    std::vector<struct pollfd> fds;

public:
    Server() { SerSocketFd = -1; }

    void ServerInit(std::string password);
    void SerSocket();
    void AcceptNewClient(std::string password);
    static void SignalHandler(int signum); // Modifica qui
    void CloseFds();
    void Run(std::string password);
};

bool Server::Signal = false;

void Server::SignalHandler(int signum)
{
    std::cout << std::endl << "Signal Received!" << std::endl;
    if (signum == 2)
        Signal = true;
}

void Server::CloseFds() {
    if (SerSocketFd != -1) {
        std::cout << RED << "Server <" << SerSocketFd << "> Disconnected" << WHI << std::endl;
        close(SerSocketFd);
    }
}

void Server::AcceptNewClient(std::string password)
{
    struct sockaddr_in cliadd;
    struct pollfd NewPoll;
    socklen_t len = sizeof(cliadd);

    int incofd = accept(SerSocketFd, (sockaddr *)&(cliadd), &len);
    if (incofd == -1) {
        std::cout << "accept() failed" << std::endl;
        return;
    }

    char buffer[1024];
    ssize_t bytes_received = recv(incofd, buffer, sizeof(buffer), 0);
    if (bytes_received == -1) {
        std::cerr << "recv() failed" << std::endl;
        close(incofd);
        return;
    }

    buffer[bytes_received] = '\0';
    if (strcmp(buffer, password.c_str()) != 0) {
        std::cerr << "Invalid password" << std::endl;
        close(incofd);
        return;
    }

    if (fcntl(incofd, F_SETFL, O_NONBLOCK) == -1) {
        std::cout << "fcntl() failed" << std::endl;
        return;
    }

    NewPoll.fd = incofd;
    NewPoll.events = POLLIN;
    NewPoll.revents = 0;
    fds.push_back(NewPoll);

    std::cout << GRE << "Client <" << incofd << "> Connected" << WHI << std::endl;
}

void Server::SerSocket() {
    struct sockaddr_in add;
    struct pollfd NewPoll;
    add.sin_family = AF_INET;
    add.sin_port = htons(this->Port);
    add.sin_addr.s_addr = INADDR_ANY;

    SerSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (SerSocketFd == -1)
        throw(std::runtime_error("faild to create socket"));

    int en = 1;
    if (setsockopt(SerSocketFd, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == -1)
        throw(std::runtime_error("faild to set option (SO_REUSEADDR) on socket"));
    if (fcntl(SerSocketFd, F_SETFL, O_NONBLOCK) == -1)
        throw(std::runtime_error("faild to set option (O_NONBLOCK) on socket"));
    if (bind(SerSocketFd, (struct sockaddr *)&add, sizeof(add)) == -1)
        throw(std::runtime_error("faild to bind socket"));
    if (listen(SerSocketFd, SOMAXCONN) == -1)
        throw(std::runtime_error("listen() faild"));

    NewPoll.fd = SerSocketFd;
    NewPoll.events = POLLIN;
    NewPoll.revents = 0;
    fds.push_back(NewPoll);
}

void Server::ServerInit(std::string password)
{
    this->Port = 4444;
    SerSocket();

    std::cout << GRE << "Server <" << SerSocketFd << "> Connected" << WHI << std::endl;
    std::cout << "Waiting to accept a connection...\n";

    while (!Signal) {
        if ((poll(&fds[0], fds.size(), -1) == -1) && !Signal)
            throw(std::runtime_error("poll() faild"));

        Run(password);
    }
    CloseFds();
}

void Server::Run(std::string password)
{
    for (size_t i = 0; i < fds.size(); i++) {
        if (fds[i].revents & POLLIN) {
            if (fds[i].fd == SerSocketFd)
                AcceptNewClient(password);
        }
    }
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cout << argc << std::endl;
        return 1;
    }
    std::string pw = argv[1];
    Server ser;
    std::cout << "---- SERVER ----" << std::endl;
    try {
        signal(SIGINT, Server::SignalHandler);
        signal(SIGQUIT, Server::SignalHandler);
        ser.ServerInit(pw);
    } catch(const std::exception& e) {
        ser.CloseFds();
        std::cerr << e.what() << std::endl;
    }
    std::cout << "The Server Closed!" << std::endl;
}
