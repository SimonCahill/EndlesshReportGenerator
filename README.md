# Endlessh Report Generator
(Very) simple and rudimentary C++ program that reads /var/log/syslog, filters out endlessh logs and determines basic stats, such as unique IDs, total accepted and closed connections. 

# Building
This will only work on Linux systems.

```bash
mkdir build && cd build # create build directory

# generate makefiles
cmake ..

# build and generate .deb
cpack .

# only build application
make

# make docs
make docs

# make everything
make all
```

# Installing
After building the software, either move or copy it to /usr/local/bin, or add the build path to your local $PATH environment variable.

## System-wide installation
```bash
# install to /usr/local/bin
cd build 
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
endlessh-report v1.2.1 - A simple report generator for endlessh tarpits.

Usage:
    endlessh-report
    endlessh-report [options]
    endlessh-report --syslog/var/log/syslog.1
    cat <file> | endlessh-report --stdin

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
|        372       |            7843            |           7894           |           51            |
```

Resulting in:

# Connection Statistics
| Total Unique IPs | Total Accepted Connections | Total Closed Connections | Total Alive Connections |
|------------------|----------------------------|--------------------------|-------------------------|
|        372       |            7843            |           7894           |           51            |

`endlessh-report -c` generates the following output:
(truncated for readability)

> **Note that these are all botnet-IPs and are already publicly listed as such.**

```markdown
# Statistics per IP
|          Host          | Accepted | Closed |
|------------------------|----------|--------|
|      218.92.0.200      |   1736   |  1736  |
|      218.92.0.208      |   2176   |  2226  |
|      218.92.0.221      |    33    |   33   |
|      219.255.1.177     |     1    |    1   |
|      220.135.2.173     |     1    |    1   |
|     220.83.201.167     |     1    |    1   |
|     221.131.34.170     |     1    |    1   |
|      221.154.190.9     |     1    |    1   |
|     222.252.20.109     |     1    |    1   |
|     222.64.180.116     |     1    |    1   |
|     223.171.91.191     |     7    |    7   |
|     223.84.249.154     |     1    |    1   |
|      27.74.253.80      |     1    |    1   |
|        3.6.86.93       |    41    |   41   |
|     31.222.238.213     |     1    |    1   |
|       31.24.10.71      |     1    |    1   |
|      34.105.187.71     |    15    |   15   |
|     34.133.201.109     |    20    |   20   |
```

Resulting in:


# Statistics per IP
|          Host          | Accepted | Closed |
|------------------------|----------|--------|
|      218.92.0.200      |   1736   |  1736  |
|      218.92.0.208      |   2176   |  2226  |
|      218.92.0.221      |    33    |   33   |
|      219.255.1.177     |     1    |    1   |
|      220.135.2.173     |     1    |    1   |
|     220.83.201.167     |     1    |    1   |
|     221.131.34.170     |     1    |    1   |
|      221.154.190.9     |     1    |    1   |
|     222.252.20.109     |     1    |    1   |
|     222.64.180.116     |     1    |    1   |
|     223.171.91.191     |     7    |    7   |
|     223.84.249.154     |     1    |    1   |
|      27.74.253.80      |     1    |    1   |
|        3.6.86.93       |    41    |   41   |
|     31.222.238.213     |     1    |    1   |
|       31.24.10.71      |     1    |    1   |
|      34.105.187.71     |    15    |   15   |
|     34.133.201.109     |    20    |   20   |

# Detailed Statistics
Since version v1.1.0 endlessh-report now allows for more detailed reports to be generated.
These include factors such as bot time wasted, and total bytes sent.

Examples are:

## Detailed IP Statistics

