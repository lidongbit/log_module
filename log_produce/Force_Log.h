#ifndef __FORCE_LOG_H__
#define __FORCE_LOG_H__
 
#include "stdio.h"
#include "string.h"
 
#define USEA_MAIN_DEBUG		
 
#ifdef USEA_MAIN_DEBUG
#define Log_printf(format, ...)     Force_Print( format"\r\n", ##__VA_ARGS__)
#define Log_info(format, ...) 	    Force_Print( "[%s Line:%d] INFO: "format"\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define Log_debug(format, ...) 	    Force_Print( "[%s Line:%d] DEBUG: "format"\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define Log_error(format, ...) 	    Force_Print( "[%s Line:%d] ERROR: "format"\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define Log_warn(format, ...) 	    Force_Print( "[%s Line:%d] WARN:  "format"\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define Log_verbose(format, ...)    Force_Print( "[%s Line:%d] VERBOSE: "format"\r\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define Log_write(format,arg1,arg2,arg3,arg4,arg5,arg6) Force_Log_Write("[%s Line:%d] LOG: "format"\r\n", __FILE__, __LINE__,,arg1,arg2,arg3,arg4,arg5,arg6)
 
#else
 
#define Log_printf(format, ...)
#define Log_info(format, ...)
#define Log_debug(format, ...)
#define Log_error(format, ...)
#define Log_warn(format, ...)
#define Log_verbose(format, ...)
 
#endif
 
 
typedef enum
{
	PUSH_CIRCLE_BUFF_OVER_LIMIT=0x55,

}FORCE_DEBUG_ERRCODE_ENUM;

typedef enum
{
	UINT8_T,
	UINT16_T,
	UINT32_T,
	INT32_T,
	INT64_T,
	FLOAT_32,
	FLOAT_64,

}FORCE_DEBUG_DATATYPE_ENUM;

typedef struct
{
    char string_name[128];
    char debug_data[128];

}FORCE_DEBUG_INFO_t;

extern void Force_Debug_Init(void* ctr_ptr,void* msg_ptr);

extern void Force_Print(const char* str,...);

extern void Force_Debug_LedBlink(void);

extern void Force_Debug_LedPs_Blink(void);


#endif
