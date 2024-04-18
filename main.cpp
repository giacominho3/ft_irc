#include "Server.hpp"

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cout << RED << "Wrong usage" << std::endl;
        std::cout << WHI << "Correct usage is: ./irc <port> <password>" << std::endl;
        return 1;
    }
    int port = atoi(argv[1]);
    std::string password = argv[2];
    Server serv;
    try
    {
        signal(SIGINT, Server::SignalHandler);
        signal(SIGQUIT, Server::SignalHandler);
        serv.ServerInit(port, password);
    }
    catch(const std::exception& e)
    {
        serv.CloseFds();
        std::cerr << e.what() << std::endl;
    }
}
