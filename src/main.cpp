/**
 * @file main.cpp
 * @author Simon Cahill (simon@simonc.eu)
 * @brief Parses endlessh's logs, counts the total number of connections, unique IPs and lists which IPs are still connected.
 * @version 0.1
 * @date 2022-01-07
 * 
 * @copyright Copyright (c) 2022 Simon Cahill
 */

////////////////////////////////
//  Project-Specific Includes //
////////////////////////////////
#include <date/date.h> // full path here to remain easy to compile

#include "extensions.hpp"
#include "options.hpp"
#include "version.hpp"

////////////////////////////////
//  Standard Includes (STL)   //
////////////////////////////////
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <regex.h>

#include <fmt/format.h>

using fmt::format;

using std::cout;
using std::cerr;
using std::chrono::seconds;
using std::chrono::system_clock;
using std::endl;
using std::function;
using std::ifstream;
using std::make_pair;
using std::map;
using std::pair;
using std::string;
using std::vector;

static bool    g_error = false; //!< Whether or not a fatal error occurred
static bool    g_disableAdvertisement = false; //!< Whether or not to disable advertisement (default: false)
static bool    g_printAbuseIpDbCsv = false; //!< Whether or not to output AbuseIPDB-compatible CSV data (default: false; disables markdown-compatible stats)
static bool    g_printIpStatistics = true; //!< Whether or not to print IP stats (default: true)
static bool    g_printConnectionStatistics = true; //!< Whether or not to print connection stats (default: true)
static bool    g_readFromStdIn = false; //!< Whether or not to read from stdin (default: false)
static bool    g_useDetailedInfo = false; //!< Whether or not reports should be detailed (default: false)
static string  g_logLocation = "/var/log/syslog"; //!< Default endlessh log location (default: /var/log/syslog)

/**
 * @brief Contains information about a given connection.
 */
struct ConnectionDetails {
    size_t              acceptedConnections; //!< The total amount of accepted connections
    size_t              closedConnections; //!< The total amount of closed connections

    vector<uint16_t>    usedPorts; //!< The amount of ports used.

    double              totalSecondsWasted; //!< The total seconds of bot time wasted

    size_t              totalBytesSent; //!< The total amount of bytes sent to the bots

    string              host; //!< The host trying to attack the system.

    ConnectionDetails(): acceptedConnections(0), closedConnections(0),
    usedPorts({}), totalSecondsWasted(0), totalBytesSent(0), host({}) {}
    ~ConnectionDetails() = default;
};

static int32_t                                 parseArgs(const int32_t&, char**); //!< Parses command-line arguments
static map<string, pair<uint32_t, uint32_t>>   getConnections(const vector<string>&); //!< Gets the logged connections
static string                                  getCurrentIsoTimestamp(); //!< Gets the current time as an ISO timestamp, accurate to the current second.
static vector<ConnectionDetails>               getDetailledConnections(const vector<string>&); //!< Gets a detailled list of logged connections
static vector<string>                          readEndlesshLog(); //!< Reads the log file into memory
static void                                    printConnectionStatistics(const uint32_t uniqueIps, const uint32_t totalAccepted, const uint32_t totalClosed, const double totalTimeWasted, const uint32_t totalBytesSent); //!< Print connection statistics
static void                                    printIpStatsTableHeader(); //!< Prints the markdown header for the statistics table
static void                                    printIpStats(const map<string, pair<uint32_t, uint32_t>>&, uint32_t& totalAccepted, uint32_t& totalClosed); //!< Prints the IP stats
static void                                    printDetailedIpStats(const vector<ConnectionDetails>&, uint32_t& totalAccepted, uint32_t& totalClosed); //!< Prints detailed IP stats

