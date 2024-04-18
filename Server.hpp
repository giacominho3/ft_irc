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
#include <string>
#include <cstring>
#include <errno.h>
#include <stdlib.h>

#define RED "\e[1;31m"
#define GRE "\e[1;32m"
#define MAG "\e[0;35m"
#define WHI "\e[0;37m"

class Server
{
    private:
        int Port;
        int SerSocketFd;
        static bool Signal;
        std::vector<struct pollfd> fds;

    public:
        Server() { SerSocketFd = -1; }

        void ServerInit(int port, std::string password);
        int ServerError(std::string message, int incofd);
        void SerSocket();
        int AcceptNewClient(std::string password);
        void HandleClient(int client_fd);
        static void SignalHandler(int signum);
        void CloseFds();
        void Run(std::string password);
};