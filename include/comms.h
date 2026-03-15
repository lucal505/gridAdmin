#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int send_msg(int sock, const char *data) {
    int data_len = strlen(data);
    int net_len = htonl(data_len); 

    int bytes = send(sock, &net_len, sizeof(net_len), 0);
    if (bytes != sizeof(net_len)) 
        return -1;

    // folsoesc while sa fiu sigur ca trimit tot si daca e reteaua aglomerata
    int total_bytes = 0;
    while(total_bytes < data_len) {
        bytes = send(sock, data + total_bytes, data_len - total_bytes, 0);
        if (bytes < 0) return -1;
        total_bytes += bytes;
    }
    
    return 0; 
}

int recv_msg(int sock, char *buffer, int max_size) {
    int net_len;

    char *len_ptr = (char*)&net_len;
    int total_bytes = 0;
    
    while (total_bytes < sizeof(net_len)) {
        int bytes = recv(sock, len_ptr + total_bytes, sizeof(net_len) - total_bytes, 0);
        if (bytes <= 0) return -1; 
        total_bytes += bytes;
    }

    int len = ntohl(net_len); 

    if (len >= max_size) {
        printf("[err]: Packet too large (%d bytes). Max allowed [%d]\n", len, max_size);
        return -1; 
    }

    total_bytes = 0;
    while (total_bytes < len) {
        int bytes = recv(sock, buffer + total_bytes, len - total_bytes, 0);
        if (bytes <= 0) return -1;
        total_bytes += bytes;
    }

    buffer[len] = 0; 
    return len; 
}