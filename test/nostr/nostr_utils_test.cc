#include <gtest/gtest.h>
#include <plog/Appenders/ConsoleAppender.h>
#include <plog/Formatters/TxtFormatter.h>

#include <nostr/nostr.hpp>

using namespace std;

TEST(NostrUtilsTest, ConstructorSetsEmptyDefaultRelays)
{
    nostr::NostrUtils* nostrUtils = new nostr::NostrUtils(new plog::ConsoleAppender<plog::TxtFormatter>());
    ASSERT_EQ(nostrUtils->defaultRelays().size(), 0);
    delete nostrUtils;
};

TEST(NostrUtilsTest, ConstructorSetsDefaultRelays)
{
    nostr::RelayList relays = { "wss://relay.damus.io", "wss://nostr.thesamecat.io" };
    nostr::NostrUtils* nostrUtils = new nostr::NostrUtils(new plog::ConsoleAppender<plog::TxtFormatter>(), relays);
    ASSERT_EQ(nostrUtils->defaultRelays().size(), 2);
    delete nostrUtils;
};

class NostrUtilsTest : public testing::Test
{
protected:
    nostr::NostrUtils* nostrUtils;

    void SetUp() override
    {
        nostrUtils = new nostr::NostrUtils(new plog::ConsoleAppender<plog::TxtFormatter>());
    }

    void TearDown() override
    {
        delete nostrUtils;
    }
};
