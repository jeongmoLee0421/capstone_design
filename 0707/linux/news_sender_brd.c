#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 30
void error_handling(char* message);

int main(int argc, char* argv[]){
    int send_sock;
    struct sockaddr_in broad_addr;
    FILE* fp;
    char buf[BUF_SIZE];
    int so_brd = 1;

    if (argc != 3){
        printf("usage %s <broad cast ip> <port>\n", argv[0]);
        exit(1);
}

    send_sock = socket(PF_INET, SOCK_DGRAM, 0); // broadcast is based on udp
    if (send_sock == -1)
        error_handling("socket() error");

    memset(&broad_addr, 0, sizeof(broad_addr));
    broad_addr.sin_family = AF_INET;
    broad_addr.sin_addr.s_addr = inet_addr(argv[1]);
    broad_addr.sin_port = htons(atoi(argv[2]));

    // set broadcast option
    setsockopt(send_sock, SOL_SOCKET, SO_BROADCAST, (void*)&so_brd, sizeof(so_brd));

    if ((fp = fopen("news.txt", "r")) == NULL)
        error_handling("fopen() error");

    while (!feof(fp)){
        fgets(buf, BUF_SIZE, fp);
        sendto(send_sock, buf, strlen(buf), 0, (struct sockaddr*)&broad_addr, sizeof(broad_addr));
        sleep(2); // time interval
}

    fclose(fp);
    close(send_sock);
    return 0;
}

void error_handling(char* message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
