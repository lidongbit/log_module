#include "double_fifo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

static char *in_buff;
static DEBUG_INFO_t* temp_item_ptr;
static DEBUG_RINGS_BUFF_STRUCT *in_buff_info_ptr;
static DEBUG_RINGS_BUFF_STRUCT in_buff_info;
#define LOCAL_BUFF_SIZE (1024*1024)
#define REMOTE_BUFF_SIZE (1024*1024)

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
        if((LOCAL_BUFF_SIZE-ow)>(REMOTE_BUFF_SIZE-ir+iw))
        {
            /*
             * in      |+++W     R---|
             * local   |      W      |
             * res     |      ---+++W|
             */
            memcpy(&local_buff.buff[ow], &in_buff[ir], (REMOTE_BUFF_SIZE-ir));
            memcpy(&local_buff.buff[ow+(REMOTE_BUFF_SIZE-ir)], &in_buff[0], iw);

        }else if((LOCAL_BUFF_SIZE-ow)>(REMOTE_BUFF_SIZE-ir)){
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
    int temp_d;
    unsigned int temp_x;
    double temp_f;
    long long temp_l;
    char s[128]={0};
    //printf("print_item:%s\r\n",str);
    while('\0' != *str)
    {
       if('%' != *str)
       {
            //putchar(str++);
           printf("%c",*str++);
           fflush(stdout);
           continue;
       }else{
            switch (*(++str))
            {
                    case 'u':
                    {
                        memcpy((char*)(&temp_u),info->debug_data+count,4);
                        printf("%u",temp_u);
                        fflush(stdout);
                        str++;
                        count += 4;
                        break;
                    }
                    case 'd':
                    {
                        memcpy((char*)(&temp_d),info->debug_data+count,4);
                        printf("%d",temp_d);
                        fflush(stdout);
                        str++;
                        count += 4;
                        break;
                    }
                    case 'x':
                    {
                        memcpy((char*)(&temp_x),info->debug_data+count,4);
                        printf("%x",temp_x);
                        fflush(stdout);
                        str++;
                        count += 4;
                        break;
                    }
                    case 'f':
                    {
                        memcpy((char*)(&temp_f),info->debug_data+count,8);
                        printf("%lf",temp_f);
                        fflush(stdout);
                        str++;
                        count += 8;
                        break;
                    }
                    case 'l':
                    {
                        memcpy((char*)(&temp_l),info->debug_data+count,8);
                        printf("%l",temp_l);
                        fflush(stdout);
                        str++;
                        count += 8;
                        break;
                    }
                    case 's':
                    {
                        memcpy(s,info->debug_data+count,strlen(info->debug_data+count));
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

}


