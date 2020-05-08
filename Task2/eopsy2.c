#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>

void SIGTERM_handler(){
    printf("%d recived sigterm\n",getpid());
    exit(1);
}

int CHILD_PROC(int var){
    printf("-- child %d created\n",var);
    printf("- %d \n",getppid());

    signal(SIGTERM,SIGTERM_handler);

    sleep(10);
    //ADD - loop wait() until signal that all processes are created
    printf("-- child %d is done executing\n", var);
    //exit(0);
}

int PARENT_PROC(){
}


int main(int argc, char **argv){

    printf("-- program beginning\n");

    int NumOfProcesses = 5;
    int sleeplen = 1;
    pid_t p_pid = getpid(); //pid of current process, the one that will be the parent
    pid_t pids[NumOfProcesses];

    for(int i =0;i<NumOfProcesses;i++){
        if((pids[i] = fork() )<0){ //split happens here...
            printf("Fork Failed!\n");
            //ADD - if failed send SIGTERM to all childs and exit with 1
            for(int j=0;j<i;j++){
                kill(pids[j],SIGTERM);
            }
        }
        else if(pids[i] == 0){
            CHILD_PROC(i);
            break;
        }
        else{
        sleep(sleeplen);
        }
    }

    if(getpid() == p_pid){ //parent process
        printf("-- All child processes created\n");
        int counter = 0;
        int status;
        while((wait(&status)) > 0){
            counter++;
        }
        printf("-- %d exit codes recived\n", counter);
        printf("-- No more child processes to be synchronized\n");
    }

    return 0;
}

