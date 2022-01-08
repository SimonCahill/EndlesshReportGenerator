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
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <regex.h>

using std::cout;
using std::cerr;
using std::endl;
using std::function;
using std::ifstream;
using std::make_pair;
using std::map;
using std::pair;
using std::string;
using std::vector;

bool    g_error = false; //!< Whether or not a fatal error occurred
bool    g_printIpStatistics = true; //!< Whether or not to print IP stats (default: true)
bool    g_printConnectionStatistics = true; //!< Whether or not to print connection stats (default: true)
bool    g_readFromStdIn = false; //!< Whether or not to read from stdin (default: false)
string  g_logLocation = "/var/log/syslog"; //!< Default endlessh log location (default: /var/log/syslog)

bool                                    splitString(const string& str, const string& delimiters, vector<string> &tokens); //!< Splits a string by one or more delimiters
bool                                    regexMatch(const char* haystack, const char* needle); //!< Matches a string against a regular expression
int32_t                                 parseArgs(const int32_t&, char**); //!< Parses command-line arguments
map<string, pair<uint32_t, uint32_t>>   getConnections(const vector<string>&); //!< Gets the logged connections
string                                  getSpacerString(const uint32_t totalWidth, const uint32_t strLength); //!< Gets the spacer string (might be removed)
string                                  trimStart(string nonTrimmed, const string& trimChar); //!< Trims the start of a string
string                                  trimEnd(string nonTrimmed, const string& trimChar); //!< Trims the end of a string
string                                  trim(string nonTrimmed, const string& trimChar); //!< Trims a string
vector<string>                          readEndlesshLog(); //!< Reads the log file into memory

int main(int32_t argC, char** argV) {
    if (parseArgs(argC, argV) == 1) {
        return 0;
    }

    // Read log file
    const auto logContents = readEndlesshLog();

    if (g_error) { return 1; }

    const auto connectionList = getConnections(logContents);
    uint32_t totalAcceptedConnections = 0;
    uint32_t totalClosedConnections = 0;

    if (g_printIpStatistics) {
        cout << "# Statistics per IP" << endl;
        cout << "|          Host          | Accepted | Closed |" << endl
            << "|------------------------|----------|--------|" << endl;
    }

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
    if (g_printIpStatistics) { cout << endl; }

    if (g_printConnectionStatistics) {
        // Prepare everything for markdown table while keeping the table code clean-ish
        // I'd rather this be ugly than the table tbh
        string tmpInt = std::to_string(connectionList.size());
        string tmp = getSpacerString(18, tmpInt.size());
        string uniqueIps = tmp + tmpInt + string(18 - tmpInt.size() - tmp.size(), ' ');

        string acceptedConns;
        tmpInt = std::to_string(totalAcceptedConnections);
        tmp = getSpacerString(28, tmpInt.size());
        acceptedConns = tmp + tmpInt + string(28 - tmpInt.size() - tmp.size(), ' ');


        string closedConns;
        tmpInt = std::to_string(totalClosedConnections);
        tmp = getSpacerString(26, tmpInt.size());
        closedConns = tmp + tmpInt + string(26 - tmpInt.size() - tmp.size(), ' ');

        string aliveConns;
        tmpInt = std::to_string(totalAcceptedConnections - totalClosedConnections);
        tmp = getSpacerString(25, tmpInt.size());
        aliveConns = tmp + tmpInt + string(25 - tmpInt.size() - tmp.size(), ' ');

        cout << "# Connection Statistics" << endl;
        cout << "| Total Unique IPs | Total Accepted Connections | Total Closed Connections | Total Alive Connections |" << endl
            << "|------------------|----------------------------|--------------------------|-------------------------|" << endl
            << "|" <<uniqueIps << "|" << acceptedConns      << "|" << closedConns      << "|" << aliveConns      << "|" << endl;
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

        for (const auto token : tokens) {
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
string trim(string nonTrimmed, const string& trimChar) { return trimStart(trimEnd(nonTrimmed, trimChar), trimChar); }

/**
 * @brief Parses arguments passed to the application and sets values accordingly.
 * 
 * @param argC The total amount of args
 * @param argV A pointer-pointer to the passed args
 * 
 * @return int32_t 1 if the application should terminate. 0 otherwise
 */
int32_t parseArgs(const int32_t& argC, char** argV) {
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
                 << "\t--help, -h\t\tPrints this message and exits"
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
        } else {
            cerr << "Unknown argument " << arg << endl;
            continue; // redundant as of now
        }
    }

    return 0;
}
