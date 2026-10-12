/*
 * UDP Server Program
 * Operations:
 *   a. String Conversion from UpperCase to LowerCase
 *   b. Reverse the String
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Function to convert string to lowercase
void to_lowercase(char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
}

// Function to reverse a string
void reverse_string(char *str) {
    int len = strlen(str);
    for (int i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;
    }
}

int main() {
    int sockfd;
    char buffer[BUFFER_SIZE];
    char result[BUFFER_SIZE];
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len = sizeof(cliaddr);

    // [1] Creating UDP socket (SOCK_DGRAM)
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("[1] UDP socket created successfully.\n");

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // [2] Setting server address configuration
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // [3] Bind the socket with the server address
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    printf("[2] Socket bound to port %d.\n", PORT);
    printf("[3] Server is listening for incoming UDP datagrams...\n\n");

    while (1) {
        // [4] Receive message from client
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                         (struct sockaddr *)&cliaddr, &len);
        if (n < 0) {
            perror("recvfrom failed");
            continue;
        }
        buffer[n] = '\0';

        // Check for exit signal
        if (strcmp(buffer, "EXIT") == 0) {
            printf("Client [%s:%d] disconnected.\n",
                   inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
            continue;
        }

        // Expected format: "<choice>:<message>"
        // e.g., "1:HELLO WORLD" or "2:HELLO WORLD"
        char choice = buffer[0];
        char *input_str = buffer + 2; // skip "<choice>:"

        printf("--------------------------------------------------\n");
        printf("Received datagram from [%s:%d]\n",
               inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
        printf("Raw Packet: %s\n", buffer);

        strncpy(result, input_str, BUFFER_SIZE - 1);
        result[BUFFER_SIZE - 1] = '\0';

        if (choice == '1') {
            printf("Requested Operation: [a] UpperCase to LowerCase\n");
            printf("Original String:     %s\n", input_str);
            to_lowercase(result);
            printf("Processed String:    %s\n", result);
        } else if (choice == '2') {
            printf("Requested Operation: [b] Reverse the String\n");
            printf("Original String:     %s\n", input_str);
            reverse_string(result);
            printf("Processed String:    %s\n", result);
        } else {
            printf("Unknown Operation requested.\n");
            strcpy(result, "Error: Invalid operation code");
        }

        // [5] Send processed response back to client
        sendto(sockfd, result, strlen(result), 0,
               (const struct sockaddr *)&cliaddr, len);
        printf("Response sent back to client.\n");
        printf("--------------------------------------------------\n\n");
    }

    close(sockfd);
    return 0;
}
