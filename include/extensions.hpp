/**
 * @file extensions.hpp
 * @author Simon Cahill (simon@simonc.eu)
 * @brief Contains useful extension methods that aren't provided by C++ out of the box.
 * @version 0.1
 * @date 2022-10-15
 * 
 * @copyright Copyright (c) 2022 Simon Cahill
 */

#ifndef ENDLESSH_REPORT_INCLUDE_EXTENSIONS_HPP
#define ENDLESSH_REPORT_INCLUDE_EXTENSIONS_HPP

// stl
#include <chrono>
#include <cmath>
#include <string>
#include <vector>

// libc
#include <regex.h>

// date
#include <date/date.h>

using std::string;
using std::vector;

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
inline bool splitString(const string& str, const string& delimiters, vector<string> &tokens) {
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
inline bool regexMatch(const char* haystack, const char* needle) {
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
inline double roundNumber(const double x, const uint32_t decimalPlaces) {
    const double multiplificationFaktor = std::pow(10.0, decimalPlaces);
    return std::ceil(x * multiplificationFaktor) / multiplificationFaktor;
}

/**
 * @brief Gets the current timestamp as an ISO timestamp, accurate to the nearest second.
 * 
 * @return string A string containing the current timestamp in ISO format.
 */
inline string getCurrentIsoTimestamp() {
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
inline string getSpacerString(const uint32_t totalWidth, const uint32_t strLength) {
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
inline string trimStart(string nonTrimmed, const string& trimChar) {
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
inline string trimEnd(string nonTrimmed, const string& trimChar) {
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
inline string trim(const string& nonTrimmed, const string& trimChar) { return trimStart(trimEnd(nonTrimmed, trimChar), trimChar); }


#endif // ENDLESSH_REPORT_INCLUDE_EXTENSIONS_HPP