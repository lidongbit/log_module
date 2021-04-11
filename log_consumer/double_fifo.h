#ifndef __DOUBLEFIFIO_H__
#define __DOUBLEFIFIO_H__

#define LOG_MSG_CONTROL_ADDR 0x3E000000
#define LOG_MSG_ADDR         0x3E100000

typedef struct
{
    char string_name[128];
    char debug_data[128];

}DEBUG_INFO_t;

typedef int int32_t;

extern void buff_init(void *ctr, void *msg);
extern void copy_buff(void);
extern void print_buff(void);
extern void print_item(DEBUG_INFO_t* info);

#endif
