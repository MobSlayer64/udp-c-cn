// Real-Time Remote Chat Client
// Usage: gcc remoteclient.c -o remoteclient -lpthread
//        ./remoteclient <server_ip> [port]   (default port: 9090)
//
// Replace <server_ip> with the server machine's IP address.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

#define DEFAULT_PORT 9090
#define MAXLINE      1024

int sockfd = -1;

// ── Thread: receive messages from server ─────────────────────────────────────
void *recv_thread(void *arg) {
    char buffer[MAXLINE];
    int n;

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        n = recv(sockfd, buffer, MAXLINE - 1, 0);
        if (n <= 0) {
            printf("\n[!] Server disconnected.\n");
            close(sockfd);
            exit(0);
        }
        if (strcmp(buffer, "/quit") == 0) {
            printf("\n[!] Server closed the chat.\n");
            close(sockfd);
            exit(0);
        }
        printf("\r\033[K[Server]: %s\n> ", buffer);
        fflush(stdout);
    }
    return NULL;
}

// ── Thread: read from stdin and send to server ────────────────────────────────
void *send_thread(void *arg) {
    char buffer[MAXLINE];

    while (1) {
        printf("> ");
        fflush(stdout);

        if (!fgets(buffer, MAXLINE, stdin)) break;

        buffer[strcspn(buffer, "\n")] = '\0';
        if (strlen(buffer) == 0) continue;

        if (strcmp(buffer, "/quit") == 0) {
            send(sockfd, "/quit", 5, 0);
            printf("[!] You left the chat.\n");
            close(sockfd);
            exit(0);
        }

        if (send(sockfd, buffer, strlen(buffer), 0) < 0) {
            perror("send failed");
            break;
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    struct sockaddr_in server_addr;
    int port = DEFAULT_PORT;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <server_ip> [port]\n", argv[0]);
        fprintf(stderr, "Example: %s 192.168.1.10 9090\n", argv[0]);
        exit(1);
    }

    if (argc >= 3) port = atoi(argv[2]);

    signal(SIGPIPE, SIG_IGN);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { perror("socket failed"); exit(1); }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(port);

    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "[!] Invalid IP address: %s\n", argv[1]);
        exit(1);
    }

    printf("╔══════════════════════════════════════╗\n");
    printf("║     Remote Chat Client  v1.0         ║\n");
    printf("╚══════════════════════════════════════╝\n");
    printf("[*] Connecting to %s:%d ...\n", argv[1], port);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("[!] Connection failed");
        exit(1);
    }

    printf("[+] Connected to server!\n");
    printf("[*] Type /quit to exit.\n");
    printf("────────────────────────────────────────\n\n");

    pthread_t tid_recv, tid_send;
    pthread_create(&tid_recv, NULL, recv_thread, NULL);
    pthread_create(&tid_send, NULL, send_thread, NULL);

    pthread_join(tid_recv, NULL);
    pthread_join(tid_send, NULL);

    return 0;
}
