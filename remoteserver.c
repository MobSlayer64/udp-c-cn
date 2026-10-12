// Real-Time Remote Chat Server
// Usage: gcc remoteserver.c -o remoteserver -lpthread
//        ./remoteserver [port]   (default port: 9090)
//
// Share your IP address with the client to connect.

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
#define NAME_LEN     32

int client_sock = -1;

// ── Thread: receive messages from remote client ──────────────────────────────
void *recv_thread(void *arg) {
    char buffer[MAXLINE];
    int n;

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        n = recv(client_sock, buffer, MAXLINE - 1, 0);
        if (n <= 0) {
            printf("\n[!] Client disconnected.\n");
            close(client_sock);
            exit(0);
        }
        if (strcmp(buffer, "/quit") == 0) {
            printf("\n[!] Client left the chat.\n");
            close(client_sock);
            exit(0);
        }
        printf("\r\033[K[Client]: %s\n> ", buffer);
        fflush(stdout);
    }
    return NULL;
}

// ── Thread: read from stdin and send to remote client ────────────────────────
void *send_thread(void *arg) {
    char buffer[MAXLINE];

    while (1) {
        printf("> ");
        fflush(stdout);

        if (!fgets(buffer, MAXLINE, stdin)) break;

        buffer[strcspn(buffer, "\n")] = '\0';
        if (strlen(buffer) == 0) continue;

        if (strcmp(buffer, "/quit") == 0) {
            send(client_sock, "/quit", 5, 0);
            printf("[!] You left the chat.\n");
            close(client_sock);
            exit(0);
        }

        if (send(client_sock, buffer, strlen(buffer), 0) < 0) {
            perror("send failed");
            break;
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    int server_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    int port = DEFAULT_PORT;
    char client_ip[INET_ADDRSTRLEN];

    if (argc >= 2) port = atoi(argv[1]);

    signal(SIGPIPE, SIG_IGN);

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) { perror("socket failed"); exit(1); }

    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port        = htons(port);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed"); exit(1);
    }

    if (listen(server_sock, 1) < 0) {
        perror("listen failed"); exit(1);
    }

    printf("╔══════════════════════════════════════╗\n");
    printf("║     Remote Chat Server  v1.0         ║\n");
    printf("╚══════════════════════════════════════╝\n");
    printf("[*] Listening on port %d ...\n", port);
    printf("[*] Share your IP with the client to connect.\n");
    printf("[*] Type /quit to exit.\n\n");

    client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
    if (client_sock < 0) { perror("accept failed"); exit(1); }

    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    printf("[+] Client connected from %s\n", client_ip);
    printf("────────────────────────────────────────\n\n");

    close(server_sock);

    pthread_t tid_recv, tid_send;
    pthread_create(&tid_recv, NULL, recv_thread, NULL);
    pthread_create(&tid_send, NULL, send_thread, NULL);

    pthread_join(tid_recv, NULL);
    pthread_join(tid_send, NULL);

    return 0;
}
