#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <unistd.h>
#include "data.h"

void init_conenections(){
    sqlite3 *db;
    sqlite3_stmt *res;
    int rc = sqlite3_open("grid.db", &db);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "[err]: Cannot open DB- %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    printf("[info]: Reading hosts from db...\n");
    char *sql_cmd = "SELECT id, ip, mac, name FROM hosts";
    rc = sqlite3_prepare_v2(db, sql_cmd, -1, &res, 0);  

    if (rc != SQLITE_OK) {
        fprintf(stderr, "[err]: Failed to fetch data - %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
    }
    int exists=0;

    while (sqlite3_step(res) == SQLITE_ROW && hosts_count < MAX_HOSTS) {
        hosts_cache[hosts_count].id = sqlite3_column_int(res, 0);
        strcpy(hosts_cache[hosts_count].ip, (const char *)sqlite3_column_text(res, 1));
        strcpy(hosts_cache[hosts_count].mac, (const char *)sqlite3_column_text(res, 2));
        strcpy(hosts_cache[hosts_count].hostname, (const char *)sqlite3_column_text(res, 3));
        printf("[info]: Found host - IP: %s, MAC: %s, Hostname: %s\n", 
            hosts_cache[hosts_count].ip, 
            hosts_cache[hosts_count].mac, 
            hosts_cache[hosts_count].hostname);

        char setup_cmd[1024];
        snprintf(setup_cmd, sizeof(setup_cmd),
                "sshpass -p '%s' ssh -o StrictHostKeyChecking=no -o ConnectTimeout=5 -f -N -M -S /tmp/ssh_mux_%s %s@%s",
                PASS, hosts_cache[hosts_count].ip, USER, hosts_cache[hosts_count].ip);

        printf("[info]: Trying to connect to [%s]\n", hosts_cache[hosts_count].ip);
        int status = system(setup_cmd);

        if (status == 0){
            printf("[info]: Ok.\n");
            exists=1;
        }
        else{
            printf("[info]: Failed.\n");
        }
        hosts_count++;
    }

    sqlite3_finalize(res);
    sqlite3_close(db);

    if(exists==0){
        printf("[err]: Couldnt establish any connection.\n");
        exit(1);
    }
    else{
        printf("[info]: Connections established.\n\n");
    }
}

void end_connections()
{
    char cmd[1024];
    int rc;


    printf("\n[info]: Closing connections...\n");
    for(int i=0; i<hosts_count; i++){
        snprintf(cmd, sizeof(cmd), 
                 "ssh -S /tmp/ssh_mux_%s -O exit %s@%s 2>/dev/null", 
                 hosts_cache[i].ip, USER, hosts_cache[i].ip);
        
        system(cmd);
    }
    system("rm -f /tmp/ssh_mux_*");
    
    printf("[shutdown]: Cleanup complete, connections closed.\n");
}