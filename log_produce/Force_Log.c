#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include "Force_Log.h"
#include <unistd.h>

#define MESAGE_BUFF_LENGTH 1*1024*1024
//static volatile unsigned int* led_con_ps = (volatile unsigned int*)(0xe000a000);

static char *mesg_buff;

static FORCE_DEBUG_INFO_t temp_var_info;
static FORCE_DEBUG_RINGS_BUFF_STRUCT *circle_buff_info;
/*--------------------------------------------------------------------
 * 函数名称:void Force_Debug_Init(void)
 * 函数输入:
 * 函数输出:
 * 函数备注:初始化
 * ------------------------------------------------------------------*/
void Force_Debug_Init(void *ctr_ptr,void* msg_ptr)
{
	//circle_buff_info = (FORCE_DEBUG_RINGS_BUFF_STRUCT *)(LOG_MSG_CONTROL_ADDR);
	//mesg_buff = (char *)(LOG_MSG_ADDR);

	circle_buff_info = (FORCE_DEBUG_RINGS_BUFF_STRUCT *)(ctr_ptr);
	mesg_buff = (char *)(msg_ptr);
    
	circle_buff_info->semaphore = 1;
	circle_buff_info->tail_index_offset = 0;
	circle_buff_info->head_index_offset
						= circle_buff_info->tail_index_offset;
	circle_buff_info->element_length = sizeof(FORCE_DEBUG_INFO_t);
	circle_buff_info->safety_resevd = 2*circle_buff_info->element_length;
	circle_buff_info->write_lock = 0;

}
/*--------------------------------------------------------------------
 * 函数名称:int32_t Force_Push_CircleBuff(void *push_ptr,int32_t length)
 * 函数输入:
 * 函数输出:
 * 函数备注:初始化
 * ------------------------------------------------------------------*/
int32_t Force_Push_CircleBuff(void *push_ptr,int32_t length)
{
	int32_t delta_offset = 0;

	int32_t ret_val = 0;

	delta_offset = circle_buff_info->tail_index_offset
							-circle_buff_info->head_index_offset;

		if(delta_offset>=0)
		{
			if((circle_buff_info->tail_index_offset+length)>MESAGE_BUFF_LENGTH)
			{
				memcpy(mesg_buff+circle_buff_info->tail_index_offset,push_ptr
						,(MESAGE_BUFF_LENGTH-circle_buff_info->tail_index_offset));

				memcpy(mesg_buff,push_ptr+(circle_buff_info->tail_index_offset+length-MESAGE_BUFF_LENGTH)
						,length-(MESAGE_BUFF_LENGTH-circle_buff_info->tail_index_offset));

				circle_buff_info->tail_index_offset =
							length-(MESAGE_BUFF_LENGTH-circle_buff_info->tail_index_offset);
			}else{
				memcpy(mesg_buff+circle_buff_info->tail_index_offset,push_ptr,length);
				circle_buff_info->tail_index_offset += circle_buff_info->element_length;
			}

		}else{
				memcpy(mesg_buff+circle_buff_info->tail_index_offset,push_ptr,length);
				circle_buff_info->tail_index_offset += circle_buff_info->element_length;
		}

	return ret_val;
}
/*--------------------------------------------------------------------
 * 函数名称:void Force_Print(const char* str,...)
 * 函数输入:
 * 函数输出:
 * 函数备注:打印数据
 * ------------------------------------------------------------------*/
void Force_Print(const char* str,...)
{
	int32_t count = 0;

	uint32_t temp_u = 0;
	int32_t temp_s = 0;
	int64_t temp_64 = 0;
    char* temp_char = NULL;
	//char *temp_char = NULL;
    double temp_f = 0.0;
	memset(temp_var_info.string_name,0,128);

	if((strlen(str))>128)
	{
		memcpy(temp_var_info.string_name,str,128);
	}else{
		memcpy(temp_var_info.string_name,str,strlen(str));
	}

	va_list args;
	va_start(args, str);

	while('\0' != *str)
	{
	   if('%' != *str)
	   {
	        str++;
	        continue;
	   }else{
	        switch (*(++str))
	        {
	                case 'u':
	                {
	                	temp_u = va_arg(args, unsigned int);
	                	memcpy(temp_var_info.debug_data+count,(char*)&temp_u,4);
	                    str++;
	                    count += 4;
	                    break;
	                }
	                case 'd':
	                {
	                	temp_s = va_arg(args,int);
	                	memcpy(temp_var_info.debug_data+count,(char*)&temp_s,4);
	                    str++;
	                    count += 4;
	                    break;
	                }
	                case 'x':
	                {
	                	temp_u = va_arg(args, unsigned int);
	                	memcpy(temp_var_info.debug_data+count,(char*)&temp_u,4);
	                    str++;
	                	count += 4;
	                	break;
	                }
	                case 'f':
	                {
	                    temp_f = va_arg(args,double);
                        memcpy(temp_var_info.debug_data+count,(char*)&temp_f,8);
	                    str++;
	                    count += 8;
	                    break;
	                }
	                case 'l':
	                {
	                	temp_64 = va_arg(args,int64_t);
	                	memcpy(temp_var_info.debug_data+count,(char*)&temp_64,8);
	                	str++;
	                    count += 8;
	                    break;
	                }
	                case 's':
	                {
                        temp_char = va_arg(args,char*);
                        memcpy(temp_var_info.debug_data+count,temp_char,strlen(temp_char)+1);
	                	str++;
                        count += (strlen(temp_char)+1);
	                	break;
	                }
	                default:break;
	            }
	     	}
	}
	va_end(args);
	Force_Push_CircleBuff((char*)&temp_var_info,circle_buff_info->element_length);
}


