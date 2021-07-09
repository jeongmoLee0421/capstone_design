#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#define BUF_SIZE 100
#define EPOLL_SIZE 50
void error_handling(char* buf);

int main(int argc, char* argv[]){
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;
    int str_len, i;
    char buf[BUF_SIZE];

    struct epoll_event* ep_events; // Used to group file descriptors that have changed state.
    struct epoll_event event; // It is also used to register event types when registering file descriptors with the epoll instance.
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

    // create epoll file descriptor repository
    epfd = epoll_create(EPOLL_SIZE);
    if (epfd == -1)
        error_handling("epoll_create() error");

    ep_events = malloc(sizeof(struct epoll_event)* EPOLL_SIZE);
    if (ep_events == NULL)
        error_handling("malloc() error");

    event.events = EPOLLIN; // Situation in which data to be received exists
    event.data.fd = serv_sock;
    epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &event); // Register serv_sock for event observation in epfd.

    while(1){
        event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1); // The file descriptor where the event occurred is populated into ep_events.
        if (event_cnt == -1){
            puts("epoll_wait() error");
            break;
}

    for (i=0; i<event_cnt; i++){ // check only changes
        if (ep_events[i].data.fd == serv_sock){ // connect request, do not confuse = with ==.
            clnt_addr_size = sizeof(clnt_addr);
            clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
            if (clnt_sock == -1)
                error_handling("accpet() error");

            // register clnt_sock for event observation in epfd
            event.events = EPOLLIN;
            event.data.fd = clnt_sock;
            epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event);
            printf("connected client: %d\n", clnt_sock);
}
        else{
            str_len = read(ep_events[i].data.fd, buf, BUF_SIZE);
            if (str_len == 0){ // close request
                epoll_ctl(epfd, EPOLL_CTL_DEL, ep_events[i].data.fd, NULL); // delete clnt_sock in epfd
                close(ep_events[i].data.fd);
                printf("closed client: %d\n", ep_events[i].data.fd);
}
            else{
                write(ep_events[i].data.fd, buf, str_len); // echo
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
