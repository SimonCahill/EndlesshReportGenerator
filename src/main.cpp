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

#include "version.hpp"

////////////////////////////////
//  Standard Includes (STL)   //
////////////////////////////////
#include <chrono>
// #include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <regex.h>

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

bool    g_error = false; //!< Whether or not a fatal error occurred
bool    g_disableAdvertisement = false; //!< Whether or not to disable advertisement (default: false)
bool    g_printAbuseIpDbCsv = false; //!< Whether or not to output AbuseIPDB-compatible CSV data (default: false; disables markdown-compatible stats)
bool    g_printIpStatistics = true; //!< Whether or not to print IP stats (default: true)
bool    g_printConnectionStatistics = true; //!< Whether or not to print connection stats (default: true)
bool    g_readFromStdIn = false; //!< Whether or not to read from stdin (default: false)
bool    g_useDetailedInfo = false; //!< Whether or not reports should be detailed (default: false)
string  g_logLocation = "/var/log/syslog"; //!< Default endlessh log location (default: /var/log/syslog)

/**
 * @brief Contains information about a given connection.
 */
struct ConnectionDetails {
    size_t              acceptedConnections;
    size_t              closedConnections;

    vector<uint16_t>    usedPorts;

    double              totalSecondsWasted;

    size_t              totalBytesSent;

    string              host;

    ConnectionDetails(): acceptedConnections(0), closedConnections(0),
    usedPorts({}), totalSecondsWasted(0), totalBytesSent(0), host({}) {}
};

bool                                    splitString(const string& str, const string& delimiters, vector<string> &tokens); //!< Splits a string by one or more delimiters
bool                                    regexMatch(const char* haystack, const char* needle); //!< Matches a string against a regular expression
double  	                            roundNumber(const double x, const uint32_t decimalPlaces); //!<
int32_t                                 parseArgs(const int32_t&, const char**); //!< Parses command-line arguments
map<string, pair<uint32_t, uint32_t>>   getConnections(const vector<string>&); //!< Gets the logged connections
string                                  getCurrentIsoTimestamp(); //!< Gets the current time as an ISO timestamp, accurate to the current second.
string                                  getSpacerString(const uint32_t totalWidth, const uint32_t strLength); //!< Gets the spacer string (might be removed)
string                                  trimStart(string nonTrimmed, const string& trimChar); //!< Trims the start of a string
string                                  trimEnd(string nonTrimmed, const string& trimChar); //!< Trims the end of a string
string                                  trim(const string& nonTrimmed, const string& trimChar); //!< Trims a string
vector<ConnectionDetails>               getDetailledConnections(const vector<string>&); //!< Gets a detailled list of logged connections
vector<string>                          readEndlesshLog(); //!< Reads the log file into memory
void                                    printConnectionStatistics(const uint32_t uniqueIps, const uint32_t totalAccepted, const uint32_t totalClosed, const double totalTimeWasted, const uint32_t totalBytesSent); //!< Print connection statistics
void                                    printIpStatsTableHeader(); //!< Prints the markdown header for the statistics table
void                                    printIpStats(const map<string, pair<uint32_t, uint32_t>>&, uint32_t& totalAccepted, uint32_t& totalClosed); //!< Prints the IP stats
void                                    printDetailedIpStats(const vector<ConnectionDetails>&, uint32_t& totalAccepted, uint32_t& totalClosed); //!< Prints detailed IP stats

