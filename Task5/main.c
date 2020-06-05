#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>

#define KEY 0x1111


//Waiting room size
#define M 2
//Female barbers
#define N1 0
//Male barbers
#define N2 1
//Unisex barbers
#define N3 1

//Multiplier on how long barbers sleep
#define BarberSleepLen 3
//Multiplier on how long to wait between new customers
#define CustomerSpawnCooldown 1
//Multiplier on how long Female Barbers take to cut
#define CutMultiplierFemale 3
//Multiplier on how long Male Barbers take to cut
#define CutMultiplierMale 3
//Multiplier on how long Unisex Barbers take to cut
#define CutMultiplierUnisex 3

typedef struct Customer{
    unsigned int CustomerID;
    char CustomerSEX;
};

void wait_rand(int multiplier);
int wait_rand_len(int multiplier);
void* BarberF(int tmp);
void* BarberM(int tmp);
void* BarberU(int tmp); //void* tmp
int getNextCustomer(char Ctype);
int getEligibleSleepingBarber(char Ctype);
int getEmptySeatIndex();
void CustomerArrives(struct Customer NewCustomer);
void SetUp();
void printQ();


sem_t FBarberSleeps[N1];
sem_t MBarberSleeps[N2];
sem_t UBarberSleeps[N3];

pid_t FBarberPID[N1];
pid_t MBarberPID[N2];
pid_t UBarberPID[N3];

int CustomerCount = 0;
//int N1;
//int N2;
//int N3;
struct Customer WaitingRoom[M+1];

int main(int argc, char** argv)
{
    //Program is runnable with parameters from the cmd
    //Get parameters
    //N1 = argv[1]; //Female customers
    //N2 = argv[2]; //Male customers
    //N3 = argv[3]; //Both
    //M = argv[4];  //Chairs in the waiting room
    srand(time(NULL)); //init the srand
    int rand_val;
    int status;
    struct Customer newCustomer;
    pthread_t BarbersF[N1], BarbersM[N2], BarbersU[N3];


    //init semaphores
    for(int i=0;i<N1;i++)
    {
        sem_init(&FBarberSleeps[i],0,1);
    }
    for(int i=0;i<N2;i++)
    {
        sem_init(&MBarberSleeps[i],0,1);
    }
    for(int i=0;i<N3;i++)
    {
        sem_init(&UBarberSleeps[i],0,1);
    }

    //SetUp
    SetUp();
    //Start N1 Female Barbers
    for(int i=0;i<N1;i++)
    {
        status=pthread_create(&BarbersF[i],NULL,(void *)BarberF,i);
    }
    //Start N2 Male Barbers
    for(int i=0;i<N2;i++)
    {
        status=pthread_create(&BarbersM[i],NULL,(void *)BarberM,N1+i);
    }
    //Start N3 Unisex Barbers
    for(int i=0;i<N3;i++)
    {
        status=pthread_create(&BarbersU[i],NULL,(void *)BarberU,N1+N2+i);
    }

    //Start Producing Customers
    while(1){

        newCustomer.CustomerID = CustomerCount;

        //get random variable
        rand_val=rand();

        //seed the RNG
        srand(rand_val);

        //Get sex from random int
        if(rand_val%2==0){
            newCustomer.CustomerSEX = 'F';
        }
        else{
            newCustomer.CustomerSEX = 'M';
        }

        printf("Customer [%d] - %c created\n",newCustomer.CustomerID,newCustomer.CustomerSEX);
        CustomerArrives(newCustomer);


        CustomerCount++;
        wait_rand(CustomerSpawnCooldown);
    }


    return 0;
}

void SetUp(){
    for(int i=0;i<M;i++)
    {
        WaitingRoom[i].CustomerSEX = 'E';
    }
}

void wait_rand(int multiplier){
    int x=wait_rand_len(multiplier);
    usleep(x);
}

int wait_rand_len(int multiplier){
    int x = rand() % (5000000 + 1 - 2000000) + 1000000;
    srand(x);
    return x*multiplier;
}

