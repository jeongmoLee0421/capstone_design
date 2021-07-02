#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
void error_handling(char* message);

int main(int argc, char* argv[]){
    int serv_sock;
    int clnt_sock;

    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size;

    int str_len = 0;
    int idx = 0;
    int write_len = 0;

    char message[] = "Hello World!";
    if (argc != 2){
        printf("Usage %s <port>\n", argv[0]);
        exit(1);
}

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
    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
    if (clnt_sock == -1)
        error_handling("accept() error");

    write(clnt_sock, message, 5);
    write(clnt_sock, message+5, 5);
    write(clnt_sock, message+10, strlen(message)-10);

    //write error
    /*while(write_len = write(clnt_sock, message[idx++], 1)){
        if(write_len == -1)
            error_handling("write() error");

        str_len += write_len;
}*/

    /*for(int i=0; i<100; i++){
        write_len = write(clnt_sock, &message[idx++], 1);
        if (write_len == -1)
            error_handling("write() error");
	str_len += write_len;
}*/
    //printf("write call count: %d\n", str_len);
    close(clnt_sock);
    close(serv_sock);
    return 0;
}

void error_handling(char* message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
