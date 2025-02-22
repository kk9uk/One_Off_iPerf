## One-off iPerf

> I'm glad that I learnt about Computer Networks from [Prof. Hong Xu (Henry)](https://henryhxu.github.io/). Henry gives comprehensive yet easy-to-follow lectures, which are always complemented with simple & clear animations. Most importantly, he & his teaching team pose interesting assignments that are not toy problems & actually relate to the real world. This is (sadly) not something common in universities & the reason why this repo exists.

When operating in client mode, `One_Off_iPerf` will send TCP packets to a specific host for a specified time window and track how much data was sent during that time frame; it will calculate and display the bandwidth based on how much data was sent in the elapsed time. When operating in server mode, `One_Off_iPerf` will receive TCP packets and track how much data was received during the lifetime of a connection; it will calculate and display the bandwidth based on how much data was received and how much time elapsed during the connection.

### Server Mode

To operate `One_Off_iPerf` in server mode, it should be invoked as follows:

`./One_Off_iPerf -s -p <listen_port>`

* `-s` indicates this is the `One_Off_iPerf` server which should consume data
* `listen_port` is the port on which the host is waiting to consume data; the port should be in the range `1024 ≤ listen_port ≤ 65535`

> For simplicity, it is assumed that these arguments will appear exactly in the order listed above.

If arguments are missing or extra arguments are provided, `One_Off_iPerf` prints the following and exit:

`Error: missing or extra arguments`

If the listen port argument is less than 1024 or greater than 65535, `One_Off_iPerf` prints the following and exit:

`Error: port number must be in the range of [1024, 65535]`

When running as a server, `One_Off_iPerf` listens for TCP connections from a client and receive data as quickly as possible. It then wait for some kind of message from the client indicating it is done sending data (we will call this a FIN message). The server then send the client an acknowledgement to this FIN message.

Data are read in chunks of 1000 bytes, and a running total of the number of bytes received is kept.

After the client has closed the connection, `One_Off_iPerf` server prints a one-line summary in the following format:

`Received=X KB, Rate=Y Mbps`

where X stands for the total number of bytes received (in kilobytes), and Y stands for the rate at which traffic could be read in megabits per second (Mbps).
Note X is an integer and Y is a decimal with three digits after the decimal mark.

For example:
`Received=6543 KB, Rate=5.234 Mbps`

The `One_Off_iPerf` server shuts down gracefully after it handles one connection from a client.

### Client Mode

To operate `One_Off_iPerf` in client mode, it should be invoked as follows:

`./One_Off_iPerf -c -h <server_hostname> -p <server_port> -t <time>`

* `-c` indicates this is the `One_Off_iPerf` client which should generate data
* `server_hostname` is the hostname or IP address of the `One_Off_iPerf` server which will consume data
* `server_port` is the port on which the remote host is waiting to consume data; the port should be in the range 1024 ≤ `server_port` ≤ 65535
* `time` is the duration in seconds for which data should be generated.

> For simplicity, it is assumed that these arguments will appear exactly in the order listed above.

If any arguments are missing or extra arguments are provided, `One_Off_iPerf` prints the following and exit:

`Error: missing or extra arguments`

If the server port argument is less than 1024 or greater than 65535, `One_Off_iPerf` prints the following and exit:

`Error: port number must be in the range of [1024, 65535]`

If the time argument ends up parsing to less than or equal to 0, `One_Off_iPerf` prints the following and exit:

`Error: time argument must be greater than 0`

If both the port and time argument are invalid, `One_Off_iPerf` prints only the port error message.

When running as a client, `One_Off_iPerf` establishs a TCP connection with the server and sends data as quickly as possible for `time` seconds. Data are sent in chunks of 1000 bytes and the data are all zeros. A running total of the number of bytes sent is kept. After the client finishes sending its data, it sends a FIN message and wait for an acknowledgement before exiting the program.

`One_Off_iPerf` client prints a one-line summary in the following format:

`Sent=X KB, Rate=Y Mbps`

where X stands for the total number of bytes sent (in kilobytes), and Y stands for the rate at which traffic could be read in megabits per second (Mbps).
Note X is an integer and Y is a decimal with three digits after the decimal mark.

For example:
`Sent=6543 KB, Rate=5.234 Mbps`

It is assumed that 1 kilobyte (KB) = 1000 bytes (B) and 1 megabyte (MB) = 1000 KB. As always, 1 byte (B) = 8 bits (b).

## Acknowledgements

This is for CSCI4430: Computer Networks, Spring 2025 of CUHK (my favorite course taken in undergraduate studies), designed by [@Yicheng Feng](mailto:yichengfeng@link.cuhk.edu.hk), which is based on [Mosharaf Chowdhury](http://www.mosharaf.com/)'s Umich EECS 489: Computer Networks.
