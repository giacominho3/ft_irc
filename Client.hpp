#include <string>

class Client
{
    public:
        Client(int fd) : fd_(fd), isOper_(0) {};
        Client(int fd, const std::string& nick, const std::string& user, const std::string& realname)
            : fd_(fd), nickname_(nick), username_(user), realname_(realname), isOper_(0) {}

        int getFD() const { return fd_; }
        const std::string& getNickname() const { return nickname_; }
        const std::string& getUsername() const { return username_; }
        const std::string& getRealname() const { return realname_; }
        int getOper() const { return isOper_; }

        void setNickname(const std::string& nickname) { nickname_ = nickname; }
        void setUsername(const std::string& username) { username_ = username; }
        void setRealname(const std::string& realname) { realname_ = realname; }
        void setOper(const int oper) { isOper_ = oper; }

    private:
        int fd_;
        std::string nickname_;
        std::string username_;
        std::string realname_;
        int isOper_;
};