```markdown
# Statistics per IP
|          Host          | Accepted | Closed | Total Time (s) | Total Bytes |
|------------------------|----------|--------|----------------|-------------|
|      218.92.0.208      |   2176   |  2226  | 64d 15h 33m 56s|  685.00MiB  |
|      179.60.147.99     |    534   |   534  |   2h 52m 10s   |   1.00MiB   |
|      61.177.173.49     |    62    |   62   |  1d 12h 1m 18s |  15.00MiB   |
|      218.92.0.221      |    33    |   33   |   8h 39m 45s   |   3.00MiB   |
|      61.177.172.98     |    48    |   48   |   19h 38m 37s  |   8.00MiB   |
|      61.177.173.47     |    56    |   56   | 1d 16h 44m 50s |  17.00MiB   |
|      210.97.53.178     |     2    |    2   |       6s       |    333B     |
|      61.177.173.48     |    31    |   31   |     55m 10s    |  298.00KiB  |
|      61.177.173.53     |    72    |   72   |  2d 6h 20m 48s |  24.00MiB   |
|      61.177.173.39     |    53    |   53   | 1d 13h 17m 29s |  16.00MiB   |
|     61.177.172.108     |    45    |   45   |    8h 4m 45s   |   3.00MiB   |
|      61.177.173.52     |    39    |   39   |   8h 28m 24s   |   3.00MiB   |
|     61.177.172.104     |    35    |   35   |   13h 28m 37s  |   5.00MiB   |
|      61.177.172.19     |    48    |   48   |   23h 57m 54s  |  10.00MiB   |
|      61.177.173.51     |    55    |   55   |  4d 6h 59m 36s |  45.00MiB   |
|      61.177.173.50     |    67    |   67   |    4d 9h 26s   |  46.00MiB   |
|      61.177.172.90     |    53    |   53   |  1d 2h 14m 29s |  11.00MiB   |
|      61.177.173.46     |    56    |   56   | 4d 14h 44m 33s |  48.00MiB   |
|     61.177.172.124     |    38    |   38   |    1d 2h 57s   |  11.00MiB   |
|      61.177.173.36     |    49    |   49   |  2d 1h 8m 47s  |  21.00MiB   |
|     185.196.220.32     |    15    |   15   |       52s      |   5.00KiB   |
|      141.98.10.154     |    49    |   49   |     2m 56s     |  16.00KiB   |
|      61.177.173.35     |    45    |   45   |  1d 5h 21m 34s |  12.00MiB   |
|      64.62.197.197     |     2    |    2   |       8s       |    668B     |
```

Resulting in:

### Statistics per IP
|          Host          | Accepted | Closed | Total Time (s) | Total Bytes |
|------------------------|----------|--------|----------------|-------------|
|      218.92.0.208      |   2176   |  2226  | 64d 15h 33m 56s|  685.00MiB  |
|      179.60.147.99     |    534   |   534  |   2h 52m 10s   |   1.00MiB   |
|      61.177.173.49     |    62    |   62   |  1d 12h 1m 18s |  15.00MiB   |
|      218.92.0.221      |    33    |   33   |   8h 39m 45s   |   3.00MiB   |
|      61.177.172.98     |    48    |   48   |   19h 38m 37s  |   8.00MiB   |
|      61.177.173.47     |    56    |   56   | 1d 16h 44m 50s |  17.00MiB   |
|      210.97.53.178     |     2    |    2   |       6s       |    333B     |
|      61.177.173.48     |    31    |   31   |     55m 10s    |  298.00KiB  |
|      61.177.173.53     |    72    |   72   |  2d 6h 20m 48s |  24.00MiB   |
|      61.177.173.39     |    53    |   53   | 1d 13h 17m 29s |  16.00MiB   |
|     61.177.172.108     |    45    |   45   |    8h 4m 45s   |   3.00MiB   |
|      61.177.173.52     |    39    |   39   |   8h 28m 24s   |   3.00MiB   |
|     61.177.172.104     |    35    |   35   |   13h 28m 37s  |   5.00MiB   |
|      61.177.172.19     |    48    |   48   |   23h 57m 54s  |  10.00MiB   |
|      61.177.173.51     |    55    |   55   |  4d 6h 59m 36s |  45.00MiB   |
|      61.177.173.50     |    67    |   67   |    4d 9h 26s   |  46.00MiB   |
|      61.177.172.90     |    53    |   53   |  1d 2h 14m 29s |  11.00MiB   |
|      61.177.173.46     |    56    |   56   | 4d 14h 44m 33s |  48.00MiB   |
|     61.177.172.124     |    38    |   38   |    1d 2h 57s   |  11.00MiB   |
|      61.177.173.36     |    49    |   49   |  2d 1h 8m 47s  |  21.00MiB   |
|     185.196.220.32     |    15    |   15   |       52s      |   5.00KiB   |
|      141.98.10.154     |    49    |   49   |     2m 56s     |  16.00KiB   |
|      61.177.173.35     |    45    |   45   |  1d 5h 21m 34s |  12.00MiB   |
|      64.62.197.197     |     2    |    2   |       8s       |    668B     |

