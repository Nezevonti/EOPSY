#include <pthread.h> 
#include <semaphore.h> 
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

// Timewarp - change to slow down / speed up the simulation
// 1 - normal, <1 - slow down, >1 speed up

#define TIMEWARP 2

// Number of philosophers, change here
#define N 8 


#define THINKING 2 
#define HUNGRY 1 
#define EATING 0 
#define LEFT (phnum + (N-1)) % N 
#define RIGHT (phnum + 1) % N 
  
int state[N]; 
int phil[N];
int phil_stat[N]; // statistic of eating.
  
sem_t mutex; 
sem_t S[N]; 

// Returns random variable for waiting, representing between
// 2 to 5 seconds, expressed in useconds
// multiplier is used for scailing the wait time
// (to make waits different)
int wait_rand_len(int multiplier){
    int upper, lower;
    upper = 5000000/TIMEWARP;
    lower = 2000000/TIMEWARP;
    int x = rand() % (upper + 1 - lower) + lower;
    srand(x);
    return x*multiplier;
}

// wait random length of time. See wait_rand_len
void wait_rand(int multiplier){
    int x=wait_rand_len(multiplier);
    usleep(x);
}

void interrupt_action(int signum){
    // print the phil_stat to show distribution of eating
    
    printf("\n\n-----Statistic-----\n\n");
    for(int i=0;i<N;i++){
	printf("[ID: %d] - %d\n",i+1, phil_stat[i]);
    }
    printf("\n\n");
    //wait to allow the user to read the stats
    wait_rand(10);
}

// check if given phil (phnum) can currently eat
int canEat(int phnum){
   if(state[phnum] == HUNGRY){ //check if the phil is hungry
      // check if the neighbours are eating. If even one of them is
      // then the cutlery is in use and eating is not possible.
      if(state[LEFT] != EATING && state[RIGHT] != EATING){
	return 1;
      }
   }
   return 0;
}

void test(int phnum) 
{ 
    if (canEat(phnum)) { //the phil can eat
        // set state to eating 
        state[phnum] = EATING;  
  	
	//print the msg and update the statistic
        printf("Philosopher [ID: %d] takes fork %d and %d\n", 
                      phnum + 1, LEFT + 1, phnum + 1); 
  
        printf("Philosopher [ID: %d] is Eating\n", phnum + 1);
	phil_stat[phnum]++;

	//wait_rand(3);
  
        // sem_post(&S[phnum]) has no effect 
        // during takefork 
        // used to wake up hungry philosophers 
        // during putfork 
        sem_post(&S[phnum]); 
    }
    else{// the philosopher cannot eat
	printf("Philosopher [ID: %d] cannot eat\n", phnum + 1);
	
	// explanation why
	if(state[LEFT] == EATING){
	   printf("-- fork %d is taken\n",LEFT+1);
	}

	if(state[RIGHT] == EATING){
	   printf("-- fork %d is taken\n",phnum+1);
	}
    }
}

// take up fork 
void take_fork(int phnum) 
{ 
    //raise the mutex
    sem_wait(&mutex); 
  
    //set philosopher state to hungry 
    state[phnum] = HUNGRY; 
  
    printf("Philosopher [ID: %d] is Hungry\n", phnum + 1); 
  
    // eat if neighbours are not eating 
    test(phnum); 
  
    sem_post(&mutex); 
  
    // if unable to eat wait to be signalled 
    sem_wait(&S[phnum]); 
  
    wait_rand(2); 
} 
  
// put down fork
void put_fork(int phnum) 
{ 
    // wait on the mutex 
    sem_wait(&mutex); 
  
    // set state to thinking 
    state[phnum] = THINKING; 
  
    printf("Philosopher [ID: %d] putting fork %d and %d down\n", 
           phnum + 1, LEFT + 1, phnum + 1); 
    printf("Philosopher [ID: %d] is thinking\n", phnum + 1); 
    // signal the neighbours to wake them up, so if they are waiting 
    // they can check again.
    test(LEFT); 
    test(RIGHT); 
  
    sem_post(&mutex); 
} 
  
void* philospher(void* num) 
{ 
  
    while (1) { 
  
        int* i = num; 
  
        wait_rand(2); 
  
        take_fork(*i); 
  
        wait_rand(2); 
  
        put_fork(*i); 
    } 
} 
  
int main() 
{ 
  
    int i; 
    pthread_t threadID[N];
    struct sigaction action;

    // init the stats table with 0s
    for(int i =0;i<N;i++){
	phil_stat[i]=0;
    }
    
    // handle closing to print the stats    
    action.sa_handler = interrupt_action;
    sigaction(SIGINT,&action,NULL);

    // initialize the semaphores 
    sem_init(&mutex, 0, 1);
  
    for (i = 0; i < N; i++){
        sem_init(&S[i], 0, 0);
    }
  
    for (i = 0; i < N; i++) { 
  	phil[i] = i;
        // create philosopher 
        pthread_create(&threadID[i], NULL, 
                       philospher, &phil[i]); 
  
        printf("Philosopher [ID: %d] is Thinking\n", i + 1); 
    } 
  
    for (i = 0; i < N; i++){
        pthread_join(threadID[i], NULL); 
    }
}  
