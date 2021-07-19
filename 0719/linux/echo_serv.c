#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <pthread.h>

#define BUF_SIZE 1024
void error_handling(char* message);
void* echo_clnt(void* arg);
char message[BUF_SIZE]; // shared value
pthread_mutex_t mutex;

int main(int argc, char* argv[]){
    int serv_sock, clnt_sock;
    int str_len, i;

    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;

    pthread_t t_id;

    if (argc != 2){
        printf("Usage %s <port>\n", argv[0]);
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

    clnt_addr_size = sizeof(clnt_addr);

    for (i=0; i<500000; i++){
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
        if (clnt_sock == -1)
            error_handling("socket() error");
        else
            printf("connected client %d\n", i+1);

        pthread_create(&t_id, NULL, echo_clnt, (void*)&clnt_sock);
        pthread_detach(t_id);
        printf("connected client ip: %s\n", inet_ntoa(clnt_addr.sin_addr));
}

    close(serv_sock);
    return 0;
}

void* echo_clnt(void* arg){
    int clnt_sock = *((int*)arg);
    int str_len = 0, i;
    char msg[BUF_SIZE];

    pthread_mutex_lock(&mutex); // lock
    while((str_len = read(clnt_sock, message, BUF_SIZE)) != 0)
        write(clnt_sock, message, str_len);
    pthread_mutex_unlock(&mutex); // unlock

    close(clnt_sock);
}

void error_handling(char* message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
