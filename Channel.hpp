#include <string>
#include <list>
#include <set>

class Client;

class Channel
{
    public:

        Channel() {}
        Channel(std::string& name) : name_(name), maxNumber(1000), isInvite(false) {}

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

        void setMode(char mode, bool enable)
        {
            if (enable)
                modes_.insert(mode);
            else
                modes_.erase(mode);
        }

        bool isModeSet(char mode) const
        {
            return modes_.find(mode) != modes_.end();
        }

    private:
        std::string name_;
        std::string topic_;
        std::set<Client*> members_;
        std::set<char> modes_;
        int maxNumber;
        bool isInvite;
};