int main(int32_t argC, char** argV) {
    if (parseArgs(argC, argV) == 1) {
        return 0;
    }

    // Check if output is desired to be in AbuseIPDB format
    // If so, disable markdown-compatible output
    if (g_printAbuseIpDbCsv) {
        cerr << "[WARNING] Disabling markdown-compatible output tables for AbuseIPDB compatibility!" << endl;
        g_printConnectionStatistics = g_printIpStatistics = false;
    }

    // Read log file
    const auto logContents = readEndlesshLog();

    if (g_error) { return 1; }

    map<string, pair<uint32_t, uint32_t>> normalConnList;
    vector<ConnectionDetails> detailedConnList;

    if (!g_useDetailedInfo) {
        normalConnList = getConnections(logContents);
    } else {
        detailedConnList = getDetailledConnections(logContents);
    }

    uint32_t totalAcceptedConnections = 0;
    uint32_t totalClosedConnections = 0;

    if (!g_disableAdvertisement && !g_printAbuseIpDbCsv) {
        cout << "# Report generated by Endlessh Reporter at " << getCurrentIsoTimestamp() << endl;
    }

    if (g_printIpStatistics) {
        printIpStatsTableHeader();
        if (!g_useDetailedInfo) {
            printIpStats(normalConnList, totalAcceptedConnections, totalClosedConnections);
        } else {
            printDetailedIpStats(detailedConnList, totalAcceptedConnections, totalClosedConnections);
        }
        cout << endl;
    }

    if (g_printConnectionStatistics) {
        if (!g_useDetailedInfo) {
            if (totalAcceptedConnections == 0 || totalClosedConnections == 0) {
                totalAcceptedConnections = totalClosedConnections = 0; // Just make sure they're both zero
                // This is stupid. I need to refactor the living fuck out of this code...
                // It's best if I only use the detailed stats and then adjust the list accordingly
                std::for_each(normalConnList.begin(), normalConnList.end(), [&](const pair<string, pair<uint32_t, uint32_t>>& x) {
                    totalAcceptedConnections += x.second.first;
                    totalClosedConnections += x.second.second;
                });
            }
            printConnectionStatistics(normalConnList.size(), totalAcceptedConnections, totalClosedConnections, 0, 0);
        } else {
            if (totalAcceptedConnections == 0 || totalClosedConnections == 0) {
                totalAcceptedConnections = totalClosedConnections = 0; // Just make sure they're both zero
                // Again. Totally stupid. I need to refactor this. Big time.
                std::for_each(detailedConnList.begin(), detailedConnList.end(), [&](const ConnectionDetails& x) {
                    totalAcceptedConnections += x.acceptedConnections;
                    totalClosedConnections += x.closedConnections;
                });
            }

            double totalSeconds = 0;
            size_t totalBytes = 0;
            std::for_each(detailedConnList.begin(), detailedConnList.end(), [&](const ConnectionDetails& x) {
                totalBytes += x.totalBytesSent;
                totalSeconds += x.totalSecondsWasted;
            });

            printConnectionStatistics(detailedConnList.size(), totalAcceptedConnections, totalClosedConnections, totalSeconds, totalBytes);
        }
    }

    if (g_printAbuseIpDbCsv) {
        cerr << "Using categories for hacking, brute-force, sshd, port sniffing" << endl;
        auto categories = "18,14,22,15";
        auto timestamp = getCurrentIsoTimestamp();
        
        const auto advertisement = format(R"(Report generated by {0:s} v{1:s})", getLongProjectName(), getApplicationVersion());
        const auto regularCommentFmt = format(
            "{{0:s}} fell into Endlessh tarpit; {{1:d}}/{{2:d}} total connections are currently still open. {0:s}",
            g_disableAdvertisement ? string() : advertisement
        );
        const auto detailedCommentFmt = format(
            "{{0:s}} fell into Endlessh tarpit; {{1:d}}/{{2:d}} total connections are currently still open. Total time wasted: {{3:s}}. Total bytes sent by tarpit: {{4:s}}. {0:s}",
            g_disableAdvertisement ? string() : advertisement
        );

        cout << "IP,Categories,ReportDate,Comment" << endl;

        if (g_useDetailedInfo) {
            for (const auto& entry : detailedConnList) {
                auto ip = entry.host;
                const auto offset = ip.find("::ffff:");
                if (offset != string::npos) {
                    ip = ip.substr(offset + 7);
                }

                int32_t totalConnections = entry.closedConnections;
                int32_t openConnections = entry.acceptedConnections - totalConnections;

                if (openConnections < 0) {
                    // This can happen if the log was rotated before a connection was closed
                    openConnections *= -1;
                    totalConnections += openConnections;
                }

                fmt::print(
                    R"({0:s},"{1:s}",{2:s},"{3:s}"{4:s})",
                    ip, categories, timestamp,
                    format(
                        detailedCommentFmt,
                        ip, openConnections, totalConnections,
                        getHumanReadableTime(entry.totalSecondsWasted),
                        getHumanReadableBytes(entry.totalBytesSent)
                    ), "\n"
                );
            }
        } else {
            for (const auto& entry : normalConnList) {
                // strip ::ffff: from IP address
                auto ip = entry.first;
                auto offset = ip.find("::ffff:");
                if (offset != string::npos) {
                    ip = ip.substr(offset + 7);
                }

                int32_t closedConnections = entry.second.second;
                int32_t openConnections = entry.second.first - closedConnections;

                if (openConnections < 0) {
                    // This can happen if the log was rotated before a connection was closed
                    openConnections *= -1;
                    closedConnections += openConnections;
                }

                fmt::print(
                    R"({0:s},"{1:s}",{2:s},"{3:s}"{4:s})",
                    ip, categories, timestamp,
                    format(
                        regularCommentFmt,
                        ip, openConnections, closedConnections
                    ), "\n"
                );
            }
        }
    }

    return 0;
}

