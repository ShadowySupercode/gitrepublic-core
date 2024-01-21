#include <string>
#include <vector>
#include <nlohmann/json.hpp>

#include "nostr.h"

using std::string;
using std::vector;
using nlohmann::json;

namespace nostr
{
    // TODO: Add null checking to seralization and deserialization methods.
    struct Event
    {
        string id; ///< SHA-256 hash of the event data.
        string pubkey; ///< Public key of the event creator.
        string created_at; ///< Unix timestamp of the event creation.
        int kind; ///< Event kind.
        vector<vector<string>> tags; ///< Arbitrary event metadata.
        string content; ///< Event content.
        string sig; ///< Event signature created with the private key of the event creator.

        string serialize()
        {
            json j = {
                {"id", id},
                {"pubkey", pubkey},
                {"created_at", created_at},
                {"kind", kind},
                {"tags", tags},
                {"content", content},
                {"sig", sig}
            };
            return j.dump();
        }

        void deserialize(string jsonStr)
        {
            json j = json::parse(jsonStr);
            id = j["id"];
            pubkey = j["pubkey"];
            created_at = j["created_at"];
            kind = j["kind"];
            tags = j["tags"];
            content = j["content"];
            sig = j["sig"];
        }
    };
}
