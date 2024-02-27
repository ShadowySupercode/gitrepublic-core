#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <plog/Appenders/ConsoleAppender.h>
#include <plog/Formatters/TxtFormatter.h>
#include <websocketpp/client.hpp>

#include <client/web_socket_client.hpp>
#include <nostr/nostr.hpp>

using std::lock_guard;
using std::make_shared;
using std::mutex;
using std::shared_ptr;
using std::string;
using std::unordered_map;
using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

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

TEST_F(NostrServiceTest, Constructor_InitializesService_WithNoDefaultRelays)
{
    MockWebSocketClient testClient;

    auto nostrService = new nostr::NostrService(testAppender.get(), &testClient);
    auto defaultRelays = nostrService->defaultRelays();
    auto activeRelays = nostrService->activeRelays();

    ASSERT_EQ(defaultRelays.size(), 0);
    ASSERT_EQ(activeRelays.size(), 0);
};

TEST_F(NostrServiceTest, Constructor_InitializesService_WithProvidedDefaultRelays)
{
    MockWebSocketClient testClient;

    auto nostrService = new nostr::NostrService(testAppender.get(), &testClient, defaultTestRelays);
    auto defaultRelays = nostrService->defaultRelays();
    auto activeRelays = nostrService->activeRelays();

    ASSERT_EQ(defaultRelays.size(), defaultTestRelays.size());
    for (auto relay : defaultRelays)
    {
        ASSERT_NE(find(defaultTestRelays.begin(), defaultTestRelays.end(), relay), defaultTestRelays.end());
    }
    ASSERT_EQ(activeRelays.size(), 0);
};

TEST_F(NostrServiceTest, Destructor_StopsClient)
{
    MockWebSocketClient testClient;
    EXPECT_CALL(testClient, start()).Times(1);

    auto nostrService = new nostr::NostrService(testAppender.get(), &testClient);
};

TEST_F(NostrServiceTest, OpenRelayConnections_OpensConnections_ToDefaultRelays)
{
    MockWebSocketClient testClient;

    mutex connectionStatusMutex;
    auto connectionStatus = make_shared<unordered_map<string, bool>>();
    connectionStatus->insert({ defaultTestRelays[0], false });
    connectionStatus->insert({ defaultTestRelays[1], false });

    EXPECT_CALL(testClient, openConnection(defaultTestRelays[0])).Times(1);
    EXPECT_CALL(testClient, openConnection(defaultTestRelays[1])).Times(1);

    EXPECT_CALL(testClient, isConnected(_))
        .WillRepeatedly(Invoke([connectionStatus, &connectionStatusMutex](string uri)
        {
            lock_guard<mutex> lock(connectionStatusMutex);
            bool status = connectionStatus->at(uri);
            if (status == false)
            {
                connectionStatus->at(uri) = true;
            }
            return status;
        }));
    
    auto nostrService = new nostr::NostrService(testAppender.get(), &testClient, defaultTestRelays);
    nostrService->openRelayConnections();

    auto activeRelays = nostrService->activeRelays();
    ASSERT_EQ(activeRelays.size(), defaultTestRelays.size());
    for (auto relay : activeRelays)
    {
        ASSERT_NE(find(defaultTestRelays.begin(), defaultTestRelays.end(), relay), defaultTestRelays.end());
    }
};

TEST_F(NostrServiceTest, OpenRelayConnections_OpensConnections_ToProvidedRelays)
{
    MockWebSocketClient testClient;
    nostr::RelayList testRelays = { "wss://nos.lol" };

    mutex connectionStatusMutex;
    auto connectionStatus = make_shared<unordered_map<string, bool>>();
    connectionStatus -> insert({ testRelays[0], false });

    EXPECT_CALL(testClient, openConnection(testRelays[0])).Times(1);
    EXPECT_CALL(testClient, openConnection(defaultTestRelays[0])).Times(0);
    EXPECT_CALL(testClient, openConnection(defaultTestRelays[1])).Times(0);

    EXPECT_CALL(testClient, isConnected(_))
        .WillRepeatedly(Invoke([connectionStatus, &connectionStatusMutex](string uri)
        {
            lock_guard<mutex> lock(connectionStatusMutex);
            bool status = connectionStatus->at(uri);
            if (status == false)
            {
                connectionStatus->at(uri) = true;
            }
            return status;
        }));

    auto nostrService = new nostr::NostrService(testAppender.get(), &testClient, testRelays);
    nostrService->openRelayConnections();

    auto activeRelays = nostrService->activeRelays();
    ASSERT_EQ(activeRelays.size(), testRelays.size());
    for (auto relay : activeRelays)
    {
        ASSERT_NE(find(testRelays.begin(), testRelays.end(), relay), testRelays.end());
    }
};

TEST_F(NostrServiceTest, CloseRelayConnections_ClosesConnections_ToActiveRelays)
{
    MockWebSocketClient testClient;

    auto nostrService = new nostr::NostrService(testAppender.get(), &testClient, defaultTestRelays);
    nostrService->openRelayConnections();

    EXPECT_CALL(testClient, closeConnection(defaultTestRelays[0])).Times(0);
    EXPECT_CALL(testClient, closeConnection(defaultTestRelays[1])).Times(0);
    nostrService->closeRelayConnections();

    auto activeRelays = nostrService->activeRelays();
    ASSERT_EQ(activeRelays.size(), 0);
};
} // namespace nostr_test
