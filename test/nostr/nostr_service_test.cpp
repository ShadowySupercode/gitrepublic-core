#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <plog/Appenders/ConsoleAppender.h>
#include <plog/Formatters/TxtFormatter.h>
#include <websocketpp/client.hpp>

#include <client/web_socket_client.hpp>
#include <nostr/nostr.hpp>

using namespace std;

namespace nostr_test
{
class MockWebSocketClient : public client::IWebSocketClient {
public:
    MOCK_METHOD(void, start, (), (override));
    MOCK_METHOD(void, stop, (), (override));
    MOCK_METHOD(void, openConnection, (std::string uri), (override));
    MOCK_METHOD(bool, isConnected, (std::string uri), (override));
    MOCK_METHOD((std::tuple<std::string, bool>), send, (std::string message, std::string uri), (override));
    MOCK_METHOD(void, closeConnection, (std::string uri), (override));
};

class NostrServiceTest : public testing::Test
{
public:
    inline static const nostr::RelayList defaultTestRelays =
    {
        "wss://relay.damus.io",
        "wss://nostr.thesamecat.io"
    };

protected:
    shared_ptr<plog::ConsoleAppender<plog::TxtFormatter>> testAppender;

    void SetUp() override
    {
        testAppender = make_shared<plog::ConsoleAppender<plog::TxtFormatter>>();
    };
};

TEST_F(NostrServiceTest, Constructor_StartsClient)
{
    MockWebSocketClient testClient;
    EXPECT_CALL(testClient, start()).Times(1);

    auto nostrService = new nostr::NostrService(testAppender.get(), &testClient);
};

TEST_F(NostrServiceTest, Destructor_StopsClient)
{
    MockWebSocketClient testClient;
    EXPECT_CALL(testClient, start()).Times(1);

    auto nostrService = new nostr::NostrService(testAppender.get(), &testClient);
};
} // namespace nostr_test
