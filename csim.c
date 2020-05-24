//#include "cachelab.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char flag=0;//marks whether to enable -v option
int setBits=0;
int associativity=0;
int blockBits=0;
char *traceName=NULL;

int setSize;
int blockSize;

int hitTime=0;;
int missTime=0;
int evictionTime=0;

void printHelpInfo(){
    printf("./csim [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n");
    printf("\t-v\tOptional verbose flag that displays trace info\n");
    printf("\t-s <s>\tNumber of set index bits\n");
    printf("\t-E <E>\tAssociativity\n");
    printf("\t-b <b>\tNumber of block bits\n");
    printf("\t-t <tracefile>\tName of valgrind trace to display\n");
}

int getBlockNumber(unsigned long address,int blockBits){
    int mask=1;
    int i;
    for(i=1;i<=blockBits-1;i++){
        mask<<=1;
        mask++;
    }
    return address&mask;
}

int getSetNumber(unsigned long address,int setBits,int blockBits){
    int mask=1<<blockBits;
    int i;
    for(i=1;i<=setBits-1;i++){
        mask<<=1;
        mask+=(1<<blockBits);
    }
    return (address&mask)>>blockBits;
}

int getTagNumber(unsigned long address,int setBits,int blockBits){
    int mask=1;
    for(i=1;i<=setBits+blockBits-1;i++){
        mask<<=1;
        mask+=1;
    }
    mask=~mask;
    return (address&mask)>>(setBits+blockBits);
}

void store(char v,char *cache,unsigned long address,int oprtsize){
    int setNumber=getSetNumber(address,setBits,blockBits);
    char *tagptr=cache+setSize*setNumber;
    char flag=0;
    int i;
    for(i=0;i<associativity;i++){
        if( *(int *)tagptr==setNumber && *(tagptr+1)) {flag=1;break;}
    }
    if(!v){
        if(flag) {
            hitTime++;
            printf(" S %x,%d hit\n",address,oprtsize);
        }
        else {
            missTime++;
        }
    }
}

int main(int argc,char **argv){
    const char *optString="vs:E:b:t:";
    

    if(argc==2){
        if(argv[1][1]=='h') printHelpInfo();
        else printf("Wrong!\n");
    }
    
    char opt=getopt(argc,argv,optString);
    while(opt!=-1){
        switch(opt){
            case 'v':
                flag=1;
                break;
            case 's':
                setBits=atoi(optarg);
                break;
            case 'E':
                associativity=atoi(optarg);
                break;
            case 'b':
                blockBits=atoi(optarg);
                break;
            case 't':
                traceName=optarg;
                break;
        }
        opt=getopt(argc,argv,optString);
    }


    /*printf("%d\n",flag);
    printf("%d\n",setBits);
    printf("%d\n",associativity);
    printf("%d\n",blockBits);
    printf("%s\n",traceName);*/


    if(!setBits||!associativity||!blockBits||!traceName) {
        printf("Lack of arguments.\n");
        return 0;
    }


    FILE *trace=fopen(traceName,"r");
    if(!trace){
        printf("Can not open file \"%s\".\n",trace);
        return 0;
    }
    int cachesize;
    char *cache=(char *)malloc(cachesize=((1<<setBits))*
                                        (associativity)*
                                        (blockSize=(1<<blockBits)+4+sizeof(int)));
    setSize=blockSize*associativity;

    // block structure: tag[4]:validbit[1]:dirtybit[1]:times[2]:bytes
    memset(cache,0,cachesize);
    char order,comma;
    unsigned long address,oprtsize;
    while(!feof(trace)){
        fscanf(trace,"%c%x%c%d",&order,&address,&comma,&oprtsize);
        switch(order){
            case 'I':break;
            case 'S':
                //printf("S %x%c%d\n",address,comma,oprtsize);
                if(flag){

                }else{
                    if(store(cache,address,oprtsize)) hitTime++;
                    else missTime++;
                }
                break;
            case 'L':
                printf("L %x%c%d\n",address,comma,oprtsize);
                break;
            case 'M':
                printf("M %x%c%d\n",address,comma,oprtsize);
                break;
        }
    }
    fclose(trace);
    free(cache);
    return 0;
}
