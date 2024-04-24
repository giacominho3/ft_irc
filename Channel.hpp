#include <string>
#include <unordered_set>
#include <unordered_map>

class Client;

class Channel
{
    public:

        Channel() {};
        Channel(std::string& name) : name_(name) {}

        void addMember(Client* client)
        {
            members_.insert(client);
        }

        bool removeMember(Client* client)
        {
            return members_.erase(client) > 0;
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
        std::unordered_set<Client*> members_;
        std::unordered_set<char> modes_;
};
