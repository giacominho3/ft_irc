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

        void setInvite()
        {
            isInvite_ = !isInvite_;
        }

        int getMaxUsers()
        {
            return maxUsers_;
        }

        void setMaxUsers(int newMax)
        {
            maxUsers_ = newMax;
        }


    private:
        std::string name_;
        std::string topic_;
        std::set<Client*> members_;
        int maxUsers_;
        bool isInvite_;
        bool isTopicOper_;
        std::string passcode_;
};
