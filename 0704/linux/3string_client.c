#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
void error_handling(char* message);

int main(int argc, char* argv[]){
    int sock;
    struct sockaddr_in serv_addr;
    int idx = 0, read_len = 0;

    char msg1[] = "hello server?";
    char msg2[] = "i'm client";
    char msg3[] = "nice to meet you";
    char* str_ptr[] = {msg1, msg2, msg3};
    int str_len;
    char buf[100];

    if (argc != 3){
        printf("Usage: %s <ip> <port>\n", argv[0]);
        exit(1);
}

    sock = socket(PF_INET, SOCK_STREAM, 0); // ipv4, tcp
    if (sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error");

    for (int i=0; i<sizeof(str_ptr)/sizeof(char*); i++){
        read(sock, (char*)&str_len, 4);
        read(sock, buf, str_len);
        printf("%s\n", buf);
        printf("%d\n", str_len);
        printf("%ld\n", strlen(buf));

        str_len = strlen(str_ptr[i]) + 1;
        write(sock, (char*)&str_len, 4);
        write(sock, str_ptr[i], str_len);
}

    close(sock);
    return 0;
}

void error_handling(char* message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
