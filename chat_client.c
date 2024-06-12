//client

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

void *receive_messages(void *arg);
void handle_signal(int sig);
void parse_config_file(const char *filename, char *servhost, int *servport);

int client_socket;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s configuration_file\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char servhost[BUFFER_SIZE];
    int servport;
    parse_config_file(argv[1], servhost, &servport);
    printf("%s:%d",servhost,servport);
    signal(SIGINT, handle_signal);

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(servhost);
    server_addr.sin_port = htons(25190);

    if (connect(client_socket, (struct sockaddr *)&server_addr , sizeof(server_addr)) == -1) {
       printf("Connected to server %s:%d\n", servhost, servport); 
perror("connect");
exit(EXIT_FAILURE);
}
printf("Connected to server %s:%d\n", servhost, servport);

pthread_t recv_thread;
if (pthread_create(&recv_thread, NULL, receive_messages, NULL) != 0) {
    perror("pthread_create");
    close(client_socket);
    exit(EXIT_FAILURE);
}

char command[BUFFER_SIZE];
while (1) {
    memset(command, 0, BUFFER_SIZE);
    fgets(command, BUFFER_SIZE, stdin);
    command[strlen(command) - 1] = '\0';  // Remove newline

    if (strcmp(command, "exit") == 0) {
        break;
    } else {
        send(client_socket, command, strlen(command), 0);
    }
}

close(client_socket);
printf("Disconnected from server\n");

return 0;
}

void *receive_messages(void *arg) {
char buffer[BUFFER_SIZE];
while (1) {
    memset(buffer, 0, BUFFER_SIZE);
    int recv_len = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);

    if (recv_len <= 0) {
        perror("recv");
        break;
    }

    buffer[recv_len] = '\0';
    printf("%s\n", buffer);
}

return NULL;
}

void handle_signal(int sig) {
if (sig == SIGINT) {
printf("Received SIGINT, closing connection and exiting...\n");
close(client_socket);
exit(EXIT_SUCCESS);
}
}

void parse_config_file(const char *filename, char *servhost, int *servport) {
FILE *config_file = fopen(filename, "r");
if (!config_file) {
perror("fopen");
exit(EXIT_FAILURE);
}
char keyword[BUFFER_SIZE], value[BUFFER_SIZE];

while (fscanf(config_file, "%[^:]: %s", keyword, value) != EOF) {
    if (strcmp(keyword, "servhost") == 0) {
        strncpy(servhost, value, BUFFER_SIZE - 1);
        servhost[BUFFER_SIZE - 1] = '\0';
    } else if (strcmp(keyword, "servport") == 0) {
        *servport = atoi(value);
    }
}

fclose(config_file);
}