#include "public_api.hpp"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <cstring>

using std::string;

// Define global configuration flags here so the core library provides the symbol
bool g_performDnsLookup{false};

string normalizeIp(string ip) {
    const auto offset = ip.find("::ffff:");
    if (offset != string::npos) {
        ip = ip.substr(offset + 7);
    }
    return ip;
}

string getHostFromIp(const string& ip) {
    string host;
    // By default the application runs without DNS lookups; keep behaviour
    // identical to original implementation when lookups are disabled.
    extern bool g_performDnsLookup; // declared in main.cpp
    if (g_performDnsLookup) {
        const string ipOnly = normalizeIp(ip);
        struct sockaddr_storage sa;
        socklen_t sa_len = 0;
        std::memset(&sa, 0, sizeof(sa));

        struct in_addr ipv4addr;
        if (inet_pton(AF_INET, ipOnly.c_str(), &ipv4addr) == 1) {
            struct sockaddr_in *sin = reinterpret_cast<struct sockaddr_in *>(&sa);
            sin->sin_family = AF_INET;
            sin->sin_addr = ipv4addr;
            sin->sin_port = 0;
            sa_len = sizeof(struct sockaddr_in);
        } else {
            struct in6_addr ipv6addr;
            if (inet_pton(AF_INET6, ipOnly.c_str(), &ipv6addr) == 1) {
                struct sockaddr_in6 *sin6 = reinterpret_cast<struct sockaddr_in6 *>(&sa);
                sin6->sin6_family = AF_INET6;
                sin6->sin6_addr = ipv6addr;
                sin6->sin6_port = 0;
                sa_len = sizeof(struct sockaddr_in6);
            } else {
                host = ip;
                return host;
            }
        }

        char hbuf[NI_MAXHOST] = {0};
        const int flags = NI_NAMEREQD;
        if (getnameinfo(reinterpret_cast<struct sockaddr *>(&sa), sa_len, hbuf, sizeof(hbuf), nullptr, 0, flags) == 0) {
            host = string(hbuf);
        } else {
            host = ip;
        }
    } else {
        host = ip;
    }

    return host;
}
