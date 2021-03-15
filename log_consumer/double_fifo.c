#include "double_fifo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <time.h>
#include <fcntl.h>           /* Definition of AT_* constants */
#include <unistd.h>

static char *in_buff;
static DEBUG_INFO_t* temp_item_ptr;
static DEBUG_RINGS_BUFF_STRUCT *in_buff_info_ptr;
static DEBUG_RINGS_BUFF_STRUCT in_buff_info;
#define LOCAL_BUFF_SIZE (1024*1024)
#define REMOTE_BUFF_SIZE (1024*1024)

static char log_name[128];
FILE *log_file = NULL;

typedef struct{
    char buff[LOCAL_BUFF_SIZE];
    int r;
    int w;
    int lost;
}LOCAL_BUFF_t;

static LOCAL_BUFF_t local_buff;

void buff_init(void *ctr, void *msg)
{
    local_buff.r=0;
    local_buff.w=0;
    local_buff.lost=0;

    in_buff = (char *)(msg);
    in_buff_info_ptr = (DEBUG_RINGS_BUFF_STRUCT *)(ctr);
}

int check_log_dir(void)
{
    if(access("/tmp/log/", F_OK) == 0) return 0;

    if(system("mkdir -p /tmp/log/"))
    {
        perror("system");
        exit(0);
    }
}

int open_log_file(void)
{
    char file_time[32] = "";
    char file_name[128] = "/tmp/log";

    time_t t = time(NULL);
    struct tm *t_now = localtime(&t);
    strftime(file_time, sizeof(file_time), "%Y%m%d%H%M%S.log", t_now);
    strcat(file_name, file_time);

    check_log_dir();
    log_file = fopen(file_name, "a+");
    if(NULL == log_file)
    {
        perror("fopen");
        exit(0);
    }
    strcpy(log_name, file_name);
    return 0;
}

void copy_buff()
{
    int ir;
    int iw;
    int or;
    int ow;

    memcpy(&in_buff_info, in_buff_info_ptr, sizeof(DEBUG_RINGS_BUFF_STRUCT));
    
    ir = in_buff_info.head_index_offset;
    iw = in_buff_info.tail_index_offset;
    or = local_buff.r;
    ow = local_buff.w;
    //printf("ir,iw,or,ow:%d %d %d %d\r\n",ir,iw,or,ow);
    if(ir<iw)
    {
        if((LOCAL_BUFF_SIZE-ow)>(iw-ir))
        {
            /*
             * in      |    R------W |temp_var_info
             * local   |   W         |
             * res     |   -------W  |
             */
            //printf("buff_len:%d,data_len:%d\r\n",1024*1024,iw-ir);
            memcpy(&(local_buff.buff[ow]), &in_buff[ir], (iw-ir));
            //printf("memcpy!\r\n");

        }else{
            /*
             * in      |    R------W |
             * local   |        W    |
             * res     |-W      -----|
             */
            memcpy(&local_buff.buff[ow], &in_buff[ir], (LOCAL_BUFF_SIZE-ow));
            memcpy(&local_buff.buff[0], &in_buff[ir+(LOCAL_BUFF_SIZE-ow)], iw-ir-(LOCAL_BUFF_SIZE-ow));

        }
        in_buff_info_ptr->head_index_offset = in_buff_info.tail_index_offset;
        local_buff.w = (local_buff.w+(iw-ir))%LOCAL_BUFF_SIZE;

    }else{
        if((LOCAL_BUFF_SIZE-ow)>=(REMOTE_BUFF_SIZE-ir+iw))
        {
            /*
             * in      |+++W     R---|
             * local   |      W      |
             * res     |      ---+++W|
             */
            memcpy(&local_buff.buff[ow], &in_buff[ir], (REMOTE_BUFF_SIZE-ir));
            memcpy(&local_buff.buff[ow+(REMOTE_BUFF_SIZE-ir)], &in_buff[0], iw);

        }else if((LOCAL_BUFF_SIZE-ow)>=(REMOTE_BUFF_SIZE-ir)){
            /*
             * in      |+++W     R---|
             * local   |        W    |
             * res     |+W      ---++|
             */
            memcpy(&local_buff.buff[ow], &in_buff[ir], (REMOTE_BUFF_SIZE-ir));
            memcpy(&local_buff.buff[ow+(REMOTE_BUFF_SIZE-ir)], &in_buff[0], (LOCAL_BUFF_SIZE-ow)-(REMOTE_BUFF_SIZE-ir));
            memcpy(&local_buff.buff[0], &in_buff[(LOCAL_BUFF_SIZE-ow)-(REMOTE_BUFF_SIZE-ir)], (REMOTE_BUFF_SIZE-ir+iw)-(LOCAL_BUFF_SIZE-ow));

        }else if((LOCAL_BUFF_SIZE-ow)<(REMOTE_BUFF_SIZE-ir)){
            /*
             * in      |+++W     R---|
             * local   |           W |
             * res     |-+++W      --|
             */
            memcpy(&local_buff.buff[ow], &in_buff[ir], (LOCAL_BUFF_SIZE-ow));
            memcpy(&local_buff.buff[0], &in_buff[ir+(LOCAL_BUFF_SIZE-ow)], (REMOTE_BUFF_SIZE-ir)-(LOCAL_BUFF_SIZE-ow));
            memcpy(&local_buff.buff[(REMOTE_BUFF_SIZE-ir)-(LOCAL_BUFF_SIZE-ow)], &in_buff[0], iw);

        }
        in_buff_info_ptr->head_index_offset = in_buff_info.tail_index_offset;
        local_buff.w = (local_buff.w+(REMOTE_BUFF_SIZE-ir+iw))%LOCAL_BUFF_SIZE;
    }

}

