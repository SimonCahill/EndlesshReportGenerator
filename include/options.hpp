/**
 * @file options.hpp
 * @author Simon Cahill (simon@simonc.eu)
 * @brief Contains the implementation of features required for use of getopt_long
 * @version 0.1
 * @date 2022-10-15
 * 
 * @copyright Copyright (c) 2022 Simon Cahill
 */

#ifndef ENDLESSH_REPORT_INCLUDE_OPTIONS_HPP
#define ENDLESSH_REPORT_INCLUDE_OPTIONS_HPP

#include "version.hpp"

// stl
#include <string>

// libc
#include <getopt.h>

// fmt
#include <fmt/format.h>

using std::string;
using std::string_view;

/**
 * @brief Gets the string containing the application's short arg string.
 * 
 * @return constexpr string_view The arg string as required for @see getopt_long
 */
constexpr string_view   getAppArgs() { return R"(icsandhvS:)"; }

/**
 * @brief Gets the application's command-line options for @see getopt_long.
 * 
 * @return const option* A const pointer to the array containing the options.
 */
const option*           getAppOptions() {
    const static option OPTIONS[] = {
        { "no-ip-stats",    no_argument,        nullptr,    'i' },
        { "no-cn-stats",    no_argument,        nullptr,    'c' },
        { "stdin",          no_argument,        nullptr,    's' },
        { "abuse-ipdb",     no_argument,        nullptr,    'a' },
        { "detailed",       no_argument,        nullptr,    'd' },
        { "help",           no_argument,        nullptr,    'h' },
        { "syslog",         required_argument,  nullptr,    'S' },
        { "version",        no_argument,        nullptr,    'v' },
        { nullptr,          no_argument,        nullptr,     0  }
    };

    return OPTIONS;
}

/**
 * @brief Gets the application's help text.
 * 
 * @param binName The name of the binary.
 * 
 * @return string The actual help text.
 */
inline string           getAppHelpText(const string_view& binName = getProjectName()) {
    const static string HELP_TEXT_FMT = R"(
{0:s} v{1:s} - {2:s}

Usage:
    {0:s}
    {0:s} [options]
    {0:s} --syslog/var/log/syslog.1
    cat <file> | {0:s} --stdin

Switches:
    --no-ip-stats,  -i      Don't print IP statistics
    --no-cn-stats,  -c      Don't print connection statistics
    --stdin,        -s      Read logs from stdin
    --abuse-ipdb,   -a      Enable AbuseIPDB-compatible CSV output
    --no-ad,        -n      No advertising please!
    --detailed,     -d      Provide detailed information
    --help,         -h      Show this text and exit
    --version,      -v      Display version information and exit

Arguments:
    --syslog [f],   -S[f]   Override syslog/endlessh log location
)";

    return fmt::format(HELP_TEXT_FMT, binName, getApplicationVersion(), getProjectDescription());
}

/**
 * @brief Gets the application's version information.
 * 
 * @return string The string containing the version information.
 */
inline string           getAppVersionText() { return fmt::format(R"({0:s} v{1:s} - {2:s})", getProjectName(), getApplicationVersion(), getProjectDescription()); }

#endif // ENDLESSH_REPORT_INCLUDE_OPTIONS_HPP