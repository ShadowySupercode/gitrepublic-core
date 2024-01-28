#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace nostr
{
    // TODO: Add null checking to seralization and deserialization methods.
    /**
     * @brief A Nostr event.
     * @remark All data transmitted over the Nostr protocol is encoded in JSON blobs.  This struct
     * is common to every Nostr event kind.  The significance of each event is determined by the
     * `tags` and `content` fields.
    */
    struct Event
    {
        std::string id; ///< SHA-256 hash of the event data.
        std::string pubkey; ///< Public key of the event creator.
        std::string created_at; ///< Unix timestamp of the event creation.
        int kind; ///< Event kind.
        std::vector<std::vector<std::string>> tags; ///< Arbitrary event metadata.
        std::string content; ///< Event content.
        std::string sig; ///< Event signature created with the private key of the event creator.

        nlohmann::json serialize()
        {
            nlohmann::json j = {
                {"id", id},
                {"pubkey", pubkey},
                {"created_at", created_at},
                {"kind", kind},
                {"tags", tags},
                {"content", content},
                {"sig", sig}
            };
            return j.dump();
        };

        void deserialize(std::string jsonStr)
        {
            nlohmann::json j = nlohmann::json::parse(jsonStr);
            id = j["id"];
            pubkey = j["pubkey"];
            created_at = j["created_at"];
            kind = j["kind"];
            tags = j["tags"];
            content = j["content"];
            sig = j["sig"];
        };
    };

    class NostrUtils
    {
    public:
        NostrUtils();
        NostrUtils(std::vector<std::string> relays);
        ~NostrUtils();
    
    protected:
        /**
         * @brief Opens connections to the specified Nostr relays.  Defaults to the relay list
         * provided to the class constructor.
         * @returns A list of the relay URLs to which connections were successfully opened.
         */
        std::vector<std::string> openRelayConnections(std::vector<std::string> relays);

        /**
         * @brief Closes any open connections to the specified Nostr relays.  Closes all open relay
         * connections if no relays are specified.
         */
        void closeRelayConnections(std::vector<std::string> relays);
        
        /**
         * @brief Publishes a Nostr event to all open relay connections.
         * @returns A list of the relay URLs to which the event was successfully published.
        */
        std::vector<std::string> publishEvent(Event event);
    };

    class NostrClient : protected NostrUtils
    {
    public:
        /**
         * @brief Default constructor.  Creates a NostrClient instance with no preferred relays.
         * @remark Set the preferred relays with setRelays() or addRelay().
         */
        NostrClient();

        /**
         * @brief Creates a NostrClient instance with the specified preferred relays.
        */
        NostrClient(std::vector<std::string> relays);

        /**
         * @brief Destructor.
         */
        ~NostrClient();

        /**
         * @brief Fetches the list of preferred Nostr relays.
         * @return The current preferred relay list.
         */
        std::vector<std::string> getRelays();

        /**
         * @brief Sets the list of preferred Nostr relays.
         * @remark This method overrides the previous relay list.
         */
        void setRelays(std::vector<std::string> relays);

        /**
         * @brief Adds a Nostr relay to the list of preferred relays.
         */
        void addRelay(std::string relay);

        /**
         * @brief Removes a Nostr relay from the list of preferred relays.
         */
        void removeRelay(std::string relay);

        /**
         * @brief Creates Nostr events to authenticate each commit, and publishes them to the
         * preferred relays.
         * @param commitIds List of Git commit IDs to publish.  Each commit ID will be used to
         * uniquely identify the Nostr event that authenticates it.
         * @param hubs List of Git remotes to which the commit has been pushed.
         * @return True if all events were published successfully, false otherwise.
         */
        bool publishCommitEvents(std::vector<std::string> commitIds, std::vector<std::string> hubs);

        /**
         * @brief Queries the preferred Nostr relays for commit events that authenticate
         * commits ahead of the specified head.
         * @param headId Git commit ID of the head of the local branch copy.
         * @param repo The name of the Git repository.
         * @param branch The name of the checked-out branch.
         * @return A list of events authenticating commits ahead of the specified head.
         */
        std::vector<Event> fetchCommitEvents(std::string headId, std::string repo, std::string branch);

        /**
         * @brief Requests deletion of the events authenticating the specified commits on the
         * preferred relays.
         * @param commitIds List of Git commit IDs.  The client will request deletion of the
         * events authenticating those commits.
         * @remark Deletion is not guaranteed, as it depends on each relay's event storage policy.
         */
        void deleteCommitEvents(std::vector<std::string> commitIds);

    private:
        std::vector<std::string> relays; ///< List of preferred Nostr relays.
    };
}
