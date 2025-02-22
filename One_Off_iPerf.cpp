#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define CHUNK_SIZE 1000

void server(int argc, char *argv[]);
void client(int argc, char *argv[]);

int main(int argc, char *argv[]) {
  try {
    if (argc < 2) {
      std::cout << "Error: missing or extra arguments" << std::endl;
      return EXIT_FAILURE;
    } else if (std::strcmp(argv[1], "-s") == 0) {
      server(argc, argv);
    } else {
      client(argc, argv);
    }
  } catch (const std::exception &e) {
    std::cout << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

void server(int argc, char *argv[]) {
  int listen_port;
  if (argc != 4) {
    std::cout << "Error: missing or extra arguments" << std::endl;
    std::quick_exit(EXIT_FAILURE);
  } else if ((listen_port = std::atoi(argv[3])) == 0) {
    std::cout << "Error: port number must be in the range of [1024, 65535]"
              << std::endl;
    std::quick_exit(EXIT_FAILURE);
  } else if (1024 > listen_port || listen_port > 65535) {
    std::cout << "Error: port number must be in the range of [1024, 65535]"
              << std::endl;
    std::quick_exit(EXIT_FAILURE);
  }

  // (1) Create socket
  int sockfd;
  if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
    std::cout << "Error opening stream socket" << std::endl;
    std::quick_exit(EXIT_FAILURE);
  }

  // (2) Set the "reuse port" socket option
  const int enable{1};
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) ==
      -1) {
    std::cout << "Error setting socket options" << std::endl;
    std::quick_exit(EXIT_FAILURE);
  }

  // (3) Create a sockaddr_in struct for the proper port and bind() to it.
  struct sockaddr_in addr;
  std::memset(&addr, 0, sizeof(addr));

  // (3.1): specify socket family.
  // This is an IPv4 socket.
  addr.sin_family = AF_INET;

  // (3.2): specify client address whitelist (hostname).
  // The socket will be a server listening to everybody.
  addr.sin_addr.s_addr = INADDR_ANY;

  // (3.3): Set the port value.
  // If port is 0, the OS will choose the port for us.
  // Use htons to convert from local byte order to network byte order.
  addr.sin_port = htons(static_cast<uint16_t>(listen_port));

  // (3.4) Bind to the port.
  if (bind(sockfd, (sockaddr *)&addr, sizeof(addr)) == -1) {
    std::cout << "Error binding stream socket" << std::endl;
    std::quick_exit(EXIT_FAILURE);
  }

  // (4) Begin listening for THE client.
  if (listen(sockfd, 1) == -1) {
    std::cout << "Error listening for THE client" << std::endl;
    std::quick_exit(EXIT_FAILURE);
  }

  // (5) Accept THE client.
  struct sockaddr_in client_addr;
  socklen_t client_addr_len{sizeof(client_addr)};
  int client_sockfd =
      accept(sockfd, (sockaddr *)&client_addr, &client_addr_len);
  if (client_sockfd == -1) {
    std::cout << "Error accepting connection" << std::endl;
    std::quick_exit(EXIT_FAILURE);
  }
  close(sockfd);

  // (6) Keep receiving till FIN.
  std::size_t total_received{};
  unsigned char buf[CHUNK_SIZE]{};
  time_t start{time(NULL)};
  while (true) {
    long received{recv(client_sockfd, &buf, sizeof(buf), 0)};
    if (received == -1) {
      std::cout << "Error reading stream message" << std::endl;
      std::quick_exit(EXIT_FAILURE);
    } else if (buf[received - 1] == 1) {
      total_received += received - 1;
      break;
    }
    total_received += received;
  }

  // (7) Send ACK.
  const unsigned char ACK{1};
  if (send(client_sockfd, &ACK, sizeof(ACK), 0) == -1) {
    std::cout << "Error sending on stream socket" << std::endl;
    std::quick_exit(EXIT_FAILURE);
  }

  // (8) Wait till THE client closes.
  if (recv(client_sockfd, &buf, sizeof(buf), 0) == -1) {
    std::cout << "Error reading stream message" << std::endl;
    std::quick_exit(EXIT_FAILURE);
  }
  time_t end{time(NULL)};

  // (9) Print summary (Received=X KB, Rate=Y Mbps).
  std::cout << "Received=" << total_received / 1000 << " KB, Rate=";
  printf("%.3f", (static_cast<double>(total_received) / 1e6) * 8 /
                     static_cast<double>(end - start));
  std::cout << " Mbps" << std::endl;

  // (10) Close connection.
  close(client_sockfd);
}

