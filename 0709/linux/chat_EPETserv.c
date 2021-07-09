#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>

#define BUF_SIZE 4
#define EPOLL_SIZE 50
#define CLNT_SIZE 100
void setnonblockingmode(int fd);
void error_handling(char* buf);

int main(int argc, char* argv[]){
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;
    int str_len, i;
    char buf[BUF_SIZE];
    int clnt_arr[CLNT_SIZE] = {0, }, clnt_num = 0;

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

    setnonblockingmode(serv_sock);
    event.events = EPOLLIN;
    event.data.fd = serv_sock;
    epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &event);

    while(1){
        event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);
        if (event_cnt == -1){
            puts("epoll_wait() error");
            break;
}

    puts("return epoll_wait");
    for (i=0; i<event_cnt; i++){
        if (ep_events[i].data.fd == serv_sock){
            clnt_addr_size = sizeof(clnt_addr);
            clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
            if (clnt_sock == -1)
                error_handling("accpet() error");
            setnonblockingmode(clnt_sock);
            clnt_arr[clnt_num++] = clnt_sock; // add client to array and client num + 1

            event.events = EPOLLIN|EPOLLET;
            event.data.fd = clnt_sock;
            epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event);
            printf("connected client: %d\n", clnt_sock);
}
        else{
            while(1){
                str_len = read(ep_events[i].data.fd, buf, BUF_SIZE);
                if (str_len == 0){ // close request
                    epoll_ctl(epfd, EPOLL_CTL_DEL, ep_events[i].data.fd, NULL);
                    close(ep_events[i].data.fd);
                    printf("closed client: %d\n", ep_events[i].data.fd);

                    for (int j=0; j<clnt_num; j++){ // Find the location of the client to delete
                        if (ep_events[i].data.fd == clnt_arr[j]){
                            int idx = j;
                            for (int k=idx; k<clnt_num; k++){ // pulls arr element
                                clnt_arr[k] = clnt_arr[k+1];
}
                        clnt_num--; // reduce clnt num
                        break;
}
}
                    break;
}
                else if (str_len < 0){
                    if (errno == EAGAIN)
                        break;
}
                else{
                    for (int j=0; j<clnt_num; j++){
                        if (clnt_arr[j] != ep_events[i].data.fd){ // except the sender
                            write(clnt_arr[j], buf, str_len); // send msg all client
}
}
}
}
}
}
}

    close(serv_sock);
    close(epfd);
    return 0;
}

void setnonblockingmode(int fd){
    int flag = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flag|O_NONBLOCK);
}

void error_handling(char* buf){
    fputs(buf, stderr);
    fputc('\n', stderr);
    exit(1);
}
