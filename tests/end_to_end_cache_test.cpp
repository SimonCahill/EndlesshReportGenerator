#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using ::testing::HasSubstr;
using ::testing::Not;

namespace {

std::filesystem::path makeUniqueTempDir() {
    const auto base = std::filesystem::temp_directory_path();
    const auto dir = base / ("endlessh-report-e2e-" + std::to_string(::getpid()) + "-" + std::to_string(std::time(nullptr)));
    std::filesystem::create_directories(dir);
    return dir;
}

void writeFile(const std::filesystem::path& p, const std::string& content) {
    std::ofstream out(p, std::ios::trunc);
    out << content;
}

std::string shellEscape(const std::string& s) {
    // Wrap in single quotes and escape embedded single quotes.
    std::string out;
    out.reserve(s.size() + 2);
    out.push_back('\'');
    for (char c : s) {
        if (c == '\'') {
            out += "'\\''";
        } else {
            out.push_back(c);
        }
    }
    out.push_back('\'');
    return out;
}

std::string runCommandCapture(const std::vector<std::string>& args) {
    std::string cmd;
    for (size_t i = 0; i < args.size(); ++i) {
        if (i != 0) cmd.push_back(' ');
        cmd += shellEscape(args[i]);
    }
    cmd += " 2>&1";

    FILE* pipe = ::popen(cmd.c_str(), "r");
    if (!pipe) {
        return "";
    }

    std::string output;
    char buffer[4096];
    while (true) {
        const size_t n = std::fread(buffer, 1, sizeof(buffer), pipe);
        if (n > 0) output.append(buffer, n);
        if (n < sizeof(buffer)) break;
    }

    ::pclose(pipe);
    return output;
}

std::string binaryPath() {
#ifndef ENDLESSH_REPORT_BINARY_PATH
    return "";
#else
    return std::string(ENDLESSH_REPORT_BINARY_PATH);
#endif
}

} // namespace

TEST(EndToEnd, AbuseIpdbUsesCacheToSkipUnchangedIps) {
    const auto bin = binaryPath();
    ASSERT_FALSE(bin.empty());
    ASSERT_TRUE(std::filesystem::exists(bin));

    const auto dir = makeUniqueTempDir();
    const auto logPath = dir / "syslog";
    const auto cachePath = dir / "cache.tsv";

    const std::string log =
        "Jan  1 00:00:00 host endlessh[1]: ACCEPT host=::ffff:1.2.3.4 port=22\n"
        "Jan  1 00:00:01 host endlessh[1]: CLOSE host=::ffff:1.2.3.4 port=22 time=1.0 bytes=100\n"
        "Jan  1 00:00:02 host endlessh[1]: ACCEPT host=::ffff:5.6.7.8 port=22\n"
        "Jan  1 00:00:03 host endlessh[1]: CLOSE host=::ffff:5.6.7.8 port=22 time=2.0 bytes=200\n";

    writeFile(logPath, log);

    auto out1 = runCommandCapture({
        bin,
        "--abuse-ipdb",
        "--syslog", logPath.string(),
        "--cache-file", cachePath.string()
    });

    EXPECT_THAT(out1, HasSubstr("IP,Categories,ReportDate,Comment"));
    EXPECT_THAT(out1, HasSubstr("1.2.3.4"));
    EXPECT_THAT(out1, HasSubstr("5.6.7.8"));

    auto out2 = runCommandCapture({
        bin,
        "--abuse-ipdb",
        "--syslog", logPath.string(),
        "--cache-file", cachePath.string()
    });

    EXPECT_THAT(out2, HasSubstr("IP,Categories,ReportDate,Comment"));
    EXPECT_THAT(out2, Not(HasSubstr("1.2.3.4")));
    EXPECT_THAT(out2, Not(HasSubstr("5.6.7.8")));

    // Now bump stats for one IP and ensure it is emitted again.
    const std::string logChanged = log +
        "Jan  1 00:00:04 host endlessh[1]: ACCEPT host=::ffff:1.2.3.4 port=22\n";
    writeFile(logPath, logChanged);

    auto out3 = runCommandCapture({
        bin,
        "--abuse-ipdb",
        "--syslog", logPath.string(),
        "--cache-file", cachePath.string()
    });

    EXPECT_THAT(out3, HasSubstr("1.2.3.4"));
    EXPECT_THAT(out3, Not(HasSubstr("5.6.7.8")));
}

TEST(EndToEnd, StrictSkipsUnchangedIpsInMarkdownListing) {
    const auto bin = binaryPath();
    ASSERT_FALSE(bin.empty());
    ASSERT_TRUE(std::filesystem::exists(bin));

    const auto dir = makeUniqueTempDir();
    const auto logPath = dir / "syslog";
    const auto cachePath = dir / "cache.tsv";

    const std::string log =
        "Jan  1 00:00:00 host endlessh[1]: ACCEPT host=::ffff:9.9.9.9 port=22\n"
        "Jan  1 00:00:01 host endlessh[1]: CLOSE host=::ffff:9.9.9.9 port=22 time=1.0 bytes=100\n";

    writeFile(logPath, log);

    auto out1 = runCommandCapture({
        bin,
        "--no-cn-stats",
        "--strict",
        "--syslog", logPath.string(),
        "--cache-file", cachePath.string()
    });

    EXPECT_THAT(out1, HasSubstr("# Statistics per IP"));
    EXPECT_THAT(out1, HasSubstr("9.9.9.9"));

    auto out2 = runCommandCapture({
        bin,
        "--no-cn-stats",
        "--strict",
        "--syslog", logPath.string(),
        "--cache-file", cachePath.string()
    });

    EXPECT_THAT(out2, HasSubstr("# Statistics per IP"));
    EXPECT_THAT(out2, Not(HasSubstr("9.9.9.9")));
}