## Detailed Connection Statistics
```markdown
# Connection Statistics
| Total Unique IPs | Total Accepted Connections | Total Closed Connections | Total Alive Connections | Total Bot Time Wasted | Total Bytes Sent |
|------------------|----------------------------|--------------------------|-------------------------|-----------------------|------------------|
|        372       |            7843            |           7895           |           52            |     180d 35m 12s      |    1893.00MiB    |
```

Resulting in:

### Connection Statistics
| Total Unique IPs | Total Accepted Connections | Total Closed Connections | Total Alive Connections | Total Bot Time Wasted | Total Bytes Sent |
|------------------|----------------------------|--------------------------|-------------------------|-----------------------|------------------|
|        372       |            7843            |           7895           |           52            |     180d 35m 12s      |    1893.00MiB    |

## Detailed AbuseIPDB CSV format
```csv
IP,Categories,ReportDate,Comment
218.92.0.208,"18,14,22,15",2022-10-15T22:43:11Z,"218.92.0.208 fell into Endlessh tarpit; 50/2276 total connections are currently still open. Total time wasted: 64d 15h 33m 56s. Total bytes sent by tarpit: 685.00MiB. Report generated by Endlessh Report Generator v1.2.1"
179.60.147.99,"18,14,22,15",2022-10-15T22:43:11Z,"179.60.147.99 fell into Endlessh tarpit; 0/534 total connections are currently still open. Total time wasted: 2h 52m 10s. Total bytes sent by tarpit: 1.00MiB. Report generated by Endlessh Report Generator v1.2.1"
61.177.173.49,"18,14,22,15",2022-10-15T22:43:11Z,"61.177.173.49 fell into Endlessh tarpit; 0/62 total connections are currently still open. Total time wasted: 1d 12h 1m 18s. Total bytes sent by tarpit: 15.00MiB. Report generated by Endlessh Report Generator v1.2.1"
218.92.0.221,"18,14,22,15",2022-10-15T22:43:11Z,"218.92.0.221 fell into Endlessh tarpit; 0/33 total connections are currently still open. Total time wasted: 8h 39m 45s. Total bytes sent by tarpit: 3.00MiB. Report generated by Endlessh Report Generator v1.2.1"
61.177.172.98,"18,14,22,15",2022-10-15T22:43:11Z,"61.177.172.98 fell into Endlessh tarpit; 0/48 total connections are currently still open. Total time wasted: 19h 38m 37s. Total bytes sent by tarpit: 8.00MiB. Report generated by Endlessh Report Generator v1.2.1"
61.177.173.47,"18,14,22,15",2022-10-15T22:43:11Z,"61.177.173.47 fell into Endlessh tarpit; 0/56 total connections are currently still open. Total time wasted: 1d 16h 44m 50s. Total bytes sent by tarpit: 17.00MiB. Report generated by Endlessh Report Generator v1.2.1"
210.97.53.178,"18,14,22,15",2022-10-15T22:43:11Z,"210.97.53.178 fell into Endlessh tarpit; 0/2 total connections are currently still open. Total time wasted: 6s. Total bytes sent by tarpit: 333B. Report generated by Endlessh Report Generator v1.2.1"
61.177.173.48,"18,14,22,15",2022-10-15T22:43:11Z,"61.177.173.48 fell into Endlessh tarpit; 0/31 total connections are currently still open. Total time wasted: 55m 10s. Total bytes sent by tarpit: 298.00KiB. Report generated by Endlessh Report Generator v1.2.1"
61.177.173.53,"18,14,22,15",2022-10-15T22:43:11Z,"61.177.173.53 fell into Endlessh tarpit; 0/72 total connections are currently still open. Total time wasted: 2d 6h 20m 48s. Total bytes sent by tarpit: 24.00MiB. Report generated by Endlessh Report Generator v1.2.1"
45.141.84.126,"18,14,22,15",2022-10-15T22:43:11Z,"45.141.84.126 fell into Endlessh tarpit; 2/13 total connections are currently still open. Total time wasted: 10d 4h 5m 29s. Total bytes sent by tarpit: 107.00MiB. Report generated by Endlessh Report Generator v1.2.1"
```
