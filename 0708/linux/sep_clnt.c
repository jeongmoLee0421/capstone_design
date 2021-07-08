#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
void error_handling(char* message);

int main(int argc, char* argv[]){
    int sock;
    char buf[BUF_SIZE];
    int str_len;
    struct sockaddr_in serv_addr;

    FILE* readfp;
    FILE* writefp;

    if (argc != 3){
        printf("Usage %s <ip> <port>\n", argv[0]);
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
    else
        puts("connected........");

    readfp = fdopen(sock, "r");
    writefp = fdopen(sock, "w");

    while(1){
        if (fgets(buf, sizeof(buf), readfp) == NULL) // If EOF is passed, the fgets function returns null ptr.
            break;
        fputs(buf, stdout);
        fflush(stdout);
}

    fputs("from client: thank you\n", writefp);
    fflush(writefp);

    fclose(writefp), fclose(readfp);
    return 0;
}

void error_handling(char* message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