/**
 * @brief Reads the file under g_logLocation and filters it for entries containing endlessh
 * 
 * @return vector<string> A vector containing all endlessh entries
 */
vector<string> readEndlesshLog() {
    const static string ENDLESSH = "endlessh";

    vector<string> output;

    auto _ = [](std::istream& stream, vector<string>& output) {
        string line;
        while (std::getline(stream, line)) {
            if (line.find(ENDLESSH) != string::npos) {
                output.push_back(line);
            }
        }
    };

    if (!g_readFromStdIn) {
        ifstream fileStream(g_logLocation);

        if (!fileStream.good()) {
            cerr << "Failed to open " + g_logLocation + "." << endl;
            g_error = true;
            return output;
        }

        _(fileStream, output);
    } else {
        _(std::cin, output);
    }

    return output;
}

/**
 * @brief Gets a map containing all of the opened and closed connections to the server.
 * 
 * @param logContents The contents of the log file as an array of lines.
 * 
 * @return map<string, pair<uint32_t, uint32_t>> A map containing the basic connection statistics.
 */
map<string, pair<uint32_t, uint32_t>> getConnections(const vector<string>& logContents) {
    map<string, pair<uint32_t, uint32_t>> connections;

    for (const auto& line : logContents) {
        vector<string> tokens;
        if (!splitString(line, " ", tokens)) {
            cerr << "Failed to parse " << line << "." << endl;
            continue;
        }

        bool isAccept = false;
        string hostToken;

        for (const auto& token : tokens) {
            if (regexMatch(token.c_str(), R"(host=[^\s])")) {
                hostToken = token.substr(token.find('=') + 1);
            } else if (token == "ACCEPT") {
                isAccept = true;
            }
        }

        if (hostToken.empty()) {
            // cerr << "Failed to find host= token!" << endl;
            // cerr << "\tLine: " << line << endl;
            continue;
        }

        auto iterator = connections.find(hostToken);
        if (iterator == connections.end()) {
            connections.emplace(
                hostToken,
                make_pair<uint32_t, uint32_t>(
                    isAccept ? 1 : 0, //    ACCEPT
                    isAccept ? 0 : 0 //     CLOSE
                )
            );
        } else if (isAccept) {
            iterator->second.first++;
        } else {
            iterator->second.second++;
        }
    }

    return connections;
}

/**
 * @brief Gets a list with detailled information about all the incoming connections to the server.
 * 
 * @param logContents The log file contents passed as an array of lines.
 * 
 * @return vector<ConnectionDetails> A list of @see ConnectionDetails containing all the goodies.
 */
vector<ConnectionDetails> getDetailledConnections(const vector<string>& logContents) {
    vector<ConnectionDetails> returnVal{};

    for (const auto& line : logContents) {
        vector<string> tokens;
        if (!splitString(line, " ", tokens)) {
            cerr << "Failed to parse " << line << "." << endl;
            continue;
        }

        bool isAccept = false;
        string hostToken;
        string portToken;
        string timeToken;
        string bytesToken;

        for (const auto& token : tokens) {
            if (regexMatch(token.c_str(), R"(host=[^\s])")) {
                hostToken = token.substr(token.find('=') + 1);
            } else if (token == "ACCEPT") {
                isAccept = true;
            } else if (regexMatch(token.c_str(), R"(port=[^\s])") && token.find('=') != string::npos) {
                portToken = token.substr(token.find('=') + 1);
            } else if (regexMatch(token.c_str(), R"(time=[^\s])") && token.find('=') != string::npos) {
                timeToken = token.substr(token.find('=') + 1);
            } else if (regexMatch(token.c_str(), R"(bytes=[^\s])") && token.find('=') != string::npos) {
                bytesToken = token.substr(token.find('=') + 1);
            }
        }

        if (hostToken.empty()) {
            // cerr << "Failed to find host= token!" << endl;
            // cerr << "\tLine: " << line << endl;
            continue;
        }

        auto element = std::find_if(returnVal.begin(), returnVal.end(), [&](const ConnectionDetails x) {
            return x.host == hostToken;
        });

        if (element == returnVal.end()) {
            ConnectionDetails x;
            x.host = hostToken;
            element = returnVal.emplace(returnVal.cend(), std::move(x));
        }

        if (isAccept) {
            element->acceptedConnections++;
            element->usedPorts.push_back(static_cast<uint16_t>(std::stoul(portToken)));
            continue;
        }

        element->closedConnections++;
        element->totalBytesSent += static_cast<size_t>(std::stoull(bytesToken));
        element->totalSecondsWasted += std::stold(timeToken);
    }

    return returnVal;
}

