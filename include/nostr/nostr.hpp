#pragma once

#include <future>
#include <mutex>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

#include <nlohmann/json.hpp>
#include <plog/Log.h>
#include <websocketpp/client.hpp>

#include "event.hpp"

namespace nostr
{
    struct Event;

    typedef std::vector<std::string> RelayList;

    typedef websocketpp::client<websocketpp::config::asio_client> websocketpp_client;

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
        RelayList defaultRelays;
        RelayList activeRelays;
        websocketpp_client client;
        std::unordered_map<std::string, websocketpp::connection_hdl> connectionHandles;
        std::mutex propertyMutex;

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
}