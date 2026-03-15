#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <sys/socket.h>  
#include <arpa/inet.h>  
#include <unistd.h>

int validate_id(int id, char *list){
    if(strcasecmp(list, "all")==0){
        return 1;
    }

    char *token;
    char list_copy[256];
    strcpy(list_copy, list);
    token = strtok(list_copy, ",");
    while (token != NULL) {
        if (atoi(token) == id){
            return 1;
        }
        token = strtok(NULL, ",");
    }
    return 0;
}

void wol(const char *mac_addr) {
    unsigned char mac[6];
    
    // convertesc mac-ul din string in hexa
    if (sscanf(mac_addr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
               &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) != 6) {
        printf("[err]: Invalid MAC address format: %s\n", mac_addr);
        return;
    }

    unsigned char magic_pkt[102];

    // primii 6 bytes sunt FF
    for (int i = 0; i < 6; i++) {
        magic_pkt[i] = 0xFF;
    }

    // urmatorii 96 bytes sunt MAC-ul repetat de 16 ori
    for (int i = 1; i <= 16; i++) {
        memcpy(&magic_pkt[i * 6], mac, 6);
    }

    // fac un socket udp
    int udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sock < 0) {
        perror("[err]: UDP socket creation failed");
        return;
    }

    // broadcast e obligatoriu pentru WoL
    int broadcast = 1;
    if (setsockopt(udp_sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
        perror("[err]: Broadcast option could not be set");
        close(udp_sock);
        return;
    }

    // by-default broadcast ip = 255.255.255.255 si port 9/7
    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(9); 
    dest_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

    if (sendto(udp_sock, magic_pkt, sizeof(magic_pkt), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("[err]: Failed to send Magic Packet");
    } else {
        printf("[info]: Magic Packet sent to [%s]\n", mac_addr);
    }

    close(udp_sock);
}