#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

void error_handling(char* message);

int main(int argc, char* argv[]){
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;
    int operand_cnt;
    int operand_arr[255], operand;
    char operator;
    int res;

    if (argc != 2){
        printf("usage: %s <port>\n", argv[0]);
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

    clnt_addr_size = sizeof(clnt_addr);
    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
    if (clnt_sock == -1)
        error_handling("accept() error");

    read(clnt_sock, &operand_cnt, 4);

    for (int i=0; i<operand_cnt; i++){
        read(clnt_sock, &operand, 4);
        operand_arr[i] = operand;
}

    read(clnt_sock, &operator, 1);

    switch(operator){
        case '+':
            res = 0;
            for (int i=0; i<operand_cnt; i++)
                res += operand_arr[i];
            break;
        case '-':
            res = operand_arr[0];
            for (int i=1; i<operand_cnt; i++)
                res -= operand_arr[i];
            break;
        case '*':
            res = 1;
            for (int i=0; i<operand_cnt; i++)
                res *= operand_arr[i];
}

    write(clnt_sock, &res, 4);
    close(clnt_sock);
    close(serv_sock);
    return 0;
}

void error_handling(char* message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
