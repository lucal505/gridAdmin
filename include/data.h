#ifndef DATA_H
#define DATA_H

#define USER "test"
#define PASS "testpass"
#define MAX_HOSTS 255
#define PORT 8080
#define RESP_SIZE 9000
#define TEMP_SIZE 8192

typedef struct{
    char ip[50];
    char mac[50];
    char hostname[64];
    char command[256];
    int sd;
} thread_data;

typedef struct {
    int id;
    char ip[20];
    char mac[50];
    char hostname[64];
} host_data;

extern host_data hosts_cache[MAX_HOSTS];
extern int hosts_count;

#endif