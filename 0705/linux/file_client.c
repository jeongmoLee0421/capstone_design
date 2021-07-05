#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 30
void error_handling(char* message);

int main(int argc, char* argv[]){
    int sock;
    FILE* fp;

    char buf[BUF_SIZE];
    int read_cnt;
    struct sockaddr_in serv_addr;
    
    if (argc != 3){
        printf("usage %s <ip> <port>\n", argv[0]);
        exit(1);
}

    if ((fp = fopen("receive.dat", "wb")) == NULL)
        error_handling("fopen() error");

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error");

    while ((read_cnt = read(sock, buf, BUF_SIZE)) != 0)
        fwrite((void*)buf, 1, read_cnt, fp);

    puts("received file data");
    write(sock, "thank you", 10);
    fclose(fp);
    close(sock);
    return 0;
}

void error_handling(char* message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
