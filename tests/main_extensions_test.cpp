#include <gtest/gtest.h>

#include <string>

#include "extensions.hpp"
#include "public_api.hpp"

using std::string;

TEST(ExtensionsTests, SplitStringBasic) {
    vector<string> tokens;
    EXPECT_TRUE(splitString("a b c", " ", tokens));
    ASSERT_EQ(tokens.size(), 3u);
    EXPECT_EQ(tokens[0], "a");
    EXPECT_EQ(tokens[1], "b");
    EXPECT_EQ(tokens[2], "c");
}

TEST(ExtensionsTests, RegexMatch) {
    EXPECT_TRUE(regexMatch("host=1.2.3.4", "host=[^\\s]+"));
    EXPECT_FALSE(regexMatch("nohost", "host=[^\\s]+"));
}

TEST(ExtensionsTests, RoundNumber) {
    EXPECT_DOUBLE_EQ(roundNumber(1.23456, 2), 1.24);
    EXPECT_DOUBLE_EQ(roundNumber(1.2, 0), 2.0);
}

TEST(ExtensionsTests, HumanReadableBytes) {
    EXPECT_EQ(getHumanReadableBytes(512), "512B");
    EXPECT_TRUE(getHumanReadableBytes(2048).find("KiB") != string::npos);
}

TEST(ExtensionsTests, HumanReadableTime) {
    EXPECT_EQ(getHumanReadableTime(30), "30s");
    // 90 seconds should include minutes
    EXPECT_TRUE(getHumanReadableTime(90).find("m") != string::npos);
}

TEST(ExtensionsTests, SpacerAndTrim) {
    EXPECT_EQ(getSpacerString(10, 4).size(), 3u);

    string s = "  hello  ";
    trimStart(s, " ");
    EXPECT_EQ(s, "hello  ");
    trimEnd(s, " ");
    EXPECT_EQ(s, "hello");
    s = "  foo  ";
    trim(s, " ");
    EXPECT_EQ(s, "foo");
}

TEST(PublicApiTests, NormalizeIp) {
    EXPECT_EQ(normalizeIp("::ffff:192.0.2.1"), "192.0.2.1");
    EXPECT_EQ(normalizeIp("2001:db8::1"), "2001:db8::1");
}

TEST(PublicApiTests, GetHostFromIpFallback) {
    // Default build configuration has DNS lookup disabled, so it should return the IP string.
    EXPECT_EQ(getHostFromIp("192.0.2.5"), "192.0.2.5");
    EXPECT_EQ(getHostFromIp("::ffff:203.0.113.7"), "::ffff:203.0.113.7");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
