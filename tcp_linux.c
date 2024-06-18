#include "tcp_linux.h"

int sock;
struct sockaddr_in server_addr;
char buffer[1024];
int bytes_read;
fd_set readfds;
struct timeval timeout;

void read_from_server(char* returnBuffer) {
    int bytes_read;

    // Initialize the file descriptor set
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);
    char buffer[1024] = "";


    // Wait for data to be available on the socket
    if (select(sock + 1, &readfds, NULL, NULL, &timeout) > 0){
        if (FD_ISSET(sock, &readfds)) {
            // Data is available to read
            bytes_read = read(sock, buffer, sizeof(buffer) - 1);
            if (bytes_read < 0) {
                perror("Read error");
            } else {
                buffer[bytes_read] = '\0';
                strcpy(returnBuffer, buffer);
                // printf("Server response: %s\n", buffer);
            }
        }
    }

}

void send_to_server(char* message) {
    send(sock, message, strlen(message), 0);
    // printf("Message sent to server\n");
}

bool init() {
    // int sock;
    // struct sockaddr_in server_addr;
    // fd_set readfds;
    // struct timeval timeout;
    printf("Connecting to server...\n");
    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return false;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    // Get the IP address of the server
    char ip[100];
    struct hostent *he;
    struct in_addr **addr_list;
    if ((he = gethostbyname(SERVER_IP)) == NULL) {
        perror("Invalid address or Address not supported");
        return false;
    }
    addr_list = (struct in_addr **)he->h_addr_list;
    strcpy(ip, inet_ntoa(*addr_list[0]));

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address or Address not supported");
        return false;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return false;
    }

    // Send a message to the server
    const char *message = "tetris connect";
    send(sock, message, strlen(message), 0);
    // printf("Message sent to server\n");

    // Set up the timeout
    timeout.tv_sec = 0;
    timeout.tv_usec = 1000000; // 20 milliseconds

    printf("Waiting for player...\n");

    while (true) {
        char response[1024] = "";
        read_from_server(response);
        if (strstr(response, "OK") != NULL){
            break;
        }
        if (strstr(response, "SERVERFULL") != NULL){
            printf("Server is full, please connect later.\n");
            return false;
        }
    }

    return true;
}