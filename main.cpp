/**
 * @file main.cpp
 * @author Simon Cahill (simon@simonc.eu)
 * @brief Parses endlessh's logs, counts the total number of connections, unique IPs and lists which IPs are still connected.
 * @version 0.1
 * @date 2022-01-07
 * 
 * @copyright Copyright (c) 2022 Simon Cahill
 */

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <regex.h>

using std::cout;
using std::cerr;
using std::endl;
using std::ifstream;
using std::make_pair;
using std::map;
using std::pair;
using std::string;
using std::vector;

bool    g_error = false;
bool    g_outputConnectionCount = true;
bool    g_outputAliveConnections = true;
bool    g_outputAliveConnectionCount = true;
bool    g_outputUniqueIds = true;
bool    g_outputUniqueIdCount = true;
string  g_logLocation = "/var/log/syslog";

bool                                    splitString(const string& str, const string& delimiters, vector<string> &tokens);
bool                                    regexMatch(const char* haystack, const char* needle);
map<string, pair<uint32_t, uint32_t>>   getConnections(const vector<string>&);
vector<string>                          readEndlesshLog();

int main() {
    // Read log file
    const auto logContents = readEndlesshLog();

    if (g_error) { return 1; }

    const auto connectionList = getConnections(logContents);
    uint32_t totalAcceptedConnections = 0;
    uint32_t totalClosedConnections = 0;

    cout << "|          Host          | Accepted | Closed |" << endl
         << "|------------------------|----------|--------|" << endl;
    for (const auto& connection : connectionList) {
        const auto spacer = string(24 / 2 - connection.first.size() / 2, ' ');
        totalAcceptedConnections += connection.second.first;
        totalClosedConnections += connection.second.second;
        cout << "|" << spacer << connection.first << spacer << "| " << connection.second.first << " | " << connection.second.second << " | " << endl;
    }
    cout << endl;

    cout << "**Total unique IPs found: " << connectionList.size() << "**" << endl
         << "**Total accepted connections: " << totalAcceptedConnections << "**" << endl
         << "**Total closed connections: " << totalClosedConnections << "**" << endl;

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
    ifstream fileStream(g_logLocation);

    if (!fileStream.good()) {
        cerr << "Failed to open " + g_logLocation + "." << endl;
        g_error = true;
        return output;
    }

    string line;

    while (std::getline(fileStream, line)) {
        if (line.find(ENDLESSH) != string::npos) {
            output.push_back(line);
        }
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

        for (const auto token : tokens) {
            if (regexMatch(token.c_str(), R"(host=[^\s])")) {
                hostToken = token.substr(token.find('='));
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
