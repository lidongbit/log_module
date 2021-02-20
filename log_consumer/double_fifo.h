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
typedef struct
{
        int32_t tail_index_offset;
        int32_t head_index_offset;
        int32_t element_length;
        int32_t safety_resevd;
        int32_t semaphore;//初始化需要置位1
        int32_t write_lock;

}DEBUG_RINGS_BUFF_STRUCT;

extern void buff_init(void *ctr, void *msg);
extern void copy_buff(void);
extern void print_buff(void);
extern void print_item(DEBUG_INFO_t* info);

#endif
