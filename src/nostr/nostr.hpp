#pragma once

#include <string>
#include <vector>

#include <nlohmann/json.hpp>
#include <plog/Log.h>
#include <websocketpp/client.hpp>

#include "types/event.hpp"

namespace nostr
{
    struct Event;

    typedef std::vector<std::string> RelayList;

    class NostrUtils
    {
    public:
        NostrUtils(plog::IAppender* appender);
        NostrUtils(plog::IAppender* appender, RelayList relays);
        ~NostrUtils();
    
        /**
         * @brief Opens connections to the default Nostr relays of the instance, as specified in
         * the constructor.
         * @return A list of the relay URLs to which connections were successfully opened.
         */
        RelayList openRelayConnections();

        /**
         * @brief Opens connections to the specified Nostr relays.
         * @returns A list of the relay URLs to which connections were successfully opened.
         */
        RelayList openRelayConnections(RelayList relays);

        /**
         * @brief Closes all open relay connections.
         */
        void closeRelayConnections();

        /**
         * @brief Closes any open connections to the specified Nostr relays.
         */
        void closeRelayConnections(RelayList relays);
        
        /**
         * @brief Publishes a Nostr event to all open relay connections.
         * @returns A list of the relay URLs to which the event was successfully published.
        */
        RelayList publishEvent(Event event);

    private:
        /**
         * @brief Determines which of the given relays are currently connected.
         * @returns A list of the URIs of currently-open relay connections from the given list.
         */
        RelayList getConnectedRelays(RelayList relays);

        /**
         * @brief Determines which of the given relays are not currently connected.
         * @returns A list of the URIs of currently-unconnected relays from the given list.
         */
        RelayList getUnconnectedRelays(RelayList relays);

        /**
         * @brief Gets the connection handle for the given relay.
         * @returns The connection handle, if found.
         */
        websocketpp::connection_hdl getConnectionHandle(std::string relay);

        /**
         * @brief Gets the connection handles for open connections from the given list.
         * @returns A list of connection handle pointers.
         */
        std::vector<websocketpp::connection_hdl> getConnectionHandles(RelayList relays);

        /**
         * @brief Determines whether the given relay is currently connected.
         * @returns True if the relay is connected, false otherwise.
         */
        bool isConnected(std::string relay);

        /**
         * @brief Removes the given relay from the instance's list of active relays.
         */
        void eraseActiveRelay(std::string relay);

        /**
         * @brief Removes the connection handle for the given relay from the instance's map of 
         * connection handles.
         */
        void eraseConnectionHandle(std::string relay);

        /**
         * @brief Opens a connection to the given relay.
         */
        void openConnection(std::string relay);

        /**
         * @brief Closes the connection to the given relay.
         */
        void closeConnection(std::string relay);

        /**
         * @brief Publishes the given event to the given relay.
         * @returns A tuple indicating the relay URL and whether the event was successfully
         * published.
         */
        std::tuple<std::string, bool> sendEvent(std::string relay, Event event);
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
        NostrClient(RelayList relays);

        /**
         * @brief Destructor.
         */
        ~NostrClient();

        /**
         * @brief Fetches the list of preferred Nostr relays.
         * @return The current preferred relay list.
         */
        RelayList getRelays();

        /**
         * @brief Sets the list of preferred Nostr relays.
         * @remark This method overrides the previous relay list.
         */
        void setRelays(RelayList relays);

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
        bool publishCommitEvents(RelayList commitIds, RelayList hubs);

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
        void deleteCommitEvents(RelayList commitIds);

    private:
        RelayList relays; ///< List of preferred Nostr relays.
    };
}