/**
 * @brief Print basic connection statistics
 * 
 * @param uniqueAddresses The total amount of unique IPs stuck in the tarpit
 * @param totalAccepted The total amount of accepted connections
 * @param totalClosed The total amount of closed connections
 * @param totalTimeWasted The total amount of time (in seconds) wasted
 * @param totalBytesSent The total amount of bytes sent to bots
 */
void printConnectionStatistics(const uint32_t uniqueAddresses, const uint32_t totalAccepted, const uint32_t totalClosed, const double totalTimeWasted, const uint32_t totalBytesSent) {
    // Prepare everything for markdown table while keeping the table code clean-ish
    // I'd rather this be ugly than the table tbh
    string tmpInt = std::to_string(uniqueAddresses);
    string tmp = getSpacerString(18, tmpInt.size());
    string uniqueIps = tmp + tmpInt + string(18 - tmpInt.size() - tmp.size(), ' ');

    string acceptedConns;
    tmpInt = std::to_string(totalAccepted);
    tmp = getSpacerString(28, tmpInt.size());
    acceptedConns = tmp + tmpInt + string(28 - tmpInt.size() - tmp.size(), ' ');


    string closedConns;
    tmpInt = std::to_string(totalClosed);
    tmp = getSpacerString(26, tmpInt.size());
    closedConns = tmp + tmpInt + string(26 - tmpInt.size() - tmp.size(), ' ');

    string aliveConns;
    if (totalAccepted >= totalClosed) {
        tmpInt = std::to_string(totalAccepted - totalClosed);
    } else {
        tmpInt = std::to_string(totalClosed - totalAccepted);
    }
    tmp = getSpacerString(25, tmpInt.size());
    aliveConns = tmp + tmpInt + string(25 - tmpInt.size() - tmp.size(), ' ');

    cout << "# Connection Statistics" << endl;
    cout << "| Total Unique IPs | Total Accepted Connections | Total Closed Connections | Total Alive Connections |";

    if (totalTimeWasted > 0) {
        cout << " Total Bot Time Wasted |";
    } if (totalBytesSent > 0) {
        cout << " Total Bytes Sent |";
    }
    cout << endl;

    cout << "|------------------|----------------------------|--------------------------|-------------------------|";

    if (totalTimeWasted > 0) {
        cout << "-----------------------|";
    } if (totalBytesSent > 0) {
        cout << "------------------|";
    }
    cout << endl;

    cout << "|" << uniqueIps << "|" << acceptedConns << "|" << closedConns << "|" << aliveConns << "|";
    
    if (totalTimeWasted > 0) {
        auto flooredSeconds = getHumanReadableTime(totalTimeWasted);
        tmp = getSpacerString(23, flooredSeconds.size());
        cout << tmp << flooredSeconds << string(23 - flooredSeconds.size() - tmp.size(), ' ') << '|';
    } if (totalBytesSent > 0) {
        auto totalBytes = getHumanReadableBytes(totalBytesSent);
        tmp = getSpacerString(18, totalBytes.size());
        cout << tmp << totalBytes << string(18 - totalBytes.size() - tmp.size(), ' ') << '|';
    }

    cout << endl;
}

/**
 * @brief Print the markdown header for the IP statistics table.
 */
void printIpStatsTableHeader() {
    if (!g_useDetailedInfo) {
        cout << "# Statistics per IP" << endl;
        cout << "|          Host          | Accepted | Closed |" << endl
             << "|------------------------|----------|--------|" << endl;
    } else {
        cout << "# Statistics per IP" << endl;
        cout << "|          Host          | Accepted | Closed | Total Time (s) | Total Bytes |" << endl
             << "|------------------------|----------|--------|----------------|-------------|" << endl;
    }
}

/**
 * @brief Prints the basic IP statistics table in markdown-format
 * 
 * @param connectionList The connection list
 * @param totalAcceptedConnections A reference to an unsigned int which will contain the total number of accepted connections.
 * @param totalClosedConnections A reference to an unsigned int which will contain the total number of closed connections.
 */