int getNextCustomer(char Ctype)
{
    if(Ctype == 'U'){
        for(int i=0;i<M;i++){
        if(WaitingRoom[i].CustomerSEX != 'E')
        {
            //WaitingRoom[i].CustomerSEX=='E';
            return i;
        }
    }
    }

    for(int i=0;i<M;i++){
        if(WaitingRoom[i].CustomerSEX == Ctype)
        {
            //WaitingRoom[i].CustomerSEX=='E';
            return i;
        }
    }
    return -1;
}

int getEligibleSleepingBarber(char Ctype)
{
    int BarberID = -1;
    int semval;
    if(Ctype = 'F'){
        //
        for(int i=0;i<N1;i++)
        {
            sem_getvalue(&FBarberSleeps[i],&semval);
            if(semval <= 0){
                return i;
            }
        }
    }
    else{ //Ctype = 'M'
        for(int i=0;i<N2;i++)
        {
            sem_getvalue(&MBarberSleeps[i],&semval);
            if(semval <= 0){
                return i;
            }
        }
    }

    //No sleeping single-sex Barbers, lets look if there is a Unisex one aviable


    return -1;//No aviable barber
}

int getEligibleSleepingBarberU()
{
    int semval;

    for(int i=0;i<N3;i++)
    {
        sem_getvalue(&UBarberSleeps[i],&semval);
        if(semval <= 0){
            return i;
        }
    }
    return -1;
}

int getEmptySeatIndex()
{
    for(int i=0;i<M;i++){
        if(WaitingRoom[i].CustomerSEX == 'E'){
            return i;
        }
    }

    return -1;
}

void* BarberM(int tmp) //Barber that cuts only M - male customers
{
    int MyIndex = (int *)(tmp);
    int sleeplen;
    int sleeptotal;
    int myCustomer;
    int semval;
    struct Customer currentCustomer;

    printf("Barber M [ID:%d] created\n",MyIndex);
    wait_rand(1);

    while(1){
    sleeplen=0;

    //printf("Barber M [ID:%d] ",MyIndex);
    //printQ();

    //Go to waiting room and check if there are eligible customers
    myCustomer = getNextCustomer('M');


    if(myCustomer==-1)
    {
        //set BarberSleeping semaphore
        printf("Barber M [ID:%d] is sleeping\n",MyIndex);
        sem_wait(&MBarberSleeps[MyIndex]);
        sleeptotal = wait_rand_len(BarberSleepLen);
        while(sleeplen<sleeptotal){
            //check semaphore
            sem_getvalue(&MBarberSleeps[MyIndex],&semval);
            if(semval > 0){
                break;
            }
            sleeplen++;
            usleep(1);
        }
        //This barber was woken up. Lets get the customer!
        currentCustomer.CustomerID = WaitingRoom[M].CustomerID;
        currentCustomer.CustomerSEX = WaitingRoom[M].CustomerSEX;
        WaitingRoom[M].CustomerSEX='E';

        printf("Barber M [ID:%d] is cutting customer [ID:%d]\n",MyIndex,currentCustomer.CustomerID);
        //Cut hair
        wait_rand(CutMultiplierMale);
        //dissmiss customer
        printf("Barber M [ID:%d] is done cutting customer [ID:%d]\n",MyIndex,currentCustomer.CustomerID);

        continue;
    }
    else
    {
        printf("Barber M [ID:%d] gets customer from chair %d\n",MyIndex,myCustomer);
        //Get customer into the chair
        currentCustomer.CustomerID = WaitingRoom[myCustomer].CustomerID;
        currentCustomer.CustomerSEX = WaitingRoom[myCustomer].CustomerSEX;
        WaitingRoom[myCustomer].CustomerSEX='E';

        //printf("Barber M [ID:%d] ",MyIndex);
        //printQ();

        printf("Barber M [ID:%d] is cutting customer [ID:%d]\n",MyIndex,currentCustomer.CustomerID);
        //Cut hair
        wait_rand(CutMultiplierMale);
        //dissmiss customer
        printf("Barber M [ID:%d] is done cutting customer [ID:%d]\n",MyIndex,currentCustomer.CustomerID);
        //kill(myCustomer,SIGTERM);
        continue;
    }


    }
}

