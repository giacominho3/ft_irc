#include "Server.hpp"

bool Server::Signal = false;

int Server::ServerError(std::string message, int incofd)
{
    std::cerr << RED << errno << " " << message << std::endl;
    close(incofd);
    return errno;
}

void Server::SignalHandler(int signum)
{
    std::cout << std::endl << "Signal Received!" << std::endl;
    if (signum == 2)
        Signal = true;
}

void Server::CloseFds()
{
    if (SerSocketFd != -1) {
        std::cout << RED << "Server <" << SerSocketFd << "> Disconnected" << WHI << std::endl;
        close(SerSocketFd);
    }
}

int Server::AcceptNewClient(std::string password)
{
    struct sockaddr_in cliadd;
    struct pollfd NewPoll;
    socklen_t len = sizeof(cliadd);
    char buffer[1024];

    memset(buffer, 0, sizeof(buffer));
    int incofd = accept(SerSocketFd, (sockaddr *)&(cliadd), &len);
    if (incofd == -1) {
        std::cerr << "accept() failed: " << strerror(errno) << std::endl;
        return 1;
    }

    if (fcntl(incofd, F_SETFL, O_NONBLOCK) == -1) {
        std::cerr << "fcntl() failed to set non-blocking mode: " << strerror(errno) << std::endl;
        close(incofd);
        return 1;
    }

    ssize_t bytes_received = recv(incofd, buffer, sizeof(buffer), 0);
    if (bytes_received == -1) {
        if (errno != EWOULDBLOCK && errno != EAGAIN) {
            return ServerError("recv() failed", incofd);
        }
    }

    buffer[bytes_received] = '\0';

    if (bytes_received > 0 && strcmp(buffer, password.c_str()) != 0) {
        return ServerError("invalid password", incofd);
    }

    NewPoll.fd = incofd;
    NewPoll.events = POLLIN;
    fds.push_back(NewPoll);

    std::cout << MAG << "Client <" << incofd << "> Connected" << WHI << std::endl;
    return 0;
}


void Server::SerSocket()
{
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

void Server::ServerInit(int port, std::string password)
{
    this->Port = port;
    SerSocket();

    std::cout << GRE << "Server <" << SerSocketFd << "> Connected" << WHI << std::endl;
    std::cout << "Waiting to accept a connection..." << std::endl;

    while (!Signal) {
        if ((poll(&fds[0], fds.size(), -1) == -1) && !Signal)
            throw(std::runtime_error("poll() faild"));

        Run(password);
    }
    CloseFds();
}

void Server::HandleClient(int client_fd)
{
    char msg[1024];

    memset(msg, 0, sizeof(msg));
    while (true)
    {
        ssize_t bytes_received = recv(client_fd, msg, sizeof(msg), 0);
        if (bytes_received == -1)
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN)
                continue;
            else
            {
                std::cerr << "recv() failed from client " << client_fd << " with error: " << strerror(errno) << std::endl;
                break;
            }
        }

        if (bytes_received == 0) {
            std::cout << RED << "Client <" << client_fd << "> disconnected" << WHI << std::endl;
            break;
        }

        msg[bytes_received] = '\0';
        // gestione dei messaggi in arrivo
        std::cout << "Message from client <" << client_fd << ">: " << msg << std::endl;
    }

    close(client_fd);
}


void Server::Run(std::string password)
{
    for (size_t i = 0; i < fds.size(); i++)
    {
        if (fds[i].revents & POLLIN)
        {
            if (fds[i].fd == SerSocketFd)
                AcceptNewClient(password);
            else
                HandleClient(fds[i].fd);
        }
    }
}