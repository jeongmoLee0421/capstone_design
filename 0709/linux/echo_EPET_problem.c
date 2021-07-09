#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#define BUF_SIZE 4 // reduce buffer size
#define EPOLL_SIZE 50
void error_handling(char* buf);

int main(int argc, char* argv[]){
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;
    int str_len, i;
    char buf[BUF_SIZE];

    struct epoll_event* ep_events;
    struct epoll_event event;
    int epfd, event_cnt;

    if (argc != 2){
        printf("usage %s <port>\n", argv[0]);
        exit(1);
}

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0 ,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");

    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    epfd = epoll_create(EPOLL_SIZE);
    if (epfd == -1)
        error_handling("epoll_create() error");

    ep_events = malloc(sizeof(struct epoll_event)* EPOLL_SIZE);
    if (ep_events == NULL)
        error_handling("malloc() error");

    event.events = EPOLLIN;
    event.data.fd = serv_sock;
    epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &event);

    while(1){
        event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);
        if (event_cnt == -1){
            puts("epoll_wait() error");
            break;
}

    puts("return epoll_wait"); // check epoll_wait func call
    for (i=0; i<event_cnt; i++){
        if (ep_events[i].data.fd == serv_sock){
            clnt_addr_size = sizeof(clnt_addr);
            clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
            if (clnt_sock == -1)
                error_handling("accpet() error");

            event.events = EPOLLIN|EPOLLET; // Set up with edge-triggered mode
            event.data.fd = clnt_sock;
            epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event);
            printf("connected client: %d\n", clnt_sock);
}
        else{
            str_len = read(ep_events[i].data.fd, buf, BUF_SIZE); // It is read only on first read, and the rest of the data is not read until the event is registered.
            if (str_len == 0){
                epoll_ctl(epfd, EPOLL_CTL_DEL, ep_events[i].data.fd, NULL);
                close(ep_events[i].data.fd);
                printf("closed client: %d\n", ep_events[i].data.fd);
}
            else{
                write(ep_events[i].data.fd, buf, str_len);
}
}
}
}

    close(serv_sock);
    close(epfd);
    return 0;
}

void error_handling(char* buf){
    fputs(buf, stderr);
    fputc('\n', stderr);
    exit(1);
}
