#include <string>
#include <list>
#include <set>

class Client;

class Channel
{
    public:

        Channel() {}
        Channel(std::string& name) : name_(name), maxUsers_(1000), isInvite_(false), isTopicOper_(false), passcode_("none") {}

        void addMember(Client* client)
        {
            members_.insert(client);
        }

        bool removeMember(Client* client)
        {
            return members_.erase(client) > 0;
        }

        bool isMember(Client *client)
        {
            if (members_.find(client) != members_.end())
                return true;
            return false;
        }

        std::set<Client*> getMembers()
        {
            return members_;
        }

        void setTopic(const std::string& topic)
        {
            topic_ = topic;
        }

        std::string getTopic() const
        {
            return topic_;
        }

        bool getInvite()
        {
            return isInvite_;
        }

        void setInvite(bool setting)
        { 
            isInvite_ = setting;
        }

        size_t getMaxUsers()
        {
            return maxUsers_;
        }

        void setMaxUsers(size_t newMax)
        {
            maxUsers_ = newMax;
        }

        void removeMaxUsers()
        {
            maxUsers_ = 1000;
        }

        bool getTopicOper()
        {
            return isTopicOper_;
        }

        void setTopicOper(bool setting)
        {
            isTopicOper_ = setting;
        }

        std::string getPasscode()
        {
            return passcode_;
        }

        void setPasscode(std::string key)
        {
            passcode_ = key;
        }

        void removePasscode()
        {
            passcode_ = "none";
        }


    private:
        std::string name_;
        std::string topic_;
        std::set<Client*> members_;
        size_t maxUsers_;
        bool isInvite_;
        bool isTopicOper_;
        std::string passcode_;
};