void client(int argc, char *argv[]) {
  const char *const server_hostname{argv[3]};
  int server_port, duration;
  if (argc != 8) {
    std::cout << "Error: missing or extra arguments" << std::endl;
    std::quick_exit(EXIT_FAILURE);
  } else if ((server_port = std::atoi(argv[5])) == 0) {
    std::cout << "Error: port number must be in the range of [1024, 65535]"
              << std::endl;
    std::quick_exit(EXIT_FAILURE);
  } else if (1024 > server_port || server_port > 65535) {
    std::cout << "Error: port number must be in the range of [1024, 65535]"
              << std::endl;
    std::quick_exit(EXIT_FAILURE);
  } else if ((duration = std::atoi(argv[7])) == 0) {
    std::cout << "Error: time argument must be greater than 0" << std::endl;
    std::quick_exit(EXIT_FAILURE);
  } else if (duration <= 0) {
    std::cout << "Error: time argument must be greater than 0" << std::endl;
    std::quick_exit(EXIT_FAILURE);
  }

  // (1) Create socket.
  int sockfd;
  if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
    std::cout << "Error opening stream socket" << std::endl;
    std::quick_exit(EXIT_FAILURE);
  }

  // (2) Create a sockaddr_in to specify remote host and port.
  struct sockaddr_in addr;
  std::memset(&addr, 0, sizeof(addr));

  // (2.1): specify socket family.
  // This is an IPv4 socket.
  addr.sin_family = AF_INET;

  // (2.2): specify remote socket address (hostname).
  // The socket will be a client, so call this unix helper function
  // to convert a hostname string to a useable `hostent` struct.
  struct hostent *host{};
  if ((host = gethostbyname(server_hostname)) == nullptr) {
    std::cout << server_hostname << ": unknown host" << std::endl;
    std::quick_exit(EXIT_FAILURE);
  }
  std::memcpy(&(addr.sin_addr), host->h_addr, host->h_length);

  // (2.3): Set the port value.
  // Use htons to convert from local byte order to network byte order.
  addr.sin_port = htons(static_cast<uint16_t>(server_port));

  // (3) Connect to remote server.
  if (connect(sockfd, (sockaddr *)&addr, sizeof(addr)) == -1) {
    std::cout << "Error connecting stream socket" << std::endl;
    std::quick_exit(EXIT_FAILURE);
  }

  // (4) Send chunks of <CHUNK_SIZE> bytes for <time> seconds.
  std::size_t total_sent{};
  unsigned char buf[CHUNK_SIZE]{};
  time_t start{time(NULL)};
  do {
    long sent{send(sockfd, &buf, sizeof(buf), 0)};
    if (sent == -1) {
      std::cout << "Error sending on stream socket" << std::endl;
      std::quick_exit(EXIT_FAILURE);
    }
    total_sent += sent;
  } while ((time(NULL) - start) < duration);

  // (5) Send FIN
  unsigned char FIN{1};
  if (send(sockfd, &FIN, sizeof(FIN), 0) == -1) {
    std::cout << "Error sending on stream socket" << std::endl;
    std::quick_exit(EXIT_FAILURE);
  }

  // (6) Wait for ACK.
  if (recv(sockfd, &FIN, sizeof(FIN), 0) == -1) {
    std::cout << "Error reading stream message" << std::endl;
    std::quick_exit(EXIT_FAILURE);
  }
  time_t end{time(NULL)};

  // (6) Print summary (Sent=X KB, Rate=Y Mbps).
  std::cout << "Sent=" << total_sent / 1000 << " KB, Rate=";
  printf("%.3f", (static_cast<double>(total_sent) / 1e6) * 8 /
                     static_cast<double>(end - start));
  std::cout << " Mbps" << std::endl;

  // (7) Close connection.
  close(sockfd);
}
