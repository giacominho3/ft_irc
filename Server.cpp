#include "Server.hpp"

// modificare tutti i messaggi per farli aderire allo standard

bool Server::Signal = false;

int Server::ServerError(std::string message, int incofd)
{
    std::cerr << RED << errno << " " << message << WHI << std::endl;
    close(incofd);
    return errno;
}

void Server::ServerResponse(std::string message, int client_fd)
{
    const char* passwordMsg = message.c_str();
    ssize_t sent = send(client_fd, passwordMsg, strlen(passwordMsg), 0);
    if (sent == -1)
    {
        std::cerr << RED << "Failed to send welcome message: " << strerror(errno) << WHI << std::endl;
        close(client_fd);
        return ;
    }
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

std::map<int, Client>::iterator Server::findClientByUsername(std::map<int, Client>& clients, std::string& username)
{
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        if (it->second.getUsername() == username)
            return it;
    }
    return clients.end();
}

std::map<int, Client>::iterator Server::findClientByNickname(std::map<int, Client>& clients, std::string& nickname)
{
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        if (it->second.getNickname() == nickname)
            return it;
    }
    return clients.end();
}

std::map<std::string, Channel>::iterator Server::findChannelByName(std::map<std::string, Channel>& channels, std::string& name)
{
    for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); ++it)
    {
        if (it->first == name)
            return it;
    }
    return channels.end();
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
    size_t pos = command.find(' ');

    if (pos == std::string::npos)
        return "";

    if (pos + 1 < command.length())
        return command.substr(pos + 1);
    else
        return "";
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

    NewPoll.fd = incofd;
    NewPoll.events = POLLIN;
    fds.push_back(NewPoll);

    std::cout << MAG << "Client <" << incofd << "> Connected" << WHI << std::endl;

    ServerResponse("\e[1;31m\n:Your Server 001 :Password needed for authentication to this Server.\r\n\e[0;37m", incofd);
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
            if (command == "QUIT")
            {
                std::cout << RED << "Client <" << client_fd << "> disconnected" << WHI << std::endl;
                send(client_fd, "QUIT", 4, 0);
                close(client_fd);
                client_buffers.erase(client_fd);
                return;
            }
            HandleMessage(client_fd, command);
            client_buffers[client_fd].erase(0, pos + 1);
        }
    }
}


void Server::HandleMessage(int client_fd, std::string command)
{
    // da togliere
    std::cout << "Messagge from client <" << client_fd << ">: " << command << std::endl;

    std::string command_type = GetCommandType(command);
    std::string command_params = GetCommandParams(command);

    // creare metodo per handlepass
    if (command_type == "PASS")
    {
        if (clients.find(client_fd) != clients.end())
            std::cout << "You had already inserted the password!" << std::endl;
        else
        {
            if (command_params == Password)
            {
                Client client(client_fd);
                clients.insert(std::pair<int, Client>(client_fd, client));
            }
            else
            {
                std::string response = "\e[1;31m\n:YourServer 001 :WRONG PASSWORD!\e[0;37m\r\n";
                ServerResponse(response, client_fd);
            }
        }
    }

    else
    {
        if (clients.find(client_fd) == clients.end())
        {
            std::string response = "\e[1;31m\n:YourServer 001 :Password needed to commuinicate!\e[0;37m\r\n";
            ServerResponse(response, client_fd);
            return ;
        }
        else
        {
            Client &client = clients.find(client_fd)->second;
            if (command_type == "NICK" || command_type == "USER" || command_type == "OPER")
                HandleLogging(client_fd, client, command_type, command_params);
            else if (command_type == "JOIN" || command_type == "PART")
                HandleChannels(client_fd, client, command_type, command_params);
            else if (command_type == "INVITE")
                HandleChannelOpers(client_fd, client, command_type, command_params);
            else if (command_type == "PRIVMSG")
                HandlePrivateMsg(client_fd, client, command_params);
        }
    }

}


