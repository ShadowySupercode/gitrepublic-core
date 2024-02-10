#include <gtest/gtest.h>
#include <plog/Appenders/ConsoleAppender.h>
#include <plog/Formatters/TxtFormatter.h>

#include <nostr/nostr.hpp>

using namespace std;

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
