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
        this->_defaultRelays = {};

        plog::init(plog::debug, appender);

        PLOG_INFO << "Starting WebSocket _client.";
        this->_client.init_asio();
        this->_client.start_perpetual();
    };

    NostrUtils::NostrUtils(plog::IAppender* appender, RelayList relays)
        : NostrUtils(appender)
    {
        this->_defaultRelays = relays;
    };

    NostrUtils::~NostrUtils()
    {
        PLOG_INFO << "Stopping WebSocket _client.";
        this->_client.stop_perpetual();
        this->_client.stop();
    }

    RelayList NostrUtils::openRelayConnections()
    {
        return this->openRelayConnections(this->_defaultRelays);
    };

    RelayList NostrUtils::openRelayConnections(RelayList relays)
    {
        PLOG_INFO << "Attempting to connect to Nostr relays.";
        RelayList unconnectedRelays = this->getUnconnectedRelays(relays);

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
        size_t activeCount = this->_activeRelays.size();
        PLOG_INFO << "Connected to " << activeCount << "/" << targetCount << " target relays.";

        // This property should only contain successful relays at this point.
        return this->_activeRelays;
    };

    void NostrUtils::closeRelayConnections()
    {
        this->closeRelayConnections(this->_activeRelays);
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
        for (string relay : this->_activeRelays)
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

        size_t targetCount = this->_activeRelays.size();
        size_t successfulCount = successfulRelays.size();
        PLOG_INFO << "Published event to " << successfulCount << "/" << targetCount << " target relays.";

        return successfulRelays;
    };

    RelayList NostrUtils::getConnectedRelays(RelayList relays)
    {
        RelayList connectedRelays;
        for (string relay : relays)
        {
            auto it = find(this->_activeRelays.begin(), this->_activeRelays.end(), relay);
            if (it != this->_activeRelays.end()) // If the relay is in this->_activeRelays
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
            auto it = find(this->_activeRelays.begin(), this->_activeRelays.end(), relay);
            if (it == this->_activeRelays.end()) // If the relay is not in this->_activeRelays
            {
                unconnectedRelays.push_back(relay);
            }
        }
        return unconnectedRelays;
    };

    websocketpp::connection_hdl NostrUtils::getConnectionHandle(string relay)
    {
        websocketpp::connection_hdl handle;

        auto it = this->_connectionHandles.find(relay);
        if (it != this->_connectionHandles.end()) // If the relay is in _connectionHandles
        {
            handle = this->_connectionHandles[relay];
        }
        
        return handle;
    };

    vector<websocketpp::connection_hdl> NostrUtils::getConnectionHandles(RelayList relays)
    {
        vector<websocketpp::connection_hdl> handles;
        for (string relay : relays)
        {
            auto it = this->_connectionHandles.find(relay);
            if (it != this->_connectionHandles.end()) // If the relay is in _connectionHandles
            {
                handles.push_back(this->_connectionHandles[relay]);
            }
        }
        return handles;
    };

    bool NostrUtils::isConnected(string relay)
    {
        auto it = find(this->_activeRelays.begin(), this->_activeRelays.end(), relay);
        if (it != this->_activeRelays.end()) // If the relay is in this->_activeRelays
        {
            return true;
        }
        return false;
    };

    void NostrUtils::eraseActiveRelay(string relay)
    {
        auto it = find(this->_activeRelays.begin(), this->_activeRelays.end(), relay);
        if (it != this->_activeRelays.end()) // If the relay is in this->_activeRelays
        {
            this->_activeRelays.erase(it);
        }
    };

    void NostrUtils::eraseConnectionHandle(string relay)
    {
        auto it = this->_connectionHandles.find(relay);
        if (it != this->_connectionHandles.end()) // If the relay is in _connectionHandles
        {
            this->_connectionHandles.erase(it);
        }
    };

    void NostrUtils::openConnection(string relay)
    {
        error_code error;
        websocketpp_client::connection_ptr connection = this->_client.get_connection(relay, error);

        if (error.value() == -1)    
        {
            PLOG_ERROR << "Error connecting to relay " << relay << ": " << error.message();
        }

        // Configure the connection here via the connection pointer.
        connection->set_fail_handler([this, relay](auto handle) {
            PLOG_ERROR << "Error connecting to relay " << relay << ": Handshake failed.";
            if (isConnected(relay))
            {
                lock_guard<mutex> lock(this->_propertyMutex);
                this->eraseActiveRelay(relay);
            }
        });

        lock_guard<mutex> lock(this->_propertyMutex);
        this->_connectionHandles[relay] = connection->get_handle();
        this->_client.connect(connection);
        this->_activeRelays.push_back(relay);
    };

    void NostrUtils::closeConnection(string relay)
    {
        websocketpp::connection_hdl handle = this->getConnectionHandle(relay);
        this->_client.close(
            handle,
            websocketpp::close::status::going_away,
            "_client requested close.");
        
        lock_guard<mutex> lock(this->_propertyMutex);
        this->eraseActiveRelay(relay);
        this->eraseConnectionHandle(relay);
    };

    tuple<string, bool> NostrUtils::sendEvent(string relay, Event event)
    {
        error_code error;
        string jsonBlob = event.serialize();

        // Make sure the connection isn't closed from under us.
        lock_guard<mutex> lock(this->_propertyMutex);
        this->_client.send(
            this->_connectionHandles[relay],
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
