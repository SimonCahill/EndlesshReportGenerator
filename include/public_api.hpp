/**
 * @file public_api.hpp
 * @brief Public API wrappers for functions that are implemented in `main.cpp`.
 */
#ifndef ENDLESSH_REPORT_PUBLIC_API_HPP
#define ENDLESSH_REPORT_PUBLIC_API_HPP

#include <string>

using std::string;

// Expose utility functions for unit testing.
string normalizeIp(string ip);
string getHostFromIp(const string& ip);
// Global configuration flags used by the application. Tests may rely on defaults.
extern bool g_performDnsLookup;

#endif // ENDLESSH_REPORT_PUBLIC_API_HPP