void printIpStats(const map<string, pair<uint32_t, uint32_t>>& connectionList, uint32_t& totalAcceptedConnections, uint32_t& totalClosedConnections) {
    for (const auto& connection : connectionList) {
        totalAcceptedConnections += connection.second.first;
        totalClosedConnections += connection.second.second;

        if (g_printIpStatistics) {
            string lastSpacer;
            string tmpString;
            const auto offset = connection.first.find("::ffff:");
            if (offset != string::npos) {
                tmpString = connection.first.substr(offset + 7);
            }
            cout << "|" << (lastSpacer = getSpacerString(24, tmpString.size()))
                << tmpString << string(24 - tmpString.size() - lastSpacer.size(), ' ') 
                << "|";

            auto strLength = std::to_string(connection.second.first).size();
            cout << (lastSpacer = getSpacerString(10, strLength))
                << connection.second.first << string(10 - strLength - lastSpacer.size(), ' ')
                << "|";
            
            
            strLength = std::to_string(connection.second.second).size();
            cout << (lastSpacer = getSpacerString(8, strLength))
                << connection.second.second << string(8 - strLength - lastSpacer.size(), ' ')
                << "|" << endl;
        }
    }
}

/**
 * @brief Prints a markdown-compatible table containing detailed information, such as the total time of the bot wasted and the bytes sent.
 * 
 * @param connectionList The list of connections.
 * @param totalAccepted The total accepted connections.
 * @param totalClosed The total closed connections.
 */
void printDetailedIpStats(const vector<ConnectionDetails>& connectionList, uint32_t& totalAccepted, uint32_t& totalClosed) {
    for (const auto& connection : connectionList) {
        totalAccepted += connection.acceptedConnections;
        totalClosed += connection.closedConnections;
        cout.precision(2);
        string tmpString{};

        string lastSpacer;
        const auto offset = connection.host.find("::ffff:");
        if (offset != string::npos) {
            tmpString = connection.host.substr(offset + 7);
        }
        cout << "|" << (lastSpacer = getSpacerString(24, tmpString.size()))
             << tmpString << string(24 - tmpString.size() - lastSpacer.size(), ' ') 
             << "|";

        auto strLength = std::to_string(connection.acceptedConnections).size();
        cout << (lastSpacer = getSpacerString(10, strLength))
             << connection.acceptedConnections << string(10 - strLength - lastSpacer.size(), ' ')
             << "|";
        
        
        strLength = std::to_string(connection.closedConnections).size();
        cout << (lastSpacer = getSpacerString(8, strLength))
             << connection.closedConnections << string(8 - strLength - lastSpacer.size(), ' ')
             << "|";

        strLength = (tmpString = getHumanReadableTime(connection.totalSecondsWasted)).size();
        cout << (lastSpacer = getSpacerString(16, strLength))
             << tmpString
             << string(16 - strLength - lastSpacer.size(), ' ')
             << "|";

        
        tmpString = getHumanReadableBytes(connection.totalBytesSent);
        strLength = tmpString.size();
        cout << (lastSpacer = getSpacerString(13, strLength))
             << tmpString << string(13 - strLength - lastSpacer.size(), ' ')
             << "|";

        cout << endl;
    }
}

/**
 * @brief Parses arguments passed to the application and sets values accordingly.
 * 
 * @param argC The total amount of args
 * @param argV A pointer-pointer to the passed args
 * 
 * @return int32_t 1 if the application should terminate. 0 otherwise
 */
int32_t parseArgs(const int32_t& argc, char** argv) {
    int32_t curIdx = 0;
    char optVal = 0;

    while ((optVal = getopt_long(argc, argv, getAppArgs().data(), getAppOptions(), &curIdx)) != -1) {
        switch (optVal) {
            default:
            case 'h':
                cout << getAppHelpText() << endl;
                return 1;
            case 'i':
                g_printIpStatistics = false;
                break;
            case 'c':
                g_printConnectionStatistics = false;
                break;
            case 'S':
                if (optarg == nullptr) {
                    cerr << "Missing path to new syslog!" << endl;
                    return 1;
                }
                g_logLocation = optarg;
                break;
            case 's':
                g_readFromStdIn = true;
                break;
            case 'a':
                g_printAbuseIpDbCsv = true;
                break;
            case 'n':
                g_disableAdvertisement = true;
                break;
            case 'd':
                g_useDetailedInfo = true;
                break;
            case 'v':
                cout << getAppVersionText() << endl;
                return 1;
        }
    }

    return 0;
}
