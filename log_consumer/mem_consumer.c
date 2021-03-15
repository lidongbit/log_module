#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h> //mmap head file
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include "double_fifo.h"
#include <string.h>
#include <sys/shm.h>

#define LOG_MSG_CONTROL_ADDR 0x3E000000
#define LOG_MSG_ADDR         0x3E100000

#define TEST_MMAP
//#define TEST_SHM

DEBUG_RINGS_BUFF_STRUCT *map_ctrl;
char *map_msg;

static void log_end(void)
{
#ifdef TEST_SHM
   if(shmdt(shm_ctrl) == -1 || shmdt(shm_msg) == -1)
   {
      fprintf(stderr, "shmdt failed\n");
      exit(EXIT_FAILURE);
   }
#endif
#ifdef TEST_MMAP
   munmap( map_ctrl, sizeof(DEBUG_RINGS_BUFF_STRUCT) );
   munmap( map_msg, 1024*1024);
   printf( "umap ok \n" );
#endif
   close_log_file();
}
int main (int argc, char *argv[])
{
   open_log_file();
#ifdef TEST_SHM
   int i;
   //char *ctr,*msg;
   void *shm_ctrl = NULL;
   void *shm_msg = NULL;
   DEBUG_RINGS_BUFF_STRUCT *shared_ctrl = NULL;
   int shmid_ctrl,shmid_msg;  //创建共享内存
   
   shmid_ctrl = shmget((key_t)1234, sizeof(DEBUG_RINGS_BUFF_STRUCT), 0666|IPC_CREAT);
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
   printf("Memory attached at 0x%X,0x%X\n", (int)shm_ctrl,(int)shm_msg);    //设置共享内存   

   buff_init(shm_ctrl,shm_msg);
#endif
#ifdef TEST_MMAP
   int fd_ctrl,fd_msg,i;

   if(argc!=3)
   {
       printf("Usage:...\n");
       exit(0);
   }
   fd_ctrl = open(argv[1],O_RDWR,00777);
   fd_msg = open(argv[2],O_RDWR,00777);

   lseek(fd_ctrl,sizeof(DEBUG_RINGS_BUFF_STRUCT),SEEK_SET);
   write(fd_ctrl,"1",1);
   lseek(fd_msg,1024*1024,SEEK_SET);
   write(fd_msg,"1",1);

   map_ctrl = (DEBUG_RINGS_BUFF_STRUCT*) mmap( NULL,sizeof(DEBUG_RINGS_BUFF_STRUCT),PROT_READ|PROT_WRITE,MAP_SHARED,fd_ctrl,0 );
   map_msg = (char*) mmap( NULL,1024*1024,PROT_READ|PROT_WRITE,MAP_SHARED,fd_msg,0 );

   close(fd_ctrl);
   close(fd_msg);
   buff_init(map_ctrl,map_msg);
   printf("consumer init ok!\n");
#endif
   //Read old value
   while(1)
   {
      copy_buff();
      print_buff();
      usleep(50000);
   }
   atexit(log_end);
   sleep(2);  
   exit(EXIT_SUCCESS);
   return 0;
}
