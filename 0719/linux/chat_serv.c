#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <pthread.h>

#define BUF_SIZE 100
#define MAX_CLNT 256

void* handle_clnt(void* arg);
void send_msg(char* msg, int len);
void error_handling(char* msg);

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutex;

int main(int argc, char* argv[]){
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    int clnt_addr_size;
    pthread_t t_id;

    if (argc != 2){
        printf("usage %s <port>\n", argv[0]);
        exit(1);
}

    pthread_mutex_init(&mutex, NULL); // create a mutex

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");

    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    while(1){
        clnt_addr_size = sizeof(clnt_addr);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
        if (clnt_sock == -1)
            error_handling("accpet() error");

        pthread_mutex_lock(&mutex); // lock;
        clnt_socks[clnt_cnt++] = clnt_sock;
        pthread_mutex_unlock(&mutex); // unlock;

        pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock); // create a thread
        pthread_detach(t_id); // destroy a thread
        printf("connect client ip: %s\n", inet_ntoa(clnt_addr.sin_addr));
}

    close(serv_sock);
    return 0;
}

void* handle_clnt(void* arg){
    int clnt_sock = *((int*)arg);
    int str_len = 0, i;
    char msg[BUF_SIZE];

    while((str_len = read(clnt_sock, msg, sizeof(msg))) != 0) // read
        send_msg(msg, str_len); // send

    pthread_mutex_lock(&mutex); // lock
    for (i=0; i<clnt_cnt; i++){ // remove disconnected client
        if (clnt_sock == clnt_socks[i]){
            /*while(i++ < clnt_cnt-1)
                clnt_socks[i] = clnt_socks[i+1];
            break;*/
            for (int j=i; j<clnt_cnt-1; j++)
                clnt_socks[i] = clnt_socks[i+1];
            break;
}
}
    clnt_cnt--;
    pthread_mutex_unlock(&mutex); // unlock;

    close(clnt_sock);
    return NULL;
}

void send_msg(char* msg, int len){
    int i;

    pthread_mutex_lock(&mutex); // lock
    for (i=0; i<clnt_cnt; i++)
        write(clnt_socks[i], msg, len);
    pthread_mutex_unlock(&mutex); // unlock;
}

void error_handling(char* msg){
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
