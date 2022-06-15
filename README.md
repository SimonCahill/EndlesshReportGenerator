# Endlessh Report Generator
(Very) simple and rudimentary C++ program that reads /var/log/syslog, filters out endlessh logs and determines basic stats, such as unique IDs, total accepted and closed connections. 

# Building
This will only work on Linux systems.

```bash
# build docs
make docs

# build debug
make debug

# build release
make

# build all
make all

# clean repo
make clean
```

# Installing
After building the software, either move or copy it to /usr/local/bin, or add the build path to your local $PATH environment variable.

## System-wide installation
```bash
# install to /usr/local/bin
sudo make install
```

## User-specific install
```bash
# ensure local bin-dir exists
mkdir -p ~/.local/bin || >&2 echo "Failed to create local bin dir!"
cp -v endlessh-report ~/.local/bin/

echo "export PATH=\"$(realpath ~)/.local/bin:\$PATH\"" | tee -a ~/.bashrc
```

# Usage

After building, simply call:

```bash
./endlessh-report
# or
endlessh-report
```

## Arguments

```
Usage: ./endlessh-report
Usage: ./endlessh-report [options]
Usage: cat file | ./endlessh-report--stdin

Switches:
        --no-ip-stats, -i       Don't print IP statistics
        --no-cn-stats, -c       Don't print connection statistics
        --stdin                 Read logs from stdin
        --abuse-ipdb, -a        Enable AbuseIPDB-compatible CSV output
        --no-ad, -n             No advertising please!
        --detailed, -d          Provide detailed information.
        --help, -h              Prints this message and exits
Arguments:
        --syslog </path/to>     Override default syslog path (/var/log/syslog)
```

## Output
This program outputs its text in Markdown-compatible form.
Because I wrote this program for my own use on my servers, I developed it for use with my report scripts that send me periodic emails.

## Recommended use

```bash
# requires pandoc
./endlessh-report | pandoc -f markdown -t html | mail -a "Content-Type: text/html; charset=UTF-8" -s "My Endlessh Report" "you@yourdomain.com"
```

# Sample Output
`endlessh-report -i` generates the following output:

```markdown
# Connection Statistics
| Total Unique IPs | Total Accepted Connections | Total Closed Connections | Total Alive Connections |
|------------------|----------------------------|--------------------------|-------------------------|
|        178       |              0             |             0            |            0            |
```

Resulting in:

# Connection Statistics
| Total Unique IPs | Total Accepted Connections | Total Closed Connections | Total Alive Connections |
|------------------|----------------------------|--------------------------|-------------------------|
|        178       |              0             |             0            |            0            |

`endlessh-report -c` generates the following output:
(truncated for readability)

> **Note that these are all botnet-IPs and are already publicly listed as such.**

```markdown
# Statistics per IP
|          Host          | Accepted | Closed |
|------------------------|----------|--------|
|  ::ffff:112.85.42.229  |    21    |   21   |
|   ::ffff:112.85.42.87  |     4    |    4   |
|   ::ffff:112.85.42.88  |    14    |   14   |
|  ::ffff:122.194.229.40 |    20    |   20   |
|  ::ffff:122.194.229.45 |    19    |   19   |
```

Resulting in:


# Statistics per IP
|          Host          | Accepted | Closed |
|------------------------|----------|--------|
|  ::ffff:112.85.42.229  |    21    |   21   |
|   ::ffff:112.85.42.87  |     4    |    4   |
|   ::ffff:112.85.42.88  |    14    |   14   |
|  ::ffff:122.194.229.40 |    20    |   20   |
|  ::ffff:122.194.229.45 |    19    |   19   |

# Detailed Statistics
Since version v1.1.0 endlessh-report now allows for more detailed reports to be generated.
These include factors such as bot time wasted, and total bytes sent.

Examples are:

## Detailed IP Statistics

```markdown
# Statistics per IP
|          Host          | Accepted | Closed | Total Time (s) | Total Bytes |
|------------------------|----------|--------|----------------|-------------|
|      218.92.0.206      |   3552   |  3547  | 2202334.810000 |  262224KiB  |
|     61.177.172.108     |     8    |    8   |   5282.840000  |   653KiB    |
|      61.177.173.46     |     8    |    8   |   7328.230000  |   909KiB    |
```

