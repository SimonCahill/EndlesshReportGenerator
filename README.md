# EndlesshBasicStats
(Very) simple and rudimentary C++ program that reads /var/log/syslog, filters out endlessh logs and determines basic stats, such as unique IDs, total accepted and closed connections. 

# Building
As a one-file project, I didn't see the need to create a Makefile. <br >
This will only work on Linux systems.

```bash
g++ -std=c++17 -o unique_ids main.cpp
```

# Usage

After building, simply call:

```bash
./unique_ids
```

## Output
This program outputs its text in Markdown-compatible form.
Because I wrote this program for my own use on my servers, I developed it for use with my report scripts that send me periodic emails.

## Recommended use

```bash
# requires pandoc
./unique_ids | pandoc -f markdown -t html | mail -a "Content-Type: text/html; charset=UTF-8" -s "My Endlessh Report" "you@yourdomain.com"
```