#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

#include "report_cache.hpp"

using ::testing::HasSubstr;
using ::testing::Not;

namespace {

std::filesystem::path makeUniqueTempDir() {
    const auto base = std::filesystem::temp_directory_path();
    const auto dir = base / ("endlessh-report-tests-" + std::to_string(::getpid()) + "-" + std::to_string(std::time(nullptr)));
    std::filesystem::create_directories(dir);
    return dir;
}

} // namespace

TEST(ReportCache, MissingFileLoadIsOk) {
    const auto tempDir = makeUniqueTempDir();
    const auto cachePath = (tempDir / "missing-cache.tsv").string();

    ReportCache cache(cachePath);
    std::string err;
    EXPECT_TRUE(cache.load(err));
}

TEST(ReportCache, SaveAndReloadRoundTrip) {
    const auto tempDir = makeUniqueTempDir();
    const auto cachePath = (tempDir / "ip-cache.tsv").string();

    ReportCache cache(cachePath);

    IpStatsSnapshot snap;
    snap.accepted = 10;
    snap.closed = 9;
    snap.secondsWasted = 123.456;
    snap.bytesSent = 654321;

    cache.update("1.2.3.4", snap);

    std::string err;
    ASSERT_TRUE(cache.save(err)) << err;

    ReportCache reloaded(cachePath);
    ASSERT_TRUE(reloaded.load(err)) << err;

    EXPECT_TRUE(reloaded.hasUnchanged("1.2.3.4", snap));

    IpStatsSnapshot changed = snap;
    changed.accepted++;
    EXPECT_FALSE(reloaded.hasUnchanged("1.2.3.4", changed));
}

TEST(IpStatsSnapshot, OptionalFieldsComparedWhenPresentInCurrent) {
    IpStatsSnapshot current;
    current.accepted = 1;
    current.closed = 1;
    current.secondsWasted = 10.0;

    IpStatsSnapshot cached;
    cached.accepted = 1;
    cached.closed = 1;
    // cached.secondsWasted is missing

    EXPECT_FALSE(current.equalsRelevant(cached));
}

TEST(IpStatsSnapshot, OptionalFieldsIgnoredWhenMissingInCurrent) {
    IpStatsSnapshot current;
    current.accepted = 1;
    current.closed = 1;
    // current has no optional values

    IpStatsSnapshot cached;
    cached.accepted = 1;
    cached.closed = 1;
    cached.secondsWasted = 999.0;
    cached.bytesSent = 42;

    EXPECT_TRUE(current.equalsRelevant(cached));
}

TEST(ReportCache, DefaultCachePathPrefersXdgCacheHome) {
    const auto tempDir = makeUniqueTempDir();

    // Save old env
    const char* oldXdg = std::getenv("XDG_CACHE_HOME");
    const std::string oldXdgStr = oldXdg ? oldXdg : "";

    ::setenv("XDG_CACHE_HOME", tempDir.string().c_str(), 1);

    const auto path = ReportCache::defaultCachePath();
    EXPECT_THAT(path, HasSubstr(tempDir.string()));
    EXPECT_THAT(path, HasSubstr("endlessh-report"));

    // restore
    if (!oldXdgStr.empty()) {
        ::setenv("XDG_CACHE_HOME", oldXdgStr.c_str(), 1);
    } else {
        ::unsetenv("XDG_CACHE_HOME");
    }
}
