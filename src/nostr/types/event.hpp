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
}
