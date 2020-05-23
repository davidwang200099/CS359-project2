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

void printHelpInfo(){
    printf("./csim [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n");
    printf("\t-v\tOptional verbose flag that displays trace info\n");
    printf("\t-s <s>\tNumber of set index bits\n");
    printf("\t-E <E>\tAssociativity\n");
    printf("\t-b <b>\tNumber of block bits\n");
    printf("\t-t <tracefile>\tName of valgrind trace to display\n");
}

int getBlockNumber(long address,int blockBits){
    int mask=1;
    int i;
    for(i=1;i<=blockBits-1;i++){
        mask<<=1;
        mask++;
    }
    return address&mask;
}

int getSetNumber(long address,int setBits,int blockBits){
    int mask=1<<blockBits;
    int i;
    for(i=1;i<=setBits-1;i++){
        mask<<=1;
        mask+=(1<<blockBits);
    }
    return address&mask;
}
void store(char *cache,long address,int oprtsize){
    int setsize=associativity*(2+64+(1<<blockBits));
    int setNumber=getSetNumber(address,setBits,blockBits);
    
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
    char *cache=(char *)malloc(cachesize=(1<<setBits)*associativity*((1<<blockBits)+2+sizeof(long)));
    // block structure: validbit:dirtybit:tag:bytes
    memset(cache,0,cachesize);
    char order,comma;
    long address,oprtsize;
    while(!feof(trace)){
        fscanf(trace,"%c%x%c%d",&order,&address,&comma,&oprtsize);
        switch(order){
            case 'I':break;
            case 'S':
                printf("S %x%c%d\n",address,comma,oprtsize);
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