int main(int32_t argC, char** argV) {
    if (parseArgs(argC, const_cast<const char**>(argV)) == 1) {
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
            printConnectionStatistics(normalConnList.size(), totalAcceptedConnections, totalClosedConnections, 0, 0);
        } else {
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
        // auto message = "{} fell into Endlessh tarpit; {} total, {} closed connections."; // wait until GCC supports fmt

        cout << "IP,Categories,ReportDate,Comment" << endl;

        if (g_useDetailedInfo) {
            for (const auto& entry : detailedConnList) {
                auto ip = entry.host;
                const auto offset = ip.find("::ffff:");
                if (offset != string::npos) {
                    ip = ip.substr(offset + 7);
                }

                cout << ip << ","
                     << '"' << categories << '"' << ","
                     << timestamp << ","
                     << '"'
                        << ip << " fell into Endlessh tarpit; "
                        << "opened " << entry.acceptedConnections << ", closed " << entry.closedConnections << " connections. "
                        << "Total time wasted: " << entry.totalSecondsWasted << "s. "
                        << "Total bytes sent by tarpit: " << entry.totalBytesSent << "B "
                        << (g_disableAdvertisement ? "" : "(Report generated by Endlessh Report Generator)")
                     << '"' << endl;
            }
        } else {
            for (const auto& entry : normalConnList) {
                // strip ::ffff: from IP address
                auto ip = entry.first;
                auto offset = ip.find("::ffff:");
                if (offset != string::npos) {
                    ip = ip.substr(offset + 7);
                }

                cout << ip << ","
                     << '"' << categories << '"' << ","
                     << timestamp << ","
                //   << std::format(message, entry.first, entry.second.first, entry.second.second) << endl;
                     << '"' << ip << " fell into Endlessh tarpit; "
                     << entry.second.first << " total, " << entry.second.second << " closed connections. "
                     << (g_disableAdvertisement ? "" : "(Report generated by Endlessh Report Generator)")
                     << '"'
                     << endl;
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
        auto flooredSeconds = std::to_string(roundNumber(totalTimeWasted, 2));
        tmp = getSpacerString(23, flooredSeconds.size());
        cout << tmp << flooredSeconds << string(23 - flooredSeconds.size() - tmp.size(), ' ') << '|';
    } if (totalBytesSent > 0) {
        auto totalBytes = std::to_string(totalBytesSent);
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
            cout << "|" << (lastSpacer = getSpacerString(24, connection.first.size()))
                << connection.first << string(24 - connection.first.size() - lastSpacer.size(), ' ') 
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

        const auto flooredSeconds = roundNumber(connection.totalSecondsWasted, 2);
        strLength = (tmpString = std::to_string(flooredSeconds)).size();
        cout << (lastSpacer = getSpacerString(16, strLength))
             << tmpString
             << string(16 - strLength - lastSpacer.size(), ' ')
             << "|";

        
        if (connection.totalBytesSent > 1024) {
            tmpString = std::to_string(connection.totalBytesSent / 1024) + "KiB";
        } else {
            tmpString = std::to_string(connection.totalBytesSent);
        }
        strLength = tmpString.size();
        cout << (lastSpacer = getSpacerString(13, strLength))
             << (connection.totalBytesSent > 1024 ? std::to_string(connection.totalBytesSent / 1024) + "KiB" : std::to_string(connection.totalBytesSent)) << string(13 - strLength - lastSpacer.size(), ' ')
             << "|";

        cout << endl;
    }
}

/**
 * @brief Splits a given string by the passed delimiters in to a vector.
 *
 * @param str string& The string to split.
 * @param delimiters string& A string containing the delimiters to split by (chars).
 * @param tokens vector<string>& A vector that will contain the elements.
 * 
 * @return true If tokens were found.
 * @return false Otherwise.
 */
bool splitString(const string& str, const string& delimiters, vector<string> &tokens) {
    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos) {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.
        lastPos = string::npos == pos ? string::npos : pos + 1;
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }

    return tokens.size() > 0;
}

/**
 * @brief Matches a string against a regular expression.
 * 
 * @param haystack The string to compare
 * @param needle The expression to compare against
 * 
 * @return true If the string matches the expression
 * @return false Otherwise
 */
bool regexMatch(const char* haystack, const char* needle) {
    int nRet;
    regex_t sRegEx;

    if (regcomp(&sRegEx, needle, REG_EXTENDED | REG_NOSUB) != 0) {
        return false;
    }

    nRet = regexec(&sRegEx, haystack, (size_t) 0, NULL, 0);
    regfree(&sRegEx);

    if (nRet != 0) {
        return false;
    }

    return true;
}

/**
 * @brief Rounds a given double to the desired amount of decimal places.
 * 
 * @param x The double to round
 * @param decimalPlaces The decimal places the double should be rounded to
 * 
 * @return double The resulting double.
 */
double roundNumber(const double x, const uint32_t decimalPlaces) {
    const double multiplificationFaktor = std::pow(10.0, decimalPlaces);
    return std::ceil(x * multiplificationFaktor) / multiplificationFaktor;
}

/**
 * @brief Gets the current timestamp as an ISO timestamp, accurate to the nearest second.
 * 
 * @return string A string containing the current timestamp in ISO format.
 */
string getCurrentIsoTimestamp() {
    auto timeNow = system_clock::now();
    return date::format("%FT%TZ", date::floor<seconds>(timeNow));
}

/**
 * @brief Gets a string containing whitespace to centre text
 * 
 * @param totalWidth The total width of the area where the string should be centred
 * @param strLength The length of the string to be centre-printed
 * 
 * @return string The spacer string
 */
string getSpacerString(const uint32_t totalWidth, const uint32_t strLength) {
    return string(totalWidth / 2 - strLength / 2, ' ');
}

/**
 * @brief Trims the beginning of a given string.
 *
 * @param nonTrimmed The non-trimmed string.
 * @param trimChar The character to trim off the string. (default: space)
 *
 * @return The trimmed string.
 */
string trimStart(string nonTrimmed, const string& trimChar) {
    //nonTrimmed.erase(nonTrimmed.begin(), find_if(nonTrimmed.begin(), nonTrimmed.end(), not1(ptr_fun<int32_t, int32_t>(isspace))));

    function<bool(char)> shouldTrimChar = [=](char c) -> bool { return trimChar.size() == 0 ? isspace(c) : trimChar.find(c) != string::npos; };

    nonTrimmed.erase(nonTrimmed.begin(), find_if(nonTrimmed.begin(), nonTrimmed.end(), not1(shouldTrimChar)));

    return nonTrimmed;
}

/**
 * @brief Trims the end of a given string.
 * @param nonTrimmed The non-trimmed string.
 * @param trimChar The character to trim off the string. (default: space)
 *
 * @return The trimmed string.
 */
string trimEnd(string nonTrimmed, const string& trimChar) {
    // nonTrimmed.erase(find_if(nonTrimmed.rbegin(), nonTrimmed.rend(), not1(ptr_fun<int32_t, int32_t>(isspace))).base(), nonTrimmed.end());

    function<bool(char)> shouldTrimChar = [=](char c) -> bool { return trimChar.size() == 0 ? isspace(c) : trimChar.find(c) != string::npos; };
    nonTrimmed.erase(find_if(nonTrimmed.rbegin(), nonTrimmed.rend(), not1(shouldTrimChar)).base(), nonTrimmed.end());

    return nonTrimmed;
}

/**
 * @brief Trims both the beginning and the end of a given string.
 *
 * @param nonTrimmed The non-trimmed string.
 * @param trimChar The character to trim off the string. (default: space)
 *
 * @return The trimmed string.
 */
string trim(const string& nonTrimmed, const string& trimChar) { return trimStart(trimEnd(nonTrimmed, trimChar), trimChar); }

/**
 * @brief Parses arguments passed to the application and sets values accordingly.
 * 
 * @param argC The total amount of args
 * @param argV A pointer-pointer to the passed args
 * 
 * @return int32_t 1 if the application should terminate. 0 otherwise
 */
int32_t parseArgs(const int32_t& argC, const char** argV) {
    for (int32_t i = 1; i < argC; i++) {
        string arg = argV[i];

        if (arg == "-h" || arg == "--help") {
            cout << "Usage: " << argV[0] << endl
                 << "Usage: " << argV[0] << " [options]" << endl
                 << "Usage: cat file | " << argV[0] << "--stdin" << endl << endl

                 << "Switches:" << endl
                 << "\t--no-ip-stats, -i\tDon't print IP statistics" << endl
                 << "\t--no-cn-stats, -c\tDon't print connection statistics" << endl
                 << "\t--stdin \t\tRead logs from stdin" << endl
                 << "\t--abuse-ipdb, -a\tEnable AbuseIPDB-compatible CSV output" << endl
                 << "\t--no-ad, -n\t\tNo advertising please!" << endl
                 << "\t--detailed, -d\t\tProvide detailed information." << endl
                 << "\t--help, -h\t\tPrints this message and exits" << endl
                 << "Arguments:" << endl
                 << "\t--syslog </path/to>\tOverride default syslog path (" << g_logLocation << ")" << endl;

            return 1;
        } else if (arg == "--no-ip-stats" || arg == "-i") {
            g_printIpStatistics = false;
        } else if (arg == "--no-cn-stats" || arg == "-c") {
            g_printConnectionStatistics = false;
        } else if (arg == "--syslog") {
            ++i;
            g_logLocation = argV[i];
        } else if (arg == "--stdin") {
            g_readFromStdIn = true;
        } else if (arg == "--abuse-ipdb" || arg == "-a") {
            g_printAbuseIpDbCsv = true;
        } else if (arg == "--no-ad" || arg == "-n") {
            g_disableAdvertisement = true;
        } else if (arg == "--detailed" || arg == "-d") {
            g_useDetailedInfo = true;
        } else if (arg == "--version" || arg == "-v") {
            cout << "v" << getApplicationVersion() << endl;
            return 1;
        } else {
            cerr << "Unknown argument " << arg << endl;
            continue; // redundant as of now
        }
    }

    return 0;
}