void* BarberF(int tmp) //Barber that cuts only M - male customers
{
    int MyIndex = tmp;
    int sleeplen;
    int sleeptotal;
    int myCustomer;
    int semval;
    struct Customer currentCustomer;

    printf("Barber F [ID:%d] created\n",MyIndex);
    wait_rand(1);

    while(1){
    sleeplen=0;

    //printf("Barber F [ID:%d] ",MyIndex);
    //printQ();

    //Go to waiting room and check if there are eligible customers
    myCustomer = getNextCustomer('F');


    if(myCustomer==-1)
    {
        //set BarberSleeping semaphore
        printf("Barber F [ID:%d] is sleeping\n",MyIndex);
        sem_wait(&FBarberSleeps[MyIndex]);
        sleeptotal = wait_rand_len(BarberSleepLen);
        while(sleeplen<sleeptotal){
            //check semaphore
            //printf("Barber [ID:%d], F is sleeping\n",sleeplen);
            sem_getvalue(&MBarberSleeps[MyIndex],&semval);
            if(semval > 0){
                break;
            }
            sleeplen++;
            usleep(1);
        }
        //This barber was woken up. Lets get the customer!
        currentCustomer.CustomerID = WaitingRoom[M].CustomerID;
        currentCustomer.CustomerSEX = WaitingRoom[M].CustomerSEX;
        WaitingRoom[M].CustomerSEX='E';

        printf("Barber F [ID:%d] is cutting customer [ID:%d]\n",MyIndex,currentCustomer.CustomerID);
        //Cut hair
        wait_rand(CutMultiplierFemale);
        //dissmiss customer
        printf("Barber F [ID:%d] is done cutting customer [ID:%d]\n",MyIndex,currentCustomer.CustomerID);

        continue;
    }
    else
    {
        printf("Barber F [ID:%d] gets customer from chair %d\n",MyIndex,myCustomer);
        //Get customer into the chair
        currentCustomer.CustomerID = WaitingRoom[myCustomer].CustomerID;
        currentCustomer.CustomerSEX = WaitingRoom[myCustomer].CustomerSEX;
        WaitingRoom[myCustomer].CustomerSEX='E';

        //printf("Barber F [ID:%d] ",MyIndex);
        //printQ();

        printf("Barber F [ID:%d] is cutting customer [ID:%d]\n",MyIndex,myCustomer);
        //Cut hair
        wait_rand(CutMultiplierFemale);
        //dissmiss customer
        printf("Barber F [ID:%d] is done cutting customer [ID:%d]\n",MyIndex,myCustomer);
        //kill(myCustomer,SIGTERM);
        continue;
    }


    }
}

