#include <plog/Init.h>
#include <plog/Log.h>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>

#include "nostr.hpp"

using namespace std;

namespace nostr
{
    NostrUtils::NostrUtils(plog::IAppender* appender)
    {
        defaultRelays = {};

        plog::init(plog::debug, appender);

        PLOG_INFO << "Starting WebSocket client.";
        this->client.init_asio();
        this->client.start_perpetual();
    };

    NostrUtils::NostrUtils(plog::IAppender* appender, RelayList relays)
        : NostrUtils(appender)
    {
        defaultRelays = relays;
    };

    NostrUtils::~NostrUtils()
    {
        PLOG_INFO << "Stopping WebSocket client.";
        this->client.stop_perpetual();
        this->client.stop();
    };

    RelayList NostrUtils::openRelayConnections()
    {
        return openRelayConnections(defaultRelays);
    };

    RelayList NostrUtils::openRelayConnections(RelayList relays)
    {
        PLOG_INFO << "Attempting to connect to Nostr relays.";
        RelayList unconnectedRelays = getUnconnectedRelays(relays);

        vector<thread> connectionThreads;
        for (string relay : unconnectedRelays)
        {
            thread connectionThread(&NostrUtils::openConnection, this, relay);
            connectionThreads.push_back(move(connectionThread));
        }

        for (thread& connectionThread : connectionThreads)
        {
            connectionThread.join();
        }

        size_t targetCount = relays.size();
        size_t activeCount = activeRelays.size();
        PLOG_INFO << "Connected to " << activeCount << "/" << targetCount << " target relays.";

        // This property should only contain successful relays at this point.
        return activeRelays;
    };

    void NostrUtils::closeRelayConnections()
    {
        closeRelayConnections(activeRelays);
    };

    void NostrUtils::closeRelayConnections(RelayList relays)
    {
        PLOG_INFO << "Disconnecting from Nostr relays.";
        RelayList connectedRelays = getConnectedRelays(relays);

        vector<thread> disconnectionThreads;
        for (string relay : connectedRelays)
        {
            thread disconnectionThread(&NostrUtils::closeConnection, this, relay);
            disconnectionThreads.push_back(move(disconnectionThread));
        }

        for (thread& disconnectionThread : disconnectionThreads)
        {
            disconnectionThread.join();
        }
    };

    RelayList NostrUtils::publishEvent(Event event)
    {
        // TODO: Add validation function.

        RelayList successfulRelays;

        PLOG_INFO << "Attempting to publish event to Nostr relays.";

        vector<future<tuple<string, bool>>> publishFutures;
        for (string relay : activeRelays)
        {
            future<tuple<string, bool>> publishFuture = async(&NostrUtils::sendEvent, this, relay, event);
            publishFutures.push_back(move(publishFuture));
        }

        for (auto& publishFuture : publishFutures)
        {
            auto [relay, isSuccess] = publishFuture.get();
            if (isSuccess)
            {
                successfulRelays.push_back(relay);
            }
        }

        size_t targetCount = activeRelays.size();
        size_t successfulCount = successfulRelays.size();
        PLOG_INFO << "Published event to " << successfulCount << "/" << targetCount << " target relays.";

        return successfulRelays;
    };

    RelayList NostrUtils::getConnectedRelays(RelayList relays)
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

    RelayList NostrUtils::getUnconnectedRelays(RelayList relays)
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

    websocketpp::connection_hdl NostrUtils::getConnectionHandle(string relay)
    {
        websocketpp::connection_hdl handle;

        auto it = connectionHandles.find(relay);
        if (it != connectionHandles.end()) // If the relay is in connectionHandles
        {
            handle = connectionHandles[relay];
        }
        
        return handle;
    };

    vector<websocketpp::connection_hdl> NostrUtils::getConnectionHandles(RelayList relays)
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

    bool NostrUtils::isConnected(string relay)
    {
        auto it = find(activeRelays.begin(), activeRelays.end(), relay);
        if (it != activeRelays.end()) // If the relay is in activeRelays
        {
            return true;
        }
        return false;
    };

    void NostrUtils::eraseActiveRelay(string relay)
    {
        auto it = find(activeRelays.begin(), activeRelays.end(), relay);
        if (it != activeRelays.end()) // If the relay is in activeRelays
        {
            activeRelays.erase(it);
        }
    };

    void NostrUtils::eraseConnectionHandle(string relay)
    {
        auto it = connectionHandles.find(relay);
        if (it != connectionHandles.end()) // If the relay is in connectionHandles
        {
            connectionHandles.erase(it);
        }
    };

    void NostrUtils::openConnection(string relay)
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

    void NostrUtils::closeConnection(string relay)
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

    tuple<string, bool> NostrUtils::sendEvent(string relay, Event event)
    {
        error_code error;
        string jsonBlob = event.serialize();

        // Make sure the connection isn't closed from under us.
        lock_guard<mutex> lock(propertyMutex);
        client.send(
            connectionHandles[relay],
            jsonBlob,
            websocketpp::frame::opcode::text,
            error);

        if (error.value() == -1)    
        {
            PLOG_ERROR << "Error publishing event to relay " << relay << ": " << error.message();
            return make_tuple(relay, false);
        }

        return make_tuple(relay, true);
    };
}
