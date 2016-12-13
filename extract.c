/*
 * This tools can extract some pieces from big file.
 * Dev by Hui Zhou, Dawei Mu at SDSC, UCSD.
 * gcc -Wall -g -D_FILE_OFFSET_BITS=64 extract.c   -o extract 
 * extract ./VX_120G 10 1000 1000 4 1 2000 ./output.txt
 * 网上说在c文件中定义：#define _FILE_OFFSET_BITS 64,无效。
 */
#include <stdio.h> //标准输入输入出的头文件,printf和scanf都在这里了
#include <stdlib.h>  //标准库头文件，内存分配、数学运算等都在这里了
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define _LARGEFILE64_SOURCE     /* See feature_test_macros(7) */

#define MAXLEN 1024

int set_offset64(int fd,long long offset)
{
        long long i,maxl=0;
        //off64_t i, maxl;
        if(fd < 0 ) return -1;
        maxl = lseek64(fd, 0, SEEK_END);
        i=lseek64(fd, offset, SEEK_SET);
        /*if i greater than the file size of fd,
           return -EEOF and reset the file pointer to
           the start of fd.*/
        if(i >= maxl) {
                lseek64(fd,0,SEEK_SET);
                return -2;
        }
        if(i != offset) return -2;
        return 0;
}

int set_offset32(int fd,long long offset)
{
        /*if we can not use 64 bits offset,the largest
           size we can move at one time is 2^30. */
 #define SHIFT_BITS 30
        unsigned long i;
        if(fd < 0) return -1;
        i=lseek(fd,0,SEEK_SET);
        /*here i must equals 0,if not,error.*/
        if(i != 0) return -2;
        for(i=0; i<(long long)offset>> SHIFT_BITS; i++) {
/*we can only check the first result of lseek( )*/
                if(i==0) {
                        i=lseek(fd,1<<SHIFT_BITS,SEEK_CUR);
                        if(i != (1<<SHIFT_BITS))
                                return -2;
                        i=0;
                }
                else lseek(fd,1<<SHIFT_BITS,SEEK_CUR);
        }
        lseek(fd,(long long)offset%(1<<SHIFT_BITS),SEEK_CUR);
        return 0;
}

int set_offset(int fd,long long offset)
{
        int i;
#ifdef  __USE_LARGEFILE64
        i=set_offset64(fd,offset);
#else
        i=set_offset32(fd,offset);
#endif
        return i;
}


int main(int argc, char *argv[])
{
  #ifdef  __USE_LARGEFILE64
        printf("Defined __USE_LARGEFILE64.\n");
  #else
        printf("Not Defined __USE_LARGEFILE64.\n");
  #endif
  #ifdef _FILE_OFFSET_BITS
        printf("Defined _FILE_OFFSET_BITS.\n");
  #else
        printf("Not Defined _FILE_OFFSET_BITS.\n");
  #endif
  printf("sizeof(long)=%lu, sizeof(size_t)=%lu\n", sizeof(long), sizeof(size_t));
        if(argc < 9 )
        {
                printf("usage: %s %s", argv[0], " orignal.file nx ny nz ElementByteSize StartTime EndTime output.file\n");

                exit(1);
        }

        time_t  start,end;

        size_t nx = atoi(argv[2]);
        size_t ny = atoi(argv[3]);
        size_t nz = atoi(argv[4]);
        size_t ElementByteSize = atoi(argv[5]);
        size_t StartTime = atoi(argv[6]);
        size_t EndTime = atoi(argv[7]);

        size_t buffersize = 0;
        buffersize = 64 * 1024 * 1024;

        unsigned char *buffer;
        buffer = (unsigned char *)malloc(buffersize * ElementByteSize);
        if (buffer == NULL)
        {
                printf("ERROR: buffer malloc.\n");
                return 1;
        }

        time(&start);

        FILE * outfile, *infile;
        outfile = fopen(argv[8], "wb" );
        if(outfile == NULL )
        {
                printf("%s, %s",argv[8],"can't open.\n");
                exit(1);
        }

        infile = fopen(argv[1], "rb");
        if(infile == NULL )
        {
                printf("%s, %s",argv[1],"can't open.\n");
                exit(1);
        }

        //locate file pointer to start position in input file
        // fpos_t offset_start = nx*ny*nz*ElementByteSize * StartTime;
        // fpos_t offset_end = nx*ny*nz*ElementByteSize * (EndTime+1);
        long long offset = nx*ny*nz*ElementByteSize * StartTime;
        long long offset_start = nx*ny*nz*ElementByteSize * StartTime;
        long long offset_end = nx*ny*nz*ElementByteSize * (EndTime+1);
        // fseek(infile, 0, SEEK_SET);
        // fsetpos(infile, &offset_start);
        int fd = fileno(infile);
        set_offset(fd, offset_start);
        size_t times = 0;
        size_t AmountTimes = ((offset_end - offset_start) / buffersize) + 1;
        while(!feof(infile))
        {
                if(offset >= offset_end) break;

                size_t datasizer = fread(buffer, ElementByteSize, buffersize, infile);
                if(datasizer <= 0)
                {
                        printf("Reading file occurs some error.\n");
                        continue;
                }
                offset += datasizer;
                if(offset > offset_end)
                {
                    size_t datasizew = fwrite(buffer, 1, datasizer -(offset-offset_end), outfile);
                    if(datasizer -(offset-offset_end) != datasizew)
                    {
                            printf("Writting output file occurs some error.");
                    }
                    offset = offset_end;
                    break;
                }else
                {
                    size_t datasizew = fwrite(buffer, 1, datasizer, outfile);
                    if(datasizer != datasizew)
                    {
                            printf("Writting output file occurs some error.");
                            break;
                    }
                }
                printf("\r%ld / %ld ...", times++, AmountTimes);
                fflush(stdout);

        }

        fclose(infile);
        fclose(outfile);

        time(&end);

        if (buffer != NULL) free(buffer);
        printf("\n%s generated Done. %lld bytes, %ld seconds.\r\n", argv[8], offset- offset_start, end-start);
        return 0;
}
