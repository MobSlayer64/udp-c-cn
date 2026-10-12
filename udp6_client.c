/*
 * UDP Client Program
 * Operations:
 *   a. String Conversion from UpperCase to LowerCase
 *   b. Reverse the String
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in servaddr;
    char buffer[BUFFER_SIZE];
    char packet[BUFFER_SIZE + 32];
    char message[BUFFER_SIZE];
    int choice;
    socklen_t len = sizeof(servaddr);

    // [1] Creating UDP socket (SOCK_DGRAM)
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("[1] UDP socket created successfully.\n");

    memset(&servaddr, 0, sizeof(servaddr));

    // [2] Setting server address details
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_IP, &servaddr.sin_addr) <= 0) {
        perror("Invalid server address");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    while (1) {
        printf("\n========================================\n");
        printf("           UDP STRING OPERATIONS        \n");
        printf("========================================\n");
        printf("1. Convert String: UpperCase -> LowerCase\n");
        printf("2. Reverse the String\n");
        printf("3. Exit\n");
        printf("Enter your choice (1-3): ");

        if (scanf("%d", &choice) != 1) {
            // Clear invalid input buffer
            while (getchar() != '\n');
            printf("Invalid input. Please enter a valid number.\n");
            continue;
        }

        // Clear newline left in input buffer
        while (getchar() != '\n');

        if (choice == 3) {
            printf("Exiting...\n");
            // Notify server of exit
            sendto(sockfd, "EXIT", strlen("EXIT"), 0,
                   (const struct sockaddr *)&servaddr, len);
            break;
        }

        if (choice != 1 && choice != 2) {
            printf("Invalid choice! Please choose 1, 2, or 3.\n");
            continue;
        }

        printf("Enter the string: ");
        if (fgets(message, BUFFER_SIZE - 4, stdin) != NULL) {
            message[strcspn(message, "\n")] = '\0';
        }

        // Format message as: "<choice>:<message>"
        snprintf(packet, sizeof(packet), "%d:%s", choice, message);

        // [3] Send packet to server
        sendto(sockfd, packet, strlen(packet), 0,
               (const struct sockaddr *)&servaddr, len);
        printf("[2] Request sent to server.\n");

        // [4] Receive reply from server
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                         (struct sockaddr *)&servaddr, &len);
        if (n < 0) {
            perror("recvfrom failed");
            continue;
        }
        buffer[n] = '\0';

        // Display results
        printf("\n------------- Result -------------\n");
        if (choice == 1) {
            printf("Operation : UpperCase to LowerCase\n");
        } else {
            printf("Operation : Reverse String\n");
        }
        printf("Original  : %s\n", message);
        printf("Result    : %s\n", buffer);
        printf("----------------------------------\n");
    }

    close(sockfd);
    printf("[3] Client socket closed.\n");
    return 0;
}