void Server::HandleLogging(int client_fd, Client &client, std::string type, std::string params)
{
    // controllare se nick o username già esistono
    // permettere la sostituzione di nick e username

    // NICK command
    if (type == "NICK")
    {
        if (params.empty())
        {
            std::string response = "\e[1;31m\n:YourServer 001 :Wrong syntax for NICK command\n\e[0;37mCorrect syntax is: NICK <nickname>\r\n";
            ServerResponse(response, client_fd);
            return ;
        }
        
        client.setNickname(params);
        if (client.getNickname().length() > 0 && client.getUsername().length() > 0)
        {
            std::string response = "\e[1;32m\n:YourServer 001 " + client.getNickname() + " :Welcome to the IRC Network " + client.getNickname() + "!" + client.getUsername() + "@localhost\r\n\e[0;37m";
            ServerResponse(response, client_fd);
        }
    }

    // USER command
    else if (type == "USER")
    {
        if (params.empty())
        {
            std::string response = "\e[1;31m\n:YourServer 001 :Wrong syntax for USER command\n\e[0;37mCorrect syntax is: USER <username> <hostname> <servername> :<realname>\r\n";
            ServerResponse(response, client_fd);
        }
        size_t firstSpace = params.find(' ');
        size_t secondSpace = params.find(' ', firstSpace + 1);
        size_t colon = params.find(':', secondSpace + 1);

        if (firstSpace != std::string::npos && secondSpace != std::string::npos && colon != std::string::npos)
        {
            client.setUsername(params.substr(0, firstSpace));
            client.setRealname(params.substr(colon + 1, params.length()));
            if (client.getNickname().length() > 0 && client.getUsername().length() > 0)
            {
                std::string response = "\e[1;32m\n:YourServer 001 " + client.getNickname() + " :Welcome to the IRC Network " + client.getNickname() + "!" + client.getUsername() + "@localhost\r\n\e[0;37m";
                ServerResponse(response, client_fd);
            }
        }
    }

    // OPER command
    else if (type == "OPER")
    {
        if (params.empty())
        {
            std::string response = "\e[1;31m\n:YourServer 001 :Wrong syntax for OPER command\n\e[0;37mCorrect syntax is: OPER <username> <password>\r\n";
            ServerResponse(response, client_fd);
        }

        size_t firstSpace = params.find(' ');
        std::string tempUser = params.substr(0, firstSpace);
        std::map<int, Client>::iterator it = findClientByUsername(clients, tempUser);

        if (it != clients.end())
        {
            size_t secondSpace = params.find(' ', firstSpace + 1);
            std::string tempOpPass = params.substr(firstSpace + 1, secondSpace);

            if (tempOpPass == opPass)
            {
                Client &tempClient = it->second;
                if (tempClient.getOper())
                {
                    std::string response = "\n:YourServer 001 :" + tempClient.getUsername() + " is already an operator for this Server!\r\n";
                    ServerResponse(response, client_fd);
                }
                else
                {
                    tempClient.setOper(1);
                    std::string response = "\n:YourServer 001 :" + tempClient.getUsername() + " is now an operator for this Server\r\n";
                    ServerResponse(response, client_fd);
                }
            }
        }
    }
}


void Server::HandleChannels(int client_fd, Client &client, std::string type, std::string params)
{
    // JOIN command
    if (type == "JOIN")
    {
        if (params.empty())
        {
            std::string response = "\e[1;31m\n:YourServer 001 :Wrong syntax for JOIN command\n\e[0;37mCorrect syntax is: JOIN #<channel-name>[,<channel-name,...]\r\n";
            ServerResponse(response, client_fd);
        }
        else
        {
            std::istringstream iss(params);
            std::string channelName;

            while (std::getline(iss, channelName, ','))
            {
                channelName.erase(0, channelName.find_first_not_of(" \n\r\t\f\v#"));
                channelName.erase(channelName.find_last_not_of(" \n\r\t\f\v") + 1);

                if (channels.find(channelName) == channels.end())
                    channels.insert(std::pair<std::string,Channel>(channelName, Channel(channelName)));

                if (channels[channelName].isMember(&client))
                {
                    std::string response = "YourServer 001 :You are already a member of #" + channelName + "\r\n";
                    ServerResponse(response, client_fd);
                }
                else
                {
                    channels[channelName].addMember(&client);
                    std::string response = "\e[1;32m\n:YourServer 001 :\e[0;37m Welcome to #" + channelName + " " + client.getUsername() + "\r\n";
                    ServerResponse(response, client_fd);
                }
            }
        }
    }

    // comando PART
    else if (type == "PART")
    {
        if (params.empty())
        {
            std::string response = "\e[1;31m\n:YourServer 001 :Wrong syntax for PART command\n\e[0;37mCorrect syntax is: PART #<channel-name>\r\n";
            ServerResponse(response, client_fd);
        }
        else
        {
            params.erase(0, params.find_first_not_of(" \n\r\t\f\v#"));
            params.erase(params.find_last_not_of(" \n\r\t\f\v") + 1);
            if (channels.find(params) == channels.end())
            {
                std::string response = "\e[1;31m\n:YourServer 001 :The channel you are trying to leave does not exist\r\n";
                ServerResponse(response, client_fd);
            }
            else
            {
                if (channels[params].isMember(&client))
                {
                    channels[params].removeMember(&client);
                    std::string response = "YourServer 001 :You left the channel #" + params + "\r\n";
                    ServerResponse(response, client_fd);
                }
                else
                {
                    std::string response = "YourServer 001 :You are not a member of the channel #" + params + "\r\n";
                    ServerResponse(response, client_fd);
                }
            }
        }
    }
}


