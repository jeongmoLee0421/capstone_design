#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void keycontrol(int sig){
    char c;

    if (sig == SIGINT){
        puts("close the program? (Y or N)");
        scanf("%c", &c);

        if (c == 'Y' || c == 'y'){
            exit(1);
}
        else{
            return;
}
}
}

int main(int argc, char* argv[]){
    struct sigaction act;

    act.sa_handler = keycontrol;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    sigaction(SIGINT, &act, 0);

    while(1){
        puts("string...");
        sleep(1);
}

    return 0;
}