void print_buff(void)
{
    while(local_buff.r != local_buff.w)
    {
        //printf("local_buff.r:%d local_buff.w:%d\r\n",local_buff.r,local_buff.w);
        temp_item_ptr = (DEBUG_INFO_t*)(&local_buff.buff[local_buff.r]);
        print_item(temp_item_ptr);
        local_buff.r = (local_buff.r+sizeof(DEBUG_INFO_t))%LOCAL_BUFF_SIZE;
    }
}

void print_item(DEBUG_INFO_t* info)
{
    char* str = (char*)(&info->string_name);
    int count = 0;
    unsigned int temp_u;
    int temp_d,temp_d_hist;
    unsigned int temp_x;
    double temp_f;
    long long temp_l;
    char s[128]={0};
    int write_flag = 0;
    //printf("print_item:%s\r\n",str);
    if(strncmp(str,"INFO ",6))
    {
       write_flag = 1;
    }
    while('\0' != *str)
    {
       if('%' != *str)
       {
            //putchar(str++);
           if(write_flag)   { fprintf(log_file,"%c",*str); }
           printf("%c",*str++);
           fflush(stdout);
           continue;
       }else{
            switch (*(++str))
            {
                    case 'u':
                    {
                        memcpy((char*)(&temp_u),info->debug_data+count,4);
                        if(write_flag)   { fprintf(log_file,"%u",temp_u); }
                        printf("%u",temp_u);
                        fflush(stdout);
                        str++;
                        count += 4;
                        break;
                    }
                    case 'd':
                    {
                        memcpy((char*)(&temp_d),info->debug_data+count,4);
                        if(write_flag)   { fprintf(log_file,"%d",temp_d); }
                        printf("%d",temp_d);
                        fflush(stdout);
                        if(temp_d-temp_d_hist!=1)
                        {
                            temp_d_hist = 0;
                        }
                        str++;
                        count += 4;
                        temp_d_hist = temp_d;
                        break;
                    }
                    case 'x':
                    {
                        memcpy((char*)(&temp_x),info->debug_data+count,4);
                        if(write_flag)    { fprintf(log_file,"%x",temp_x); }
                        printf("%x",temp_x);
                        fflush(stdout);
                        str++;
                        count += 4;
                        break;
                    }
                    case 'f':
                    {
                        memcpy((char*)(&temp_f),info->debug_data+count,8);
                        if(write_flag)    { fprintf(log_file,"%lf",temp_f); }
                        printf("%lf",temp_f);
                        fflush(stdout);
                        str++;
                        count += 8;
                        break;
                    }
                    case 'l':
                    {
                        memcpy((char*)(&temp_l),info->debug_data+count,8);
                        if(write_flag)    { fprintf(log_file,"%lld",temp_l); }
                        printf("%lld",temp_l);
                        fflush(stdout);
                        str++;
                        count += 8;
                        break;
                    }
                    case 's':
                    {
                        memcpy(s,info->debug_data+count,strlen(info->debug_data+count));
                        if(write_flag)    { fprintf(log_file,"%s",s); }
                        printf("%s",s);
                        fflush(stdout);
                        str++;
                        count += (strlen(info->debug_data+count)+1);
                        break;
                    }
                    default:break;
                }
            }
    }
    //fflush(stdout);
    fflush(log_file);
}
void close_log_file()
{
    fflush(log_file);
    fclose(log_file);
}
