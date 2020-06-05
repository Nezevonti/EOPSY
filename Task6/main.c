#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void HelpFunc()
{   //Print help and exit the program
    printf("copy [-m] <source_filename> <destination_filename> - using memcopy()\n");
    printf("copy <source_filename> <destination_filename> - using read()\n");
    printf("copy  or copy -h - help. This menu\n");
    exit(0);
}

void DoMem(char* In, char* Out)
{   //init
    int sourcefile, destfile;
    char *src, *dest;
    size_t filesize;
    struct stat sbuf;


    //printf("Do memory stuff\n");
    //open files
    sourcefile = open(In,O_RDONLY);
    destfile = open(Out, O_RDWR | O_CREAT,0666);

    //check if both were opened properly
    if(sourcefile == -1 || destfile == -1){
        perror("Open error\n");
        exit(1);
    }

    //get size of the input file
    fstat(sourcefile,&sbuf);

    //map the input file to memory
    src = mmap(NULL,sbuf.st_size,PROT_READ,MAP_PRIVATE,sourcefile,0);


    //truncate the memory
    ftruncate(destfile,sbuf.st_size);

    //map the destination
    dest = mmap(NULL, sbuf.st_size,PROT_READ | PROT_WRITE, MAP_SHARED,destfile,0);

    //Copy from source mapping to the destination
    memcpy(dest,src,sbuf.st_size);
    //Delete the mapping
    munmap(src, sbuf.st_size);
    munmap(dest, sbuf.st_size);

    //close files
    close(sourcefile);
    close(destfile);


    //printf("Do memory stuff\n");
    exit(0);
}

void DoRead(char* In, char* Out)
{
    int filesize;
    int sourcefile, destfile;


    //printf("Do read stuff\n");
    //open both files
    sourcefile = open(In,O_RDONLY);
    destfile = open(Out, O_RDWR | O_CREAT,0666);

    //Check if both were opend properly
    if(sourcefile == -1 || destfile == -1){
        perror("Open error\n");
        exit(1);
    }

    //find the filesize by locating the end of src file.
    filesize = lseek(sourcefile,0,SEEK_END);
    lseek(sourcefile,0,SEEK_SET);

    //init buffer
    char buffer[filesize];

    //read from source into buffer
    read(sourcefile,buffer,filesize);
    //write into dest from the buffer
    write(destfile,buffer,filesize);

    //close files
    close(sourcefile);
    close(destfile);


    //printf("Do read stuff\n");
    exit(0);
}

int main(int argc,char *argv[])
{
    //Declare all the variables used in main.
    int option; //option or parameter
    bool flag; //flag used to save choice of method.
    char* SourceFile; //String with name of the source file
    char* DestFile; //String with name of the destination file

    /* Printing number and all parameters, used for debugging during development
    printf("%d \n",argc);
    for(int i=0;i<argc;i++)
    {
        printf("%s \n",argv[i]);
    }
    printf("--- --- ---\n");
    */

    if(argc==1) HelpFunc(); //If there was only 1 parameter (only program name) go to help


    //iterate through the parameters. The getopt returns the parameter option (-a/ -o/ -g etc)
    while((option = getopt(argc,argv,"mh"))!=-1)
    {
        //for every option, look what ot is.
        //If there is -h only it will be acted upon. -m will be ignored
        //If there is an -m elsewhere in the call it will choose it insted of the read() option
        switch(option)
        {
            case 'h': //-h option h was chosen
                HelpFunc();
                break;
            case 'm': //-m option was chosen
                //DoMem()
                flag=true;
                break;
            default: //no option was choosen, use the read() route.
                //DoRead()
                flag=false;
                break;
        }
    }
    //All options were iterated. If the command line parameters were input properly
    //Then the next arguments will be names of the files
    SourceFile = argv[optind];
    DestFile = argv[optind+1];


    //Print destination and source filenames
    printf("src : %s \n",SourceFile);
    printf("dst : %s \n",DestFile);

    //Check if the filenames are not null. If one/both of them is - print a message and exit
    if(SourceFile == NULL || DestFile == NULL)
    {
        printf("One of input files is null... wrong input... exiting\n");
        exit(0);
    }

    if(flag) //check the flag that was set int the getopt loop and choose right function.
    {
        DoMem(SourceFile,DestFile);
    }
    else
    {
        DoRead(SourceFile,DestFile);
    }

    return 0;
}
