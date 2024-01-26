#include <string>
#include <thread>
#include <vector>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>

#include "nostr.hpp"

using namespace std;

typedef websocketpp::client<websocketpp::config::asio_client> websocketpp_client;

namespace nostr
{
    class NostrUtils
    {
    private:
        vector<string> defaultRelays;
        vector<string> activeRelays;
        websocketpp_client client;
        unordered_map<string, websocketpp::connection_hdl> connectionHandles;

    public:
        NostrUtils()
        {
            defaultRelays = {};

            client.init_asio();
            client.start_perpetual();
        };

        NostrUtils(vector<string> relays) : NostrUtils()
        {
            defaultRelays = relays;
        };

        ~NostrUtils()
        {
            client.stop_perpetual();
            client.stop();
        };
    
    protected:
        vector<string> openRelayConnections(vector<string>* relays)
        {
            vector<string> targetRelays;
            vector<string> successfulRelays;

            if (relays == nullptr)
            {
                targetRelays = defaultRelays;
            }
            else if (relays->empty())
            {
                targetRelays = defaultRelays;
            }
            else 
            {
                targetRelays = *relays;
            }

            for (string relay : targetRelays)
            {
                // Skip relays that are already connected.
                auto it = find(activeRelays.begin(), activeRelays.end(), relay);
                if (it != activeRelays.end())
                {
                    continue;
                }

                error_code error;
                websocketpp_client::connection_ptr connection = client.get_connection(relay, error);

                if (error.value() == -1)    
                {
                    // TODO: Log error.
                }

                // Configure the connection here via the connection pointer.
                // TODO: Set handlers.

                connectionHandles[relay] = connection->get_handle();
                client.connect(connection);
                activeRelays.push_back(relay);
            }
        };

        void closeRelayConnections(vector<string>* relays)
        {
            vector<string> targetRelays;

            if (relays == nullptr)
            {
                targetRelays = activeRelays;
            }
            else if (relays->empty())
            {
                targetRelays = activeRelays;
            }
            else 
            {
                targetRelays = *relays;
            }

            for (string relay : targetRelays)
            {
                // Skip relays that are not connected.
                auto it = find(activeRelays.begin(), activeRelays.end(), relay);
                if (it == activeRelays.end())
                {
                    continue;
                }

                client.close(
                    connectionHandles[relay],
                    websocketpp::close::status::going_away, "Client requested close.");
                activeRelays.erase(it);
            }
        };
}
