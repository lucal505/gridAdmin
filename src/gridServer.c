#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sqlite3.h>
#include "handlers.h"
#include "data.h"

host_data hosts_cache[MAX_HOSTS]; 
int hosts_count = 0;

int main() {
    signal(SIGINT, handle_sigint);

    int server_socket, *new_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(server_addr);

    pthread_mutex_init(&socket_lock, NULL);
    pthread_mutex_init(&grid_lock, NULL);

    init_conenections(); 

    // initializare socket server
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // optiune pentru reutilizarea adresei la restart
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_socket, 5);

    printf("[server]: Waiting for clients on port %d...\n", PORT);

    while (1) {
        int client_fd = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }

        printf("[server]: New connection from %s:%d\n", 
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        new_socket = malloc(sizeof(int));
        *new_socket = client_fd;

        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, client_handler, (void*)new_socket) < 0) {
            perror("Could not create thread");
            free(new_socket);
            close(client_fd);
        }

        pthread_detach(client_thread);
    }

    end_connections();
    pthread_mutex_destroy(&socket_lock);
    pthread_mutex_destroy(&grid_lock);
    return 0;
}