void* BarberU(int tmp) //Barber that cuts both M and F customers
{
    int MyIndex = tmp;
    int sleeplen;
    int sleeptotal;
    int myCustomer;
    int semval;
    struct Customer currentCustomer;

    printf("Barber U [ID:%d] created\n",MyIndex);
    wait_rand(1);

    while(1){
    sleeplen=0;

    //printf("Barber U [ID:%d] ",MyIndex);
    //printQ();

    //Go to waiting room and check if there are eligible customers
    myCustomer = getNextCustomer('U');


    if(myCustomer==-1)
    {
        //set BarberSleeping semaphore
        printf("Barber U [ID:%d] is sleeping\n",MyIndex);
        sem_wait(&UBarberSleeps[MyIndex]);
        sleeptotal = wait_rand_len(BarberSleepLen);
        while(sleeplen<sleeptotal){
            //check semaphore
            sem_getvalue(&MBarberSleeps[MyIndex],&semval);
            if(semval > 0){
                break;
            }
            sleeplen++;
            usleep(1);
        }
        //This barber was woken up. Lets get the customer!
        currentCustomer.CustomerID = WaitingRoom[M].CustomerID;
        currentCustomer.CustomerSEX = WaitingRoom[M].CustomerSEX;
        WaitingRoom[M].CustomerSEX='E';

        printf("Barber U [ID:%d] is cutting customer [ID:%d]\n",MyIndex,currentCustomer.CustomerID);
        //Cut hair
        wait_rand(CutMultiplierUnisex);
        //dissmiss customer
        printf("Barber U [ID:%d] is done cutting customer [ID:%d]\n",MyIndex,currentCustomer.CustomerID);

        continue;
    }
    else
    {
        printf("Barber U [ID:%d] gets customer from chair %d\n",MyIndex,myCustomer);
        //Get customer into the chair
        currentCustomer.CustomerID = WaitingRoom[myCustomer].CustomerID;
        currentCustomer.CustomerSEX = WaitingRoom[myCustomer].CustomerSEX;
        WaitingRoom[myCustomer].CustomerSEX='E';


        //printf("Barber U [ID:%d] ",MyIndex);
        //printQ();

        printf("Barber U [ID:%d] is cutting customer [ID:%d]\n",MyIndex,myCustomer);
        //Cut hair
        wait_rand(CutMultiplierUnisex);
        //dissmiss customer
        printf("Barber U [ID:%d] is done cutting customer [ID:%d]\n",MyIndex,myCustomer);
        //kill(myCustomer,SIGTERM);
        continue;
    }


    }
}

void CustomerArrives(struct Customer NewCustomer)
{
    int seatIndex;
    int BarbIndex;
    //Customer was generated and arrives at the hair salon
    printf("Customer [%d] arrived at the hair salon.\n",NewCustomer.CustomerID);
    //Is there a eligible barber sleeping?
    BarbIndex = getEligibleSleepingBarber(NewCustomer.CustomerSEX);
    if(BarbIndex==-1){
        BarbIndex=getEligibleSleepingBarberU();
        if(BarbIndex != -1)
        {
            sem_post(&UBarberSleeps[BarbIndex]);
            printf("Customer [%d] found and woke a barber up.\n",NewCustomer.CustomerID);
            WaitingRoom[M].CustomerID=NewCustomer.CustomerID;
            WaitingRoom[M].CustomerSEX=NewCustomer.CustomerSEX;
            return;
        }
    }
    else
    {
        if(NewCustomer.CustomerSEX=='F')
        {
            sem_post(&FBarberSleeps[BarbIndex]);
            printf("Customer [%d] found and woke a barber up.\n",NewCustomer.CustomerID);
            WaitingRoom[M].CustomerID=NewCustomer.CustomerID;
            WaitingRoom[M].CustomerSEX=NewCustomer.CustomerSEX;
            return;
        }
        else
        {
            sem_post(&MBarberSleeps[BarbIndex]);
            printf("Customer [%d] found and woke a barber up.\n",NewCustomer.CustomerID);
            WaitingRoom[M].CustomerID=NewCustomer.CustomerID;
            WaitingRoom[M].CustomerSEX=NewCustomer.CustomerSEX;
            return;
        }
    }
        //Yes - go and get them
        //return

    //Is there space in the queue?
    seatIndex = getEmptySeatIndex();
    if(seatIndex==-1)
    {
        //No seats in the waiting room
        printf("Customer [%d] found no empty seat and left the hair salon.\n",NewCustomer.CustomerID);
        printQ();
    }
    else
    {
        WaitingRoom[seatIndex].CustomerID = NewCustomer.CustomerID;
        WaitingRoom[seatIndex].CustomerSEX = NewCustomer.CustomerSEX;
        printf("Customer [%d] found no aviable barbers and took place in the waiting room.\n",NewCustomer.CustomerID);
        printQ();
    }

}

void printQ()
{
    for(int i=0;i<M;i++)
    {
        printf("%c-%d/",WaitingRoom[i].CustomerSEX,WaitingRoom[i].CustomerID);
    }
    printf("\n");
}
