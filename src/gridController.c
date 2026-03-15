#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "comms.h"

#define PORT 8080
#define SERVER_IP "127.0.0.1"   // rulez serverul local si ma folosesc de 
                                // serverele facultatii pentru a executa comenzile prin ssh

void get_user_selection(int sock, char *selection) {
    char buffer[4096];
    
    send_msg(sock, "hosts_list");
    
    recv_msg(sock, buffer, sizeof(buffer));
    
    // soatem markerul de final daca exista
    char *p = strstr(buffer, "<end>");
    if(p) *p = 0;

    printf("%s", buffer);
    printf("[info]: Enter targets (ex: 1,3) or 'all': ");
    
    fgets(selection, 100, stdin);
    selection[strcspn(selection, "\n")] = 0;
    
    // adaugare optiunea 'all' by-default
    if(strlen(selection) == 0) {
        strcpy(selection, "ALL");
    }
}

int main(){
    int sd;
    struct sockaddr_in server_addr;
    char response[4096]="";
    
    if((sd=socket(AF_INET,SOCK_STREAM,0))<0){
        perror("[err]: Socket create");
        return 1;
    }

    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(PORT);
    server_addr.sin_addr.s_addr=inet_addr(SERVER_IP);

    if(connect(sd,(struct sockaddr *)&server_addr,sizeof(server_addr))<0){
        perror("[err]: Connect failed");
        close(sd);
        return 1;
    }

    printf("[info]: Connected to server.\n");

    while (1){
        printf("[MAIN MENU]\n1. Check hosts state.\n2. Wake on LAN.\n3. Shutdown.\n4. Custom bash command.\n0. Exit.\n");

        printf("[info]: Please enter your choice: ");
        char choice[10];
        while (1){
            fgets(choice, sizeof(choice), stdin);
            if (choice[0] >= '0' && choice[0] <= '4' && strlen(choice) == 2)
                break;
            
            printf("[warn]: Invalid choice. Choice must be between 0 and 4: ");
        }

        printf("[info]: You selected option %s", choice);

        char msg[256]="";
        char selection[100]="";

        switch(choice[0]) {
            case '1': 
                get_user_selection(sd, selection);
                snprintf(msg, sizeof(msg), "uptime|%s", selection);
                break;

            case '2':
                get_user_selection(sd, selection);
                snprintf(msg, sizeof(msg), "WOL|%s", selection);
                break;

            case '3':
                get_user_selection(sd, selection);
                snprintf(msg, sizeof(msg), "shutdown -h now|%s", selection);
                break;

            case '4':
                printf("[info]: Enter your command: ");
                char cmd[100];
                fgets(cmd, 100, stdin);
                cmd[strcspn(cmd, "\n")] = 0;
                
                get_user_selection(sd, selection);
                snprintf(msg, sizeof(msg), "%s|%s", cmd, selection);
                break;
                
            case '0':
                printf("[info]: Exiting...\n");
                return 0;
        }

        send_msg(sd, msg);
        printf("[info]: Command sent...\n\n");

        while(1){
            int bytes_read = recv_msg(sd, response, sizeof(response));
            if (bytes_read <= 0){
                printf("[err]: Connection lost.\n");
                close(sd);
                return 1;
            }

            response[bytes_read] = 0;

            //verific daca e finalul raspunsului
            if (strstr(response, "<end>") != NULL){
                *strstr(response, "<end>") = 0;
                printf("%s", response);
                break;  
            }
            
            printf("%s", response);
            memset(response, 0, sizeof(response));
        }

        printf("[info]: Returning to menu...\n\n");
        sleep(2);
    }

    return 0;
}