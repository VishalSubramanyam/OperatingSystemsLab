#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(){
    while(1){
        // spawn 5 children
        for(int i = 0;i < 5;i++){
            int child = fork();
            if(child == 0){
                // spawn 10 children
                int grchild;
                for(int j = 0;j < 10;j++){
                    grchild = fork();
                    if(grchild == 0){
                        while (1)
                        {
                            int i;
                            i = 0;
                            i++;
                            i--;
                        }
                    }
                }
                int status;
                waitpid(grchild, &status, 0);
            }
        }
        // sleep for 2 mins
        sleep(120);
    }
}
