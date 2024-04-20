#include <string>

class Client
{
    public:
        Client(int fd) : fd_(fd) {};
        Client(int fd, const std::string& nick, const std::string& user, const std::string& realname)
            : fd_(fd), nickname_(nick), username_(user), realname_(realname) {}

        int getFD() const { return fd_; }
        const std::string& getNickname() const { return nickname_; }
        const std::string& getUsername() const { return username_; }
        const std::string& getRealname() const { return realname_; }

        void setNickname(const std::string& nickname) { nickname_ = nickname; }
        void setUsername(const std::string& username) { username_ = username; }
        void setRealname(const std::string& realname) { realname_ = realname; }

    private:
        int fd_;
        std::string nickname_;
        std::string username_;
        std::string realname_;
};
