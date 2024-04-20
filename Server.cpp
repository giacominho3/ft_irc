#include "Server.hpp"

bool Server::Signal = false;

int Server::ServerError(std::string message, int incofd)
{
    std::cerr << RED << errno << " " << message << WHI << std::endl;
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

int Server::AcceptNewClient()
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

    // ssize_t bytes_received = recv(incofd, buffer, sizeof(buffer), 0);
    // if (bytes_received == -1) {
    //     if (errno != EWOULDBLOCK && errno != EAGAIN) {
    //         return ServerError("recv() failed", incofd);
    //     }
    // }

    // buffer[bytes_received] = '\0';

    // if (bytes_received > 0 && strcmp(buffer, password.c_str()) != 0) {
    //     return ServerError("invalid password", incofd);
    // }

    NewPoll.fd = incofd;
    NewPoll.events = POLLIN;
    fds.push_back(NewPoll);

    std::cout << MAG << "Client <" << incofd << "> Connected" << WHI << std::endl;
    return 0;
}


void Server::SetupServSocket()
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
    this->Password = password;
    SetupServSocket();

    std::cout << GRE << "Server <" << SerSocketFd << "> Connected" << WHI << std::endl;
    std::cout << "Waiting to accept a connection..." << std::endl;

    while (!Signal) {
        if ((poll(&fds[0], fds.size(), -1) == -1) && !Signal)
            throw(std::runtime_error("poll() faild"));

        Run();
    }
    CloseFds();
}


void Server::HandleClient(int client_fd)
{
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    if (bytes_received == -1) {
        if (errno == EWOULDBLOCK || errno == EAGAIN)
        {
            return;
        }
        else
        {
            std::cerr << "recv() failed from client " << client_fd << " with error: " << strerror(errno) << std::endl;
            close(client_fd);
            client_buffers.erase(client_fd);
            return;
        }
    }
    else if (bytes_received == 0)
    {
        std::cout << RED << "Client <" << client_fd << "> disconnected" << WHI << std::endl;
        close(client_fd);
        client_buffers.erase(client_fd);
        return;
    }
    else
    {
        client_buffers[client_fd].append(buffer, bytes_received);

        size_t pos;
        while ((pos = client_buffers[client_fd].find('\n')) != std::string::npos)
        {
            std::string command = client_buffers[client_fd].substr(0, pos);
            HandleMessage(client_fd, command);
            client_buffers[client_fd].erase(0, pos + 1);
        }
    }
}


void Server::HandleMessage(int client_fd, std::string command)
{
    std::cout << "Messagge from client <" << client_fd << ">: " << command << std::endl;

    std::string command_type = GetCommandType(command);
    std::string command_params = GetCommandParams(command);

    if (strcmp(command_type.c_str(), "PASS") == 0)
    {
        if (clients.find(client_fd) != clients.end())
            std::cout << "You had already inserted the password!" << std::endl;
        else
        {
            if (strcmp(command_params.c_str(), Password.c_str()) == 0)
            {
                Client client(client_fd);
                clients.insert(std::pair<int, Client>(client_fd, client));
            }
            else
                std::cout << RED << "WRONG PASSWORD" << WHI << std::endl;
        }
    }

    else
    {
        if (clients.find(client_fd) == clients.end())
        {
            std::cout << RED << "Password needed to communicate!" << WHI << std::endl;
            return ;
        }
        else
        {
            Client &client = clients.find(client_fd)->second;
            if (strcmp(command_type.c_str(), "NICK") == 0 || strcmp(command_type.c_str(), "USER") == 0)
                HandleLogging(client, command_type, command_params);
        }
    }

}

std::string Server::GetCommandType(std::string command)
{
    int pos = 0;

    while (isupper(command[pos]))
        pos++;
    
    std::string type = command.substr(0, pos);
    return type;
}

std::string Server::GetCommandParams(std::string command)
{
    int pos = 0;

    while (isupper(command[pos]))
        pos++;
    
    std::string params = command.substr(pos + 1, command.length());
    return params;
}

void Server::HandleLogging(Client &client, std::string type, std::string params)
{
    if (strcmp(type.c_str(), "NICK") == 0)
    {
        if (params.length() == 0)
            std::cout << RED << "Wrong syntax for NICK command" << WHI << std::endl << "Correct syntax is: NICK <nickname>" << std::endl;
        client.setNickname(params);
    }

    else if (strcmp(type.c_str(), "USER") == 0)
    {
        // USER <username> <hostname> <servername> :<realname>
        if (params.length() == 0)
            std::cout << RED << "Wrong syntax for USER command" << WHI << std::endl << "Correct syntax is: USER <username> <hostname> <servername> :<realname>" << std::endl;
        size_t firstSpace = params.find(' ');
        size_t secondSpace = params.find(' ', firstSpace + 1);
        size_t colon = params.find(':', secondSpace + 1);

        if (firstSpace != std::string::npos && secondSpace != std::string::npos && colon != std::string::npos)
        {
            client.setUsername(params.substr(0, firstSpace + 1));
            client.setRealname(params.substr(colon + 1));
            std::cout << client.getUsername() << std::endl;
            std::cout << client.getRealname() << std::endl;
        }
    }
}


void Server::Run()
{
    for (size_t i = 0; i < fds.size(); i++)
    {
        usleep(100);
        if (fds[i].revents & POLLIN)
        {
            if (fds[i].fd == SerSocketFd)
                AcceptNewClient();
            else
                HandleClient(fds[i].fd);
        }
    }
}