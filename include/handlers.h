#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <pthread.h>
#include <sqlite3.h>
#include "comms.h"
#include "manage_connections.h"
#include "misc.h"

pthread_mutex_t socket_lock;
pthread_mutex_t grid_lock;

void *connection_handler(void *td){
    thread_data *data = (thread_data *)td;
    FILE *fd;
    char cmd[1024];
    char line[1024];
    char *response = (char *)malloc(RESP_SIZE);
    char *temp = (char *)malloc(TEMP_SIZE);
    response[0] = 0;

    snprintf(cmd, sizeof(cmd),
             "ssh -o BatchMode=yes -o ConnectTimeout=5 -S /tmp/ssh_mux_%s %s@%s \"%s\" 2>&1",
             data->ip, USER, data->ip, data->command);

    printf("[info-thread:%s]: Executing command [%s]\n", data->ip, cmd);
    fd = popen(cmd, "r");

    if (!fd){ 
        snprintf(response, RESP_SIZE, "[info-thread:%s]: Error on popen()!\n", data->ip);
    }
    else{
        //extracting output
        temp[0] = 0;
        while (fgets(line, sizeof(line), fd) != NULL){
            strcat(temp, line);
        }

        int status = pclose(fd);
        int exit_code = WEXITSTATUS(status);

        if (strncmp(data->command, "uptime", 6) == 0) {
            if (exit_code == 0 && strlen(temp) > 0 && (strstr(temp, "up"))) {
                temp[strcspn(temp, "\r\n")] = 0;
                snprintf(response, RESP_SIZE, "[info]: Status [%s|%s] -> ONLINE |%s\n", data->hostname, data->ip, temp);
            } 
            else {
                snprintf(response, RESP_SIZE, "[info]: Status [%s|%s] -> OFFLINE\n", data->hostname, data->ip);
            }
        }
        else{
            if (exit_code == 0){
                snprintf(response, RESP_SIZE, "[info]: Host [%s|%s]\n[output]:\n%s", data->hostname, data->ip, temp);
            }
            else{
                snprintf(response, RESP_SIZE, "[err]: Host [%s|%s] (Error code: %d)\n", data->hostname, data->ip, exit_code);
            }
        }
    }

    pthread_mutex_lock(&socket_lock);
    send_msg(data->sd, response);
    pthread_mutex_unlock(&socket_lock);

    free(response);
    free(temp);
    free(data);
    return NULL;
}

void *client_handler(void *arg) {
    int client_sock = *(int*)arg;
    free(arg);

    char msg[256];
    int bytes_read;

    while(recv_msg(client_sock, msg, sizeof(msg)) > 0) {
        printf("[info-client-%d]: Received command [%s]\n", client_sock, msg);

        if (strcmp(msg, "hosts_list") == 0) {
            char buffer[4096] = ""; 

            strcat(buffer, "\n[Stations list]\n");
            for (int i = 0; i < hosts_count; i++) {
                int id = hosts_cache[i].id;
                const char *ip = hosts_cache[i].ip;
                const char *name = hosts_cache[i].hostname;
                
                if (name == NULL) name = "unnamed";

                char line[128];
                snprintf(line, sizeof(line), "[%d] NAME: %s | IP: %-15s\n", hosts_cache[i].id, 
                                hosts_cache[i].hostname, hosts_cache[i].ip);
                strcat(buffer, line);
            }
            strcat(buffer, "<end>"); // marker

            send_msg(client_sock, buffer);
            continue; 
        }

        char *cmd=strtok(msg, "|");
        char *targets=strtok(NULL, "|");

        if (!cmd || !targets) {
            printf("[err]: Invalid cmd format %d.\n", client_sock);
            continue; 
        }

        printf("[info]: Executing [%s] on targets [%s]\n", cmd, targets);

        // incerc sa blochez accesul altor clienti la comunicarea cu grid-ul
        printf("[info]: Client %d is waiting for grid lock...\n", client_sock);
        pthread_mutex_lock(&grid_lock); 
        printf("[info]: Client %d acquired grid lock. Executing...\n", client_sock);
        
        pthread_t threads[MAX_HOSTS];
        int thread_count = 0;

        for (int i = 0; i < hosts_count; i++) {
            int id = hosts_cache[i].id;
            const char *ip = hosts_cache[i].ip;
            const char *mac = hosts_cache[i].mac;
            const char *hostname = hosts_cache[i].hostname;
            
            if (validate_id(id, targets)) {
                if (strcmp(cmd, "WOL") == 0) {
                    wol(mac);
                    
                    char info[256];
                    snprintf(info, sizeof(info), "[server]: Magic Packet sent to [%s|%s]\n", hostname, mac);
                    send_msg(client_sock, info);
                } 
                else {
                    thread_data *td = (thread_data *)malloc(sizeof(thread_data));
                    strcpy(td->ip, ip);
                    strcpy(td->mac, mac);
                    strcpy(td->hostname, hostname);
                    strcpy(td->command, cmd);
                    td->sd = client_sock;

                    if (pthread_create(&threads[thread_count], NULL, connection_handler, (void *)td) == 0){
                        thread_count++;
                    }
                }
            }
        }

        for(int i=0; i<thread_count; i++) pthread_join(threads[i], NULL);
        
        pthread_mutex_unlock(&grid_lock);

        send_msg(client_sock, "<end>");
    }
    close(client_sock);
    return NULL;
}

void handle_sigint(int sig) {
    end_connections();
    exit(0);
}