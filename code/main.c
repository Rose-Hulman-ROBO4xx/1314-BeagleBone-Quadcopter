#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
void main(){
    int f;
    f = open("asdf", O_NONBLOCK | O_RDONLY);
    if (f < 0){
        printf("error\n");
        exit(-1);
    }

    int x;
    char buffer[256];
    while(1){
        int num_read = read(f, buffer, 256);
        buffer[num_read] = '\0';
        if (num_read>=1){
            char * pch;
            pch = strtok(buffer, "\n");
            while(pch != NULL){
                printf("%s\n", pch);
                fflush(stdout);
                pch = strtok(NULL, "\n");
            }
        }
        usleep(0);

    }

}
