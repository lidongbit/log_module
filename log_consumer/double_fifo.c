#include "double_fifo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include "buffer_manager.h"
typedef buffer_info_t DEBUG_RINGS_BUFF_STRUCT;
static char *in_buff;
static DEBUG_INFO_t* temp_item_ptr;
static DEBUG_RINGS_BUFF_STRUCT *in_buff_info_ptr;
static DEBUG_RINGS_BUFF_STRUCT in_buff_info;
#define LOCAL_BUFF_SIZE (1024*1024)
#define REMOTE_BUFF_SIZE (1024*1024)


static DEBUG_RINGS_BUFF_STRUCT local_buff;
static char buff[LOCAL_BUFF_SIZE];

void buff_init(void *ctr, void *msg)
{
    local_buff.head_index_offset=0;
    local_buff.tail_index_offset=0;
    local_buff.element_length=sizeof(DEBUG_INFO_t);
    local_buff.buff_length = LOCAL_BUFF_SIZE;
    local_buff.semaphore = 1;

    in_buff = (char *)(msg);
    in_buff_info_ptr = (DEBUG_RINGS_BUFF_STRUCT *)(ctr);
}

void copy_buff()
{
    pull_circle_buff_bundle(in_buff_info_ptr,in_buff,&local_buff,buff);

}

void print_buff(void)
{
    while(local_buff.head_index_offset != local_buff.tail_index_offset)
    {
        //printf("local_buff.r:%d local_buff.w:%d\r\n",local_buff.r,local_buff.w);
        temp_item_ptr = (DEBUG_INFO_t*)(&buff[local_buff.head_index_offset]);
        print_item(temp_item_ptr);
        local_buff.head_index_offset = (local_buff.head_index_offset+sizeof(DEBUG_INFO_t))%LOCAL_BUFF_SIZE;
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


