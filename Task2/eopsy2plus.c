#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

#define WITH_SIGNALS



void SIGTERM_handler(){
    printf("CHILD[%d]: recived sigterm\n",getpid());
    exit(1);
}

#ifdef WITH_SIGNALS
int flag = 0;

void IGNORE_handler(){
   //Do nothin', see nothin'
}

void PARENT_SIGINT_handler(){
   printf("PARENT[%d]: SIGINT recived\n",getpid());
   flag=1;
}

void CHILD_KEYINTERRUPT_handler(){
	printf("CHILD[%d]: Keybord interrupt ignored\n",getpid());
}

int CHILD_PROC(int var){
    printf("CHILD[%d]: child %d created\n", getpid(), var);
    printf("- %d \n",getppid());

    signal(SIGTERM,SIGTERM_handler);

    #ifdef WITH_SIGNALS
    signal(SIGINT,CHILD_KEYINTERRUPT_handler);
    #endif

    sleep(10);
    //ADD - loop wait() until signal that all processes are created
    printf("CHILD[%d] child %d is done executing\n", getpid(),var);
    exit(0);
}

#endif

int PARENT_PROC(){
}


int main(int argc, char **argv){

    printf("-- program beginning\n");

    int NumOfProcesses = 5;
    int sleeplen = 1;
    pid_t p_pid = getpid(); //pid of current process, the one that will be the parent
    pid_t pids[NumOfProcesses];

    #ifdef WITH_SIGNALS
    for(int i=1;i<=31;i++){
	if(i!= SIGINT){
	   signal(i,IGNORE_handler());
	}
	if(p_pid!=-1){
	   signal(SIGINT,PARENT_SIGNINT_handler());
	}
    }
    #endif
    int tmp=0;
    for(int i =0;i<NumOfProcesses;i++){
        if((pids[i] = fork() )<0){ //split happens here...
            printf("Fork Failed!\n");
            //ADD - if failed send SIGTERM to all childs and exit with 1
            for(int j=0;j<i;j++){
                kill(pids[j],SIGTERM);
            }
        }
        else if(pids[i] == 0){
            setgid(pids[i],pids[0]); //group all childs into one group
	    tmp=getgid(pids[i])
            CHILD_PROC(i);
            break;
        }
        else{
	#ifdef WITH_SIGNALS
	   if(flag==1){
		printf("PARENT [%d]: Creation interrupted!\n",getpid());
		killpg(tmp,SIGTERM);
		break;
           }
	#endif
        sleep(sleeplen);
        }
    }

    if(getpid() == p_pid){ //parent process
        printf("-- All child processes created\n");
        int counter = 0;
        while((wait()) != -1){
            counter++;
        }
        printf("-- %d exit codes recived\n", counter);
        printf("-- No more child processes to be synchronized\n");
    }


   #ifdef WITH_SIGNALS
   for(int i=1;i<=31;i++){
	signal(i,SIG_DFL)
    }
   #endif


    return 0;
}
