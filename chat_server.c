//Chat Server


#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024
#define USERNAME_LEN 32

typedef struct {
    int socket;
    char username[USERNAME_LEN];
} client_t;

client_t clients[MAX_CLIENTS];
int client_count = 0;

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void *handle_client(void *arg);
void handle_signal(int sig);
void parse_config_file(const char *filename, int *port);
void add_client(int client_socket, const char *username);
void remove_client(int client_socket);
void broadcast_message(int sender_socket, const char *message);
void send_private_message(int sender_socket, const char *receiver_username, const char *message);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s configuration_file\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port;
    parse_config_file(argv[1], &port);

    signal(SIGINT, handle_signal);

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 5) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    socklen_t server_addr_len = sizeof(server_addr);
    if (getsockname(server_socket, (struct sockaddr *)&server_addr, &server_addr_len) == -1) {
        perror("getsockname");
        exit(EXIT_FAILURE);
    }

    printf("Server started on %s:%d\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    fd_set read_fds;
    FD_ZERO(&read_fds);

    while (1) {
        FD_SET(server_socket, &read_fds);

        int max_fd = server_socket;
        for (int i = 0; i < client_count; i++) {
            FD_SET(clients[i].socket, &read_fds);
            max_fd = (clients[i].socket > max_fd) ? clients[i].socket : max_fd;
        }

        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);

        if (activity == -1 && errno != EINTR) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(server_socket, &read_fds)) {
            int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
            if (client_socket == -1) {
                perror("accept");
                continue;
            }

            pthread_t thread_id;
            if (pthread_create(&thread_id, NULL, handle_client, (void *)&client_socket) != 0) {
perror("pthread_create");
close(client_socket);
continue;
}

        printf("Accepted connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    }

    for (int i = 0; i < client_count; i++) {
        if (FD_ISSET(clients[i].socket, &read_fds)) {
            char buffer[BUFFER_SIZE];
            memset(buffer, 0, BUFFER_SIZE);

            int recv_len = recv(clients[i].socket, buffer, BUFFER_SIZE - 1, 0);

            if (recv_len <= 0) {
                printf("Client %s disconnected\n", clients[i].username);
                remove_client(clients[i].socket);
                close(clients[i].socket);
                continue;
            }

            buffer[recv_len] = '\0';

            char command[BUFFER_SIZE];
            sscanf(buffer, "%s", command);

            if (strncmp(command, "login",5) == 0) {
                char username[USERNAME_LEN];
                int i;
                //sscanf(buffer, "%*s %s", username);
                int j=0;
                for(i=6;i<strlen(command);i++){
                    username[j]=command[i];
                    j++;
                    printf("*");
                }
                add_client(clients[i].socket, username);
                printf("Client logged in as %s\n", username);
            } else if (strcmp(command, "logout") == 0) {
                printf("Client %s logged out\n", clients[i].username);
                remove_client(clients[i].socket);
            } else if (strcmp(command, "chat") == 0) {
                char receiver_username[USERNAME_LEN];
                char message[BUFFER_SIZE];
                sscanf(buffer, "%*s %s %[^\n]", receiver_username, message);

                if (receiver_username[0] == '@') {
                    send_private_message(clients[i].socket, receiver_username + 1, message);
                } else {
                    broadcast_message(clients[i].socket, message);
                }
            }
        }
    }
}

return 0;

}

void *handle_client(void *arg) {
int client_socket = *((int *)arg);
// Detach thread to prevent memory leak
pthread_detach(pthread_self());

return NULL;

}

void handle_signal(int sig) {
if (sig == SIGINT) {
printf("Received SIGINT, closing all connections and exiting...\n");
for (int i = 0; i < client_count; i++) {
close(clients[i].socket);
}
exit(EXIT_SUCCESS);
}
}

void parse_config_file(const char *filename, int *port) {
FILE *config_file = fopen(filename, "r");
if (!config_file) {
perror("fopen");
exit(EXIT_FAILURE);
}
char keyword[BUFFER_SIZE], value[BUFFER_SIZE];

while (fscanf(config_file, "%[^:]: %s", keyword, value) != EOF) {
    if (strcmp(keyword, "port") == 0) {
        *port = atoi(value);
    }
}

fclose(config_file);
}

void add_client(int client_socket, const char *username) {
pthread_mutex_lock(&clients_mutex);
for (int i = 0; i < MAX_CLIENTS; i++) {
    if (clients[i].socket == 0) {
        clients[i].socket = client_socket;
        strncpy(clients[i].username, username, USERNAME_LEN - 1);
        clients[i].username[USERNAME_LEN - 1] = '\0';
        client_count++;
        break;
    }
}

pthread_mutex_unlock(&clients_mutex);
}

void remove_client(int client_socket) {
pthread_mutex_lock(&clients_mutex);
for (int i =0; i < MAX_CLIENTS; i++) {
if (clients[i].socket == client_socket) {
clients[i].socket = 0;
memset(clients[i].username, 0, USERNAME_LEN);
client_count--;
break;
}
}
pthread_mutex_unlock(&clients_mutex);
}

void broadcast_message(int sender_socket, const char *message) {
pthread_mutex_lock(&clients_mutex);
char sender_username[USERNAME_LEN];
for (int i = 0; i < MAX_CLIENTS; i++) {
    if (clients[i].socket == sender_socket) {
        strncpy(sender_username, clients[i].username, USERNAME_LEN - 1);
        sender_username[USERNAME_LEN - 1] = '\0';
        break;
    }
}

char formatted_message[BUFFER_SIZE];
snprintf(formatted_message, BUFFER_SIZE, "%s >> %s", sender_username, message);

for (int i = 0; i < MAX_CLIENTS; i++) {
    if (clients[i].socket != 0 && clients[i].socket != sender_socket) {
        send(clients[i].socket, formatted_message, strlen(formatted_message), 0);
    }
}

pthread_mutex_unlock(&clients_mutex);
}

void send_private_message(int sender_socket, const char *receiver_username, const char *message) {
pthread_mutex_lock(&clients_mutex);
char sender_username[USERNAME_LEN];
for (int i = 0; i < MAX_CLIENTS; i++) {
    if (clients[i].socket == sender_socket) {
        strncpy(sender_username, clients[i].username, USERNAME_LEN - 1);
        sender_username[USERNAME_LEN - 1] = '\0';
        break;
    }
}

char formatted_message[BUFFER_SIZE];
snprintf(formatted_message, BUFFER_SIZE, "%s >> %s", sender_username, message);

for (int i = 0; i < MAX_CLIENTS; i++) {
    if (clients[i].socket != 0 && strcmp(clients[i].username, receiver_username) == 0) {
        send(clients[i].socket, formatted_message, strlen(formatted_message), 0);
        break;
    }
}

pthread_mutex_unlock(&clients_mutex);
}