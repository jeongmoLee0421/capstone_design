#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>

#define BUF_SIZE 1024
void error_handling(char* message);

int main(int argc, char* argv[]){
    int sock, fd;
    char file_name[BUF_SIZE];
    char buf[BUF_SIZE];
    struct sockaddr_in serv_addr;

    if (argc != 3){
        printf("usage %s <ip> <port>\n", argv[0]);
        exit(1);
}

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error");

    fputs("input file name: ", stdout);
    fgets(file_name, BUF_SIZE, stdin);
    file_name[strlen(file_name) - 1] = 0;
    printf("%s %ld", file_name, strlen(file_name));
    write(sock, file_name, strlen(file_name));

    read(sock, buf, BUF_SIZE);
    fd = open("receive_file.txt", O_CREAT|O_WRONLY|O_TRUNC);
    if (fd == -1)
        error_handling("open() error");
    write(fd, buf, strlen(buf));

    close(fd);
    close(sock);
    return 0;
}

void error_handling(char* message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
