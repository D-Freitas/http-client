#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdbool.h>

void error(const char *msg) {
  perror(msg); exit(0);
}

void verify_if_args_passed_correctly(const int argc, char *argv[]) {
  if (argc != 4) {
    fprintf(stderr, "Usage: %s <hostname> <port> <resource>\n", argv[0]);
    exit(1);
  }
}

void verify_if_socket_is_open(int sockfd) {
  if (sockfd < 0) {
    error("Could not open socket");
  }
}

int get_bits(int bytes) {
  int bits = 0;
  while (bits / 8 != bytes) {
    bits += 8;
  }
  return bits;
}

int main(int argc, char* argv[]) {
  verify_if_args_passed_correctly(argc, argv);

  int bytes;

  struct hostent *server;
  struct sockaddr_in serv_addr;
  int port = atoi(argv[2]);
  char* host = argv[1];
  
  server = gethostbyname(host);
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

  char buffer[get_bits(128)];
  strcpy(buffer, "GET ");
  strcat(buffer, argv[3]);
  strcat(buffer, " HTTP/1.0\r\n\r\n");

  char message[get_bits(128)];
  char response[get_bits(512)];

  sprintf(message, buffer, argv[1], argv[2]);
  printf("Request: \n%s\n",message);

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  verify_if_socket_is_open(sockfd);

  bool is_server_null = server == NULL;
  bool is_connected = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0;
  if (is_server_null) {
    error("No such host");
  }
  if (is_connected) {
    error("Could not connect");
  }
  
  int total = strlen(message);

  for (int sent = 0; sent < total; sent += bytes) {
    bytes = write(sockfd, message + sent, total - sent);
    if (bytes < 0) {
      error("Could not writing message to socket");
    }
  }

  memset(response, 0, sizeof(response));
  total = sizeof(response) - 1;

  int received = 0;
  while (received < total) {
    bytes = read(sockfd, response + received, total - received);
    if (bytes < 0) {
      error("Could not writing message to socket");
    }
    if (bytes == 0) {
      break;
    }
    received += bytes;
  }
  
  if (received == total) {
    error("Could not storing complete response from socket");
  }
  close(sockfd);
  
  printf("Response:\n%s\n", response);
  return 0;
}