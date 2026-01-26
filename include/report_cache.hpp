/**
 * @file report_cache.hpp
 * @author Simon Cahill (simon@simonc.eu)
 * @brief Persistent per-IP statistics cache used to suppress unchanged entries.
 * @version 0.1
 * @date 2026-01-24
 */

#ifndef ENDLESSH_REPORT_INCLUDE_REPORT_CACHE_HPP
#define ENDLESSH_REPORT_INCLUDE_REPORT_CACHE_HPP

#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include <fmt/format.h>

using std::map;
using std::optional;
using std::string;
using std::vector;

struct IpStatsSnapshot {
    uint64_t accepted{0};
    uint64_t closed{0};

    // Optional fields (present when detailed stats were available at time of caching)
    optional<double> secondsWasted{};
    optional<uint64_t> bytesSent{};

    bool equalsRelevant(const IpStatsSnapshot& other) const {
        if (accepted != other.accepted) { return false; }
        if (closed != other.closed) { return false; }

        // Only compare optional fields when current snapshot has them.
        // If current has extra detail and cached does not, treat as change.
        if (secondsWasted.has_value()) {
            if (!other.secondsWasted.has_value()) { return false; }
            const double diff = std::abs(*secondsWasted - *other.secondsWasted);
            // Treat <= 0.5ms difference as unchanged to avoid floating noise.
            if (diff > 0.0005) { return false; }
        }
        if (bytesSent.has_value()) {
            if (!other.bytesSent.has_value()) { return false; }
            if (*bytesSent != *other.bytesSent) { return false; }
        }
        return true;
    }
};

class ReportCache {
public:
    explicit ReportCache(string cacheFilePath): m_cacheFilePath(std::move(cacheFilePath)) {}

    const string& cacheFilePath() const { return m_cacheFilePath; }

    bool load(string& errorOut) {
        m_entries.clear();

        std::ifstream in(m_cacheFilePath);
        if (!in.good()) {
            // Cache missing is not an error.
            return true;
        }

        string line;
        while (std::getline(in, line)) {
            if (line.empty()) { continue; }
            if (line.rfind('#', 0) == 0) { continue; }

            vector<string> tokens;
            split(line, '\t', tokens);
            // ip\taccepted\tclosed\tseconds\tbytes
            if (tokens.size() < 3) { continue; }

            IpStatsSnapshot snap;
            try {
                const auto& ip = tokens[0];
                snap.accepted = std::stoull(tokens[1]);
                snap.closed = std::stoull(tokens[2]);

                if (tokens.size() >= 4 && !tokens[3].empty() && tokens[3] != "-") {
                    snap.secondsWasted = std::stod(tokens[3]);
                }
                if (tokens.size() >= 5 && !tokens[4].empty() && tokens[4] != "-") {
                    snap.bytesSent = std::stoull(tokens[4]);
                }

                m_entries[ip] = std::move(snap);
            } catch (...) {
                // Skip malformed lines.
                continue;
            }
        }

        return true;
    }

    bool save(string& errorOut) const {
        try {
            std::filesystem::path p(m_cacheFilePath);
            if (p.has_parent_path()) {
                std::filesystem::create_directories(p.parent_path());
            }

            std::ofstream out(m_cacheFilePath, std::ios::trunc);
            if (!out.good()) {
                errorOut = fmt::format("Failed to write cache file: {0:s}", m_cacheFilePath);
                return false;
            }

            out << "# endlessh-report ip cache v1\n";
            out << "# ip\taccepted\tclosed\tsecondsWasted\tbytesSent\n";

            for (const auto& [ip, snap] : m_entries) {
                out << ip << '\t'
                    << snap.accepted << '\t'
                    << snap.closed << '\t'
                    << (snap.secondsWasted.has_value() ? fmt::format("{0:.3f}", *snap.secondsWasted) : string("-")) << '\t'
                    << (snap.bytesSent.has_value() ? fmt::format("{0:d}", *snap.bytesSent) : string("-"))
                    << "\n";
            }

            return true;
        } catch (const std::exception& ex) {
            errorOut = ex.what();
            return false;
        }
    }

    bool hasUnchanged(const string& ip, const IpStatsSnapshot& current) const {
        const auto it = m_entries.find(ip);
        if (it == m_entries.end()) { return false; }
        return current.equalsRelevant(it->second);
    }

    void update(const string& ip, const IpStatsSnapshot& current) {
        m_entries[ip] = current;
    }

    static string defaultCachePath() {
        const char* xdg = std::getenv("XDG_CACHE_HOME");
        if (xdg != nullptr && string(xdg).size() > 0) {
            return (std::filesystem::path(xdg) / "endlessh-report" / "ip-cache.tsv").string();
        }

        const char* home = std::getenv("HOME");
        if (home != nullptr && string(home).size() > 0) {
            return (std::filesystem::path(home) / ".cache" / "endlessh-report" / "ip-cache.tsv").string();
        }

        // Fallback: local cache.
        return "./endlessh-report-ip-cache.tsv";
    }

private:
    static void split(const string& in, const char delim, vector<string>& out) {
        out.clear();
        size_t start = 0;
        while (start <= in.size()) {
            const auto pos = in.find(delim, start);
            if (pos == string::npos) {
                out.push_back(in.substr(start));
                break;
            }
            out.push_back(in.substr(start, pos - start));
            start = pos + 1;
        }
    }

    string m_cacheFilePath;
    map<string, IpStatsSnapshot> m_entries;
};

#endif // ENDLESSH_REPORT_INCLUDE_REPORT_CACHE_HPP
