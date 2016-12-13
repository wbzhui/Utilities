#include <stdio.h> //标准输入输入出的头文件,printf和scanf都在这里了
#include <stdlib.h>  //标准库头文件，内存分配、数学运算等都在这里了
#include <string.h>

#define MAXLEN 1024
int main(int argc, char *argv[])
{
    if(argc < 2 )
    {
        printf("usage: %s %s", argv[0], "binfile [type:uchar/char/int/float/double] [wrapnum:1,2.....]\n");
        printf("default type: int.\n");
        printf("example:\n");
        printf("  1.%s file.bin int 2 #print file.bin as integer and every 2 numbers write a wrap char.\n", argv[0]);
        printf("  2.%s file.bin       #print file.bin as integer without any wrap char.\n", argv[0]);
        exit(1);
    }
    char type[10] = "\0";
    int wrapnum = 0;
    if(argc ==2)
    {
        sprintf(type,"int");
    }
    else
    {
        strcpy(type,argv[2]); 
    }
    if(argc ==4)
    {
        wrapnum= atoi(argv[3]);
    }
    
    FILE * outfile, *infile;
    //outfile = fopen(argv[2], "wb" );
    infile = fopen(argv[1], "rb");
    unsigned char buf[MAXLEN];
    if(infile == NULL )
    {
        printf("%s, %s",argv[1],"not exit.\n");
        exit(1);
    }
    int rc;
    int amount = 0;
    if(strcmp(type,"int")==0)
    {
        int result=0;
        while( (rc = fread(&result, sizeof(int), 1, infile)) != 0 )
        {
            printf("%d ",result);
            amount++;
            if(wrapnum!=0 && amount%wrapnum==0) printf("\r\n");
        }
    }else if(strcmp(type,"uchar")==0)
    {
        unsigned char result;
        while( (rc = fread(&result, sizeof(unsigned char), 1, infile)) != 0 )
        {
            printf("%02X",result);
            amount++;
            if(wrapnum!=0 && amount%wrapnum==0) printf("\r\n");
        }
    }else if(strcmp(type,"char")==0)
    {
        char result;
        while( (rc = fread(&result, sizeof(char), 1, infile)) != 0 )
        {
            printf("%c",result);
            amount++;
            if(wrapnum!=0 && amount%wrapnum==0) printf("\r\n");
        }
    }else if(strcmp(type,"float")==0)
    {
        float result;
        while( (rc = fread(&result, sizeof(float), 1, infile)) != 0 )
        {
            printf("%5.2f,",result);
            amount++;
            if(wrapnum!=0 && amount%wrapnum==0) printf("\r\n");
        }
    
    }else if(strcmp(type,"double")==0)
    {
        double result;
        while( (rc = fread(&result, sizeof(double), 1, infile)) != 0 )
        {
            printf("%e,",result);
            amount++;
            if(wrapnum!=0 && amount%wrapnum==0) printf("\r\n");
        }
    }
    
    fclose(infile);
    
    printf("\r\nFound %d elments. Done.\r\n", amount);
    return 0;
}