void Server::HandleChannelOpers(int client_fd, Client &client, std::string type, std::string params)
{
    (void)client;
    // INVITE command
    if (type == "INVITE")
    {
        if (params.empty())
        {
            std::string response = "\e[1;31m\n:YourServer 001 :Wrong syntax for INVITE command\n\e[0;37mCorrect syntax is: INVITE <nickname> #<channel-name>\r\n";
            ServerResponse(response, client_fd);
        }
        else if (!client.getOper())
        {
            std::string response = "\e[1;31m\n:YourServer 001 : You are not an operator for this Server!\e[0;37m\n\r\n";
            ServerResponse(response, client_fd);
        }
        else
        {
            size_t firstSpace = params.find(' ');
            std::string tempUser = params.substr(0, firstSpace);
            std::string tempChannel = params.substr(firstSpace + 1, params.length());
            tempUser.erase(0, tempUser.find_first_not_of(" \n\r\t\f\v"));
            tempUser.erase(tempUser.find_last_not_of(" \n\r\t\f\v") + 1);
            tempChannel.erase(0, tempChannel.find_first_not_of(" \n\r\t\f\v#"));
            tempChannel.erase(tempChannel.find_last_not_of(" \n\r\t\f\v") + 1);

            if (findClientByNickname(clients, tempUser) == clients.end())
            {
                // gestione errore
            }
            else if (findChannelByName(channels, tempChannel) == channels.end())
            {
                // gestione errore
            }
            else
            {
                std::string toSendStr = client.getNickname() + " " + type + " " + tempUser + " :#" + tempChannel;
                const char *toSend = toSendStr.c_str();
                Client tmp = findClientByNickname(clients, tempUser)->second;
                send(tmp.getFD(), toSend, strlen(toSend), 0);
            }
        }
    }
}


void Server::HandlePrivateMsg(int client_fd, Client &client, std::string params)
{
    // PRIVMSG command
    if (params.empty())
    {
        std::string response = "\e[1;31m\n:YourServer 001 :Wrong syntax for PRIVMSG command\n\e[0;37mCorrect syntax is: PRIVMSG #<channel-name>/<nickname> :<message>\r\n";
        ServerResponse(response, client_fd);
    }
    else
    {
        size_t divider = params.find(':');
        std::string receiver = params.substr(0, divider - 1);
        receiver.erase(0, receiver.find_first_not_of(" \n\r\t\f\v"));
        receiver.erase(receiver.find_last_not_of(" \n\r\t\f\v") + 1);
        std::string message = params.substr(divider + 1, params.length());
        message = client.getNickname() + " : " + message;
        const char *toSend = message.c_str();

        if (receiver.find('#') != receiver.npos)
        {
            // il messaggio è per un canale
            receiver.erase(0, receiver.find_first_not_of("#"));
            if (findChannelByName(channels, receiver) != channels.end())
            {
                Channel &channel = findChannelByName(channels, receiver)->second;
                if (channel.isMember(&client))
                {
                    std::unordered_set<Client*> channelMembers = channel.getMembers();
                    for (std::unordered_set<Client*>::iterator it = channelMembers.begin(); it != channelMembers.end(); ++it)
                    {
                        Client* c = *it;
                        if (c->getFD() != client_fd)
                            send(c->getFD(), toSend, strlen(toSend), 0);
                        // controllare errori nell'invio
                    }
                }
                else
                {
                    std::string response = "\e[1;31m\n:YourServer 001 :You are not a member of this channel!\e[0;37m\r\n";
                    ServerResponse(response, client_fd);
                }
            }
            else
            {
                std::string response = "\e[1;31m\n:YourServer 001 :Seems like the channel name you typed does not exist!\e[0;37m\r\n";
                ServerResponse(response, client_fd);
            }
        }

        else
        {
            // il messaggio è per un utente
            if (findClientByNickname(clients, receiver) != clients.end())
            {
                Client &receiverClient = findClientByNickname(clients, receiver)->second;
                send(receiverClient.getFD(), toSend, strlen(toSend), 0);
                // controllare errori nell'invio
            }
            else
            {
                std::string response = "\e[1;31m\n:YourServer 001 :Seems like the nickname you typed does not exist!\e[0;37m\r\n";
                ServerResponse(response, client_fd);
            }
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