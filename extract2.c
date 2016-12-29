/*
 * This tools can extract some pieces from big file.
 * Dev by Hui Zhou, Dawei Mu at SDSC, UCSD.
 * gcc -Wall -g -D_FILE_OFFSET_BITS=64 extract.c   -o extract
 * ./extract2 ./VX_120G 10 1000 1000 4 1 2000 ./output.txt
 * On BWS ./extract2 /scratch/sciteam/droten/SAF_dyn/cvmsi25_mid_dmz/output_vlm/VZ 10560 5 1000 4 8950 9050   ~/VZ_D_08950-09050
 * 网上说在c文件中定义：#define _FILE_OFFSET_BITS 64,无效。
 */
#include <stdio.h> //标准输入输入出的头文件,printf和scanf都在这里了
#include <stdlib.h>  //标准库头文件，内存分配、数学运算等都在这里了
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define FILE_OFFSET_BITS 64
#define _LARGEFILE64_SOURCE  1   /* See feature_test_macros(7) */
#define LARGEFILE64_SOURCE  1 
//#define __USE_LARGEFILE64
#define _LARGE_FILES
#ifndef O_LARGEFILE
//#define O_LARGEFILE 0
#endif

#define MAXLEN 1024

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
  #ifdef _LARGE_FILES
        printf("Defined _LARGE_FILES.\n");
  #else
        printf("Not Defined _LARGE_FILES.\n");
  #endif

  printf("sizeof(long)=%lu, sizeof(size_t)=%lu, sizeof(off_t)=%lu\n", sizeof(long), sizeof(size_t), sizeof(off_t));
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
        buffersize = 64 * 1024 * 1024 * ElementByteSize;
        buffersize = 64 * 1024 * 1024;

        unsigned char *buffer;
        buffer = (unsigned char *)malloc(buffersize );
        if (buffer == NULL)
        {
                printf("ERROR: buffer malloc.\n");
                return 1;
        }

        time(&start);

        //FILE * outfile, *infile;
        int fdin, fdout;
        //outfile = fopen(argv[8], "wb" );
        fdout = open(argv[8], O_WRONLY|O_CREAT);
        if (fdout == -1)
        {
                printf("%s, %s",argv[8],"can't open for write.\n");
                exit(1);
        }

        //infile = fopen(argv[1], "rb");
        fdin = open(argv[1], O_RDONLY);
        if (fdin == -1)
        {
                printf("%s, %s",argv[1],"can't open.\n");
                exit(1);
        }

        //locate file pointer to start position in input file
        // fpos_t offset_start = nx*ny*nz*ElementByteSize * StartTime;
        // fpos_t offset_end = nx*ny*nz*ElementByteSize * (EndTime+1);
        off_t offset =       (off_t)nx*ny*nz*ElementByteSize * (off_t)StartTime;
        off_t offset_start = (off_t)nx*ny*nz*ElementByteSize * (off_t)StartTime;
        off_t offset_end =   (off_t)nx*ny*nz*ElementByteSize * (off_t)(EndTime+1);
        //printf("offset:%ld,offset_start:%ld, offset_end:%ld\n",offset,offset_start, offset_end);
        //printf("offset:%ld,offset_start:%ld, offset_end:%ld\n",offset,offset_start, offset_end);
        // fseek(infile, 0, SEEK_SET);
        // fsetpos(infile, &offset_start);
        //int fd = fileno(infile);
        //if(set_offset(fd, offset_start)<0)
        //printf("Debug offset_start:%ld,offset_end:%ld\n", offset_start, offset_end);
        lseek(fdin,0,SEEK_SET);
        if(lseek(fdin, offset_start,SEEK_SET)<0)
        {
                printf("%s %s",argv[1],"can't set offset.\n");
                close(fdin);
                close(fdout);
                exit(1);
        }
        size_t times = 0;
        size_t AmountTimes = ((offset_end - offset_start) / (buffersize));
        //printf("offset:%ld,offset_start:%ld, offset_end:%ld\n",offset,offset_start, offset_end);
        printf("nx:%d,ny:%d,nz:%d,ElementSize:%d,StartTime:%d,EndTime:%d\n",nx,ny,nz,ElementByteSize,StartTime,EndTime);
        printf("offset from %ld to %ld.\n", offset_start, offset_end);

        while(1)
        {
                if(offset >= offset_end) break;

                //size_t datasizer = fread(buffer, 1, buffersize, infile);
                ssize_t datasizer = read(fdin, buffer, buffersize);
                if(datasizer <= 0)
                {
                        printf("Reading file occurs some error. Request:%d, Actually got:%d\n", buffersize, datasizer);
                        break;
                }
                offset += datasizer;
                //printf("Debug offset:%ld\n", offset);
                if(offset > offset_end)
                {
                    //size_t datasizew = fwrite(buffer, 1, datasizer -(offset-offset_end), outfile);
                    ssize_t datasizew = write(fdout, buffer, datasizer -(offset-offset_end));
                    if(datasizer -(offset-offset_end) != datasizew)
                    {
                            printf("Writting output file occurs some error.");
                    }
                    offset = offset_end;
                    break;
                }else
                {
                    //size_t datasizew = fwrite(buffer, 1, datasizer, outfile);
                    ssize_t datasizew = write(fdout, buffer, datasizer);
                    if(datasizer != datasizew)
                    {
                            printf("Writting output file occurs some error.");
                            break;
                    }
                }
                printf("\r%ld / %ld ...", times++, AmountTimes);
                fflush(stdout);

        }

        //fclose(infile);
        //fclose(outfile);
        close(fdin);
        close(fdout);

        time(&end);

        if (buffer != NULL) free(buffer);
        printf("\n%s generated Done. %lld bytes, %ld seconds.\r\n", argv[8], offset- offset_start, end-start);
        return 0;
}
