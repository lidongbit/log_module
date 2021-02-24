#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h> //mmap head file
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include "Force_Log.h"
#include <string.h>
#include <sys/shm.h>

//#define TEST_SHM
#define TEST_MMAP

int main (int argc, char*argv[])
{
#ifdef TEST_SHM
   int i;
   double df = 3.14;
   //char *ctr,*msg;
   void *shm_ctrl = NULL;
   void *shm_msg = NULL;
   FORCE_DEBUG_RINGS_BUFF_STRUCT *shared_ctrl = NULL;
   int shmid_ctrl,shmid_msg;

   //创建共享内存
   shmid_ctrl = shmget((key_t)1234, sizeof(FORCE_DEBUG_RINGS_BUFF_STRUCT), 0666|IPC_CREAT);
   shmid_msg = shmget((key_t)2345, 1024*1024, 0666|IPC_CREAT);
   if( shmid_msg == -1 || shmid_ctrl == -1)
   {  
      fprintf(stderr, "shmget failed\n");
      exit(EXIT_FAILURE);
   } 

   //将共享内存连接到当前进程的地址空间
   shm_ctrl = shmat(shmid_ctrl, (void*)0, 0); 
   shm_msg = shmat(shmid_msg,(void*)0,0);  
   if(shm_ctrl == (void*)-1 || shm_msg == (void*)-1)
   {  
      fprintf(stderr, "shmat failed\n");     
      exit(EXIT_FAILURE);
   }  
   printf("Memory attached at 0x%X,0x%X\n", (int)shm_ctrl,(int)shm_msg);

   //设置共享内存
   Force_Debug_Init(shm_ctrl,shm_msg);
   printf("produce init ok!\n");
#endif
#ifdef TEST_MMAP
   int fd_ctrl,fd_msg,i;
   FORCE_DEBUG_RINGS_BUFF_STRUCT *map_ctrl;
   char *map_msg;

   if(argc!=3)
   {
       printf("Usage:...\n");
       exit(0);
   }

   fd_ctrl = open(argv[1],O_CREAT|O_RDWR|O_TRUNC,00777);
   fd_msg = open(argv[2],O_CREAT|O_RDWR|O_TRUNC,00777);

   lseek(fd_ctrl,sizeof(FORCE_DEBUG_RINGS_BUFF_STRUCT),SEEK_SET);
   write(fd_ctrl,"1",1);
   lseek(fd_msg,1024*1024,SEEK_SET);
   write(fd_msg,"1",1);

   map_ctrl = (FORCE_DEBUG_RINGS_BUFF_STRUCT*) mmap( NULL,sizeof(FORCE_DEBUG_RINGS_BUFF_STRUCT),PROT_READ|PROT_WRITE,MAP_SHARED,fd_ctrl,0 );
   map_msg = (char*) mmap( NULL,1024*1024,PROT_READ|PROT_WRITE,MAP_SHARED,fd_msg,0 );

   close(fd_ctrl);
   close(fd_msg);
   Force_Debug_Init(map_ctrl,map_msg);
   printf("produce init ok!\n");
#endif
   while(1)
   {
      i++;
      Log_info("hello:%d",i);
      printf("i:%d\n",i);
      usleep(10000);

      if(i==4095)
          continue;
      if(i>4097)
         break;
   }
#ifdef TEST_SHM
   if(shmdt(shm_ctrl) == -1 || shmdt(shm_msg) == -1)   
   {      
      fprintf(stderr, "shmdt failed\n");     
      exit(EXIT_FAILURE);
   }  
#endif
#ifdef TEST_MMAP
   munmap( map_ctrl, sizeof(FORCE_DEBUG_RINGS_BUFF_STRUCT) );
   munmap( map_msg, 1024*1024);
   printf( "umap ok \n" );
#endif
   sleep(2);  
   exit(EXIT_SUCCESS);
}
