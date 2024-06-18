#ifndef TCP_LINUX_H
#define TCP_LINUX_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <sys/types.h>
#include <netdb.h>

#define SERVER_IP "codingbear.mcloudtw.com" // Replace with your server's IP address
#define SERVER_PORT 6060  // Replace with your server's port

extern int sock;
extern struct sockaddr_in server_addr;
extern char buffer[1024];
extern int bytes_read;
extern fd_set readfds;

void read_from_server(char* buffer);
bool init();

#endif