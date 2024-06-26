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
#include <map>
#include <ctype.h>
#include <sstream>
#include "Client.hpp"
#include "Channel.hpp"

#define RED "\e[1;31m"
#define GRE "\e[1;32m"
#define MAG "\e[0;35m"
#define WHI "\e[0;37m"

class Server
{
    private:
        int Port;
        std::string Password;
        std::string opPass;
        int SerSocketFd;
        static bool Signal;
        std::vector<struct pollfd> fds;
        std::map<int, std::string> client_buffers;
        std::map<int, Client> clients;
        std::map<std::string, Channel> channels;

    public:
        Server() { SerSocketFd = -1; opPass = "13091995"; }

        std::map<int, Client>::iterator findClientByUsername(std::map<int, Client>& clients, std::string& username);
        std::map<int, Client>::iterator findClientByNickname(std::map<int, Client>& clients, std::string& nickname);
        std::map<std::string, Channel>::iterator findChannelByName(std::map<std::string, Channel>& channels, std::string& name);
        void ServerInit(int port, std::string password);
        int ServerError(std::string message, int incofd);
        void ServerResponse(std::string message, int client_fd);
        void SetupServSocket();
        int AcceptNewClient();
        void HandleClient(int client_fd);
        void HandleMessage(int client_fd, std::string message);
        std::string GetCommandType(std::string command);
        std::string GetCommandParams(std::string command);
        static void SignalHandler(int signum);
        void CloseFds();
        void HandleLogging(int client_fd, Client &client, std::string type, std::string params);
        void HandleChannels(int client_fd, Client &client, std::string type, std::string params);
        void HandleChannelOpers(int client_fd, Client &client, std::string type, std::string params);
        void HandlePrivateMsg(int client_fd, Client &client, std::string params);
        void Run();
};