## Detailed Connection Statistics
```markdown
# Connection Statistics
| Total Unique IPs | Total Accepted Connections | Total Closed Connections | Total Alive Connections | Total Bot Time Wasted | Total Bytes Sent |
|------------------|----------------------------|--------------------------|-------------------------|-----------------------|------------------|
|        99        |              0             |             0            |            0            |    4743646.220000     |     595462069    |
```

## Detailed AbuseIPDB CSV format
```csv
IP,Categories,ReportDate,Comment
218.92.0.206,"18,14,22,15",2022-06-14T21:40:58Z,"218.92.0.206 fell into Endlessh tarpit; opened 3561, closed 3569 connections. Total time wasted: 2.21585e+06s. Total bytes sent by tarpit: 269973303B (Report generated by Endlessh Report Generator)"
61.177.172.108,"18,14,22,15",2022-06-14T21:40:58Z,"61.177.172.108 fell into Endlessh tarpit; opened 8, closed 8 connections. Total time wasted: 5282.83s. Total bytes sent by tarpit: 669662B (Report generated by Endlessh Report Generator)"
61.177.173.46,"18,14,22,15",2022-06-14T21:40:58Z,"61.177.173.46 fell into Endlessh tarpit; opened 8, closed 8 connections. Total time wasted: 7328.23s. Total bytes sent by tarpit: 931388B (Report generated by Endlessh Report Generator)"
136.144.41.181,"18,14,22,15",2022-06-14T21:40:58Z,"136.144.41.181 fell into Endlessh tarpit; opened 1, closed 1 connections. Total time wasted: 116.022s. Total bytes sent by tarpit: 13799B (Report generated by Endlessh Report Generator)"
61.177.173.50,"18,14,22,15",2022-06-14T21:40:58Z,"61.177.173.50 fell into Endlessh tarpit; opened 12, closed 12 connections. Total time wasted: 9732.31s. Total bytes sent by tarpit: 1255507B (Report generated by Endlessh Report Generator)"
45.61.188.110,"18,14,22,15",2022-06-14T21:40:58Z,"45.61.188.110 fell into Endlessh tarpit; opened 5, closed 5 connections. Total time wasted: 15.004s. Total bytes sent by tarpit: 1154B (Report generated by Endlessh Report Generator)"
61.177.173.35,"18,14,22,15",2022-06-14T21:40:58Z,"61.177.173.35 fell into Endlessh tarpit; opened 10, closed 10 connections. Total time wasted: 6529.89s. Total bytes sent by tarpit: 831889B (Report generated by Endlessh Report Generator)"
61.177.172.98,"18,14,22,15",2022-06-14T21:40:58Z,"61.177.172.98 fell into Endlessh tarpit; opened 9, closed 9 connections. Total time wasted: 4610.36s. Total bytes sent by tarpit: 592071B (Report generated by Endlessh Report Generator)"
223.71.167.164,"18,14,22,15",2022-06-14T21:40:58Z,"223.71.167.164 fell into Endlessh tarpit; opened 3, closed 3 connections. Total time wasted: 8.001s. Total bytes sent by tarpit: 598B (Report generated by Endlessh Report Generator)"
45.61.185.160,"18,14,22,15",2022-06-14T21:40:58Z,"45.61.185.160 fell into Endlessh tarpit; opened 3, closed 3 connections. Total time wasted: 9.004s. Total bytes sent by tarpit: 955B (Report generated by Endlessh Report Generator)"
78.142.18.204,"18,14,22,15",2022-06-14T21:40:58Z,"78.142.18.204 fell into Endlessh tarpit; opened 28, closed 28 connections. Total time wasted: 103.027s. Total bytes sent by tarpit: 9708B (Report generated by Endlessh Report Generator)"
61.177.173.51,"18,14,22,15",2022-06-14T21:40:58Z,"61.177.173.51 fell into Endlessh tarpit; opened 7, closed 7 connections. Total time wasted: 3609.56s. Total bytes sent by tarpit: 463135B (Report generated by Endlessh Report Generator)"
67.207.83.91,"18,14,22,15",2022-06-14T21:40:58Z,"67.207.83.91 fell into Endlessh tarpit; opened 1, closed 1 connections. Total time wasted: 6.002s. Total bytes sent by tarpit: 644B (Report generated by Endlessh Report Generator)"
45.61.184.111,"18,14,22,15",2022-06-14T21:40:58Z,"45.61.184.111 fell into Endlessh tarpit; opened 7, closed 7 connections. Total time wasted: 21.004s. Total bytes sent by tarpit: 2096B (Report generated by Endlessh Report Generator)"
```
