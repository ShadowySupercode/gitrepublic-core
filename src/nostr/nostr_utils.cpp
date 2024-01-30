#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <plog/Init.h>
#include <plog/Log.h>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>

#include "nostr.hpp"

using namespace std;

typedef websocketpp::client<websocketpp::config::asio_client> websocketpp_client;

// TODO: Add threading for faster connection/publishing.
namespace nostr
{
    class NostrUtils
    {
    private:
        RelayList defaultRelays;
        RelayList activeRelays;
        websocketpp_client client;
        unordered_map<string, websocketpp::connection_hdl> connectionHandles;
        mutex propertyMutex;

    public:
        NostrUtils(plog::IAppender* appender)
        {
            defaultRelays = {};

            plog::init(plog::debug, appender);

            PLOG_INFO << "Starting WebSocket client.";
            client.init_asio();
            client.start_perpetual();
        };

        NostrUtils(plog::IAppender* appender, RelayList relays)
            : NostrUtils(appender)
        {
            defaultRelays = relays;
        };

        ~NostrUtils()
        {
            PLOG_INFO << "Stopping WebSocket client.";
            client.stop_perpetual();
            client.stop();
        };
    
        RelayList openRelayConnections()
        {
            return openRelayConnections(defaultRelays);
        };

        RelayList openRelayConnections(RelayList relays)
        {
            PLOG_INFO << "Attempting to connect to Nostr relays.";
            RelayList unconnectedRelays = getUnconnectedRelays(relays);

            vector<thread> connectionThreads;
            for (string relay : unconnectedRelays)
            {
                thread connectionThread(&NostrUtils::openConnection, this, relay);
                connectionThreads.emplace_back(connectionThread);
            }

            for (thread& connectionThread : connectionThreads)
            {
                connectionThread.join();
            }

            int targetCount = relays.size();
            int activeCount = activeRelays.size();
            PLOG_INFO << "Connected to " << activeCount << "/" << targetCount << " target relays.";
        };

        void closeRelayConnections()
        {
            closeRelayConnections(activeRelays);
        };

        void closeRelayConnections(RelayList relays)
        {
            PLOG_INFO << "Disconnecting from Nostr relays.";
            RelayList connectedRelays = getConnectedRelays(relays);

            vector<thread> disconnectionThreads;
            for (string relay : connectedRelays)
            {
                thread disconnectionThread(&NostrUtils::closeConnection, this, relay);
                disconnectionThreads.emplace_back(disconnectionThread);
            }

            for (thread& disconnectionThread : disconnectionThreads)
            {
                disconnectionThread.join();
            }
        };

        RelayList publishEvent(Event event)
        {
            // TODO: Add validation function.

            RelayList successfulRelays;

            PLOG_INFO << "Attempting to publish event to Nostr relays.";
            for (string relay : activeRelays)
            {
                error_code error;
                string jsonBlob = event.serialize();
                client.send(
                    connectionHandles[relay],
                    jsonBlob,
                    websocketpp::frame::opcode::text,
                    error);

                if (error.value() == -1)    
                {
                    PLOG_ERROR << "Error publishing event to relay " << relay << ": " << error.message();
                    continue;
                }

                successfulRelays.push_back(relay);
            }

            int targetCount = activeRelays.size();
            int successfulCount = successfulRelays.size();
            PLOG_INFO << "Published event to " << successfulCount << "/" << targetCount << " target relays.";

            return successfulRelays;
        };

    private:
        RelayList getConnectedRelays(RelayList relays)
        {
            RelayList connectedRelays;
            for (string relay : relays)
            {
                auto it = find(activeRelays.begin(), activeRelays.end(), relay);
                if (it != activeRelays.end()) // If the relay is in activeRelays
                {
                    connectedRelays.push_back(relay);
                }
            }
            return connectedRelays;
        };

        RelayList getUnconnectedRelays(RelayList relays)
        {
            RelayList unconnectedRelays;
            for (string relay : relays)
            {
                auto it = find(activeRelays.begin(), activeRelays.end(), relay);
                if (it == activeRelays.end()) // If the relay is not in activeRelays
                {
                    unconnectedRelays.push_back(relay);
                }
            }
            return unconnectedRelays;
        };

        websocketpp::connection_hdl getConnectionHandle(string relay)
        {
            auto it = connectionHandles.find(relay);
            if (it != connectionHandles.end()) // If the relay is in connectionHandles
            {
                return connectionHandles[relay];
            }
        };

        vector<websocketpp::connection_hdl> getConnectionHandles(RelayList relays)
        {
            vector<websocketpp::connection_hdl> handles;
            for (string relay : relays)
            {
                auto it = connectionHandles.find(relay);
                if (it != connectionHandles.end()) // If the relay is in connectionHandles
                {
                    handles.push_back(connectionHandles[relay]);
                }
            }
            return handles;
        };

        bool isConnected(string relay)
        {
            auto it = find(activeRelays.begin(), activeRelays.end(), relay);
            if (it != activeRelays.end()) // If the relay is in activeRelays
            {
                return true;
            }
            return false;
        };

        void eraseActiveRelay(string relay)
        {
            auto it = find(activeRelays.begin(), activeRelays.end(), relay);
            if (it != activeRelays.end()) // If the relay is in activeRelays
            {
                activeRelays.erase(it);
            }
        };

        void eraseConnectionHandle(string relay)
        {
            auto it = connectionHandles.find(relay);
            if (it != connectionHandles.end()) // If the relay is in connectionHandles
            {
                connectionHandles.erase(it);
            }
        };

        void openConnection(string relay)
        {
            error_code error;
            websocketpp_client::connection_ptr connection = client.get_connection(relay, error);

            if (error.value() == -1)    
            {
                PLOG_ERROR << "Error connecting to relay " << relay << ": " << error.message();
            }

            // Configure the connection here via the connection pointer.
            connection->set_fail_handler([this, relay](auto handle) {
                PLOG_ERROR << "Error connecting to relay " << relay << ": Handshake failed.";
                if (isConnected(relay))
                {
                    lock_guard<mutex> lock(propertyMutex);
                    eraseActiveRelay(relay);
                }
            });

            lock_guard<mutex> lock(propertyMutex);
            connectionHandles[relay] = connection->get_handle();
            client.connect(connection);
            activeRelays.push_back(relay);
        };

        void closeConnection(string relay)
        {
            websocketpp::connection_hdl handle = getConnectionHandle(relay);
            client.close(
                handle,
                websocketpp::close::status::going_away,
                "Client requested close.");
            
            lock_guard<mutex> lock(propertyMutex);
            eraseActiveRelay(relay);
            eraseConnectionHandle(relay);
        };
    };
}
