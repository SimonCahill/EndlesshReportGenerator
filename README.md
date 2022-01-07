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

## Arguments

```
Usage: ./unique_ids
Usage: ./unique_ids [options]
Usage: cat file | ./unique_ids--stdin

Switches:
        --no-ip-stats, -i       Don't print IP statistics
        --no-cn-stats, -c       Don't print connection statistics
        --stdin                 Read logs from stdin
Arguments:
        --syslog </path/to>     Override default syslog path (/var/log/syslog)
```

## Output
This program outputs its text in Markdown-compatible form.
Because I wrote this program for my own use on my servers, I developed it for use with my report scripts that send me periodic emails.

## Recommended use

```bash
# requires pandoc
./unique_ids | pandoc -f markdown -t html | mail -a "Content-Type: text/html; charset=UTF-8" -s "My Endlessh Report" "you@yourdomain.com"
```

# Sample Output
`unique_ids -i` generates the following output:

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

`unique_ids -c` generates the following output:
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
|   ::ffff:141.98.10.63  |    32    |   32   |
|   ::ffff:141.98.11.16  |    28    |   28   |
|   ::ffff:198.98.62.88  |     1    |    0   |
|  ::ffff:209.141.43.186 |    13    |   13   |
|  ::ffff:221.131.165.33 |    12    |   12   |
|  ::ffff:221.131.165.50 |    12    |   12   |
|  ::ffff:221.131.165.56 |    16    |   16   |
|  ::ffff:221.131.165.62 |    12    |   12   |
|  ::ffff:221.131.165.65 |    10    |   10   |
|  ::ffff:221.131.165.75 |    26    |   25   |
| ::ffff:221.181.185.111 |    14    |   14   |
| ::ffff:221.181.185.143 |    25    |   25   |
| ::ffff:221.181.185.151 |    10    |    9   |
| ::ffff:221.181.185.159 |    33    |   32   |
|  ::ffff:221.181.185.94 |     9    |    9   |
| ::ffff:222.121.190.122 |     2    |    2   |
| ::ffff:222.186.180.130 |    13    |   13   |
|  ::ffff:222.186.30.112 |    33    |   33   |
|   ::ffff:222.186.42.7  |    20    |   20   |
|  ::ffff:222.187.232.39 |    10    |   10   |
|  ::ffff:222.187.238.58 |    12    |   12   |
|  ::ffff:222.187.254.41 |    22    |   22   |
|   ::ffff:45.141.84.10  |     1    |    0   |
|  ::ffff:45.154.255.147 |     1    |    1   |
|  ::ffff:45.155.204.161 |    62    |   62   |
|   ::ffff:45.67.14.25   |     1    |    1   |
|   ::ffff:45.67.14.29   |     2    |    2   |
|   ::ffff:46.19.139.18  |    22    |   22   |
|  ::ffff:49.235.112.157 |     2    |    2   |
|   ::ffff:49.88.112.77  |    97    |   97   |
|   ::ffff:62.233.50.53  |    17    |   17   |
|  ::ffff:92.255.85.135  |    232   |   232  |
|  ::ffff:92.255.85.146  |    30    |   30   |
|  ::ffff:92.255.85.237  |    107   |   107  |
|   ::ffff:92.255.85.28  |    28    |   28   |
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
|   ::ffff:141.98.10.63  |    32    |   32   |
|   ::ffff:141.98.11.16  |    28    |   28   |
|   ::ffff:198.98.62.88  |     1    |    0   |
|  ::ffff:209.141.43.186 |    13    |   13   |
|  ::ffff:221.131.165.33 |    12    |   12   |
|  ::ffff:221.131.165.50 |    12    |   12   |
|  ::ffff:221.131.165.56 |    16    |   16   |
|  ::ffff:221.131.165.62 |    12    |   12   |
|  ::ffff:221.131.165.65 |    10    |   10   |
|  ::ffff:221.131.165.75 |    26    |   25   |
| ::ffff:221.181.185.111 |    14    |   14   |
| ::ffff:221.181.185.143 |    25    |   25   |
| ::ffff:221.181.185.151 |    10    |    9   |
| ::ffff:221.181.185.159 |    33    |   32   |
|  ::ffff:221.181.185.94 |     9    |    9   |
| ::ffff:222.121.190.122 |     2    |    2   |
| ::ffff:222.186.180.130 |    13    |   13   |
|  ::ffff:222.186.30.112 |    33    |   33   |
|   ::ffff:222.186.42.7  |    20    |   20   |
|  ::ffff:222.187.232.39 |    10    |   10   |
|  ::ffff:222.187.238.58 |    12    |   12   |
|  ::ffff:222.187.254.41 |    22    |   22   |
|   ::ffff:45.141.84.10  |     1    |    0   |
|  ::ffff:45.154.255.147 |     1    |    1   |
|  ::ffff:45.155.204.161 |    62    |   62   |
|   ::ffff:45.67.14.25   |     1    |    1   |
|   ::ffff:45.67.14.29   |     2    |    2   |
|   ::ffff:46.19.139.18  |    22    |   22   |
|  ::ffff:49.235.112.157 |     2    |    2   |
|   ::ffff:49.88.112.77  |    97    |   97   |
|   ::ffff:62.233.50.53  |    17    |   17   |
|  ::ffff:92.255.85.135  |    232   |   232  |
|  ::ffff:92.255.85.146  |    30    |   30   |
|  ::ffff:92.255.85.237  |    107   |   107  |
|   ::ffff:92.255.85.28  |    28    |   28   |