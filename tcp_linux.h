#ifndef TCP_LINUX_H
#define TCP_LINUX_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <stdbool.h>
#include <sys/types.h>

#define SERVER_IP "127.0.0.1" // Replace with your server's IP address
#define SERVER_PORT 8080      // Replace with your server's port

extern int sock;
extern struct sockaddr_in server_addr;
extern char buffer[1024];
extern int bytes_read;
extern fd_set readfds;

void read_from_server(char* buffer);
bool init();

#endif