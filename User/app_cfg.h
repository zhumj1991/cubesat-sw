/*
*********************************************************************************************************
*	                                  
*	模块名称 : 应用配置函数
*	文件名称 : app_cfg.c
*	版    本 : V1.0
*	说    明 : 主要用于任务堆栈和优先级的配置，以及uC/SERIAL配置。
*	修改记录 :
*		版本号    日期          作者              说明
*		v1.0    2013-03-26    Eric2013      ST固件库版本 V1.0.2版本
*   v2.0    2014-02-23    Eric2013      ST固件库V1.3.0版本
*
*********************************************************************************************************
*/

#ifndef  APP_CFG_MODULE_PRESENT
#define  APP_CFG_MODULE_PRESENT

/*
*********************************************************************************************************
*                                       MODULE ENABLE / DISABLE
*********************************************************************************************************
*/

#define  APP_CFG_SERIAL_EN                            DEF_DISABLED

/*
*********************************************************************************************************
*                                            TASK PRIORITIES
*********************************************************************************************************
*/

#define TASK_START_PRIO																2u
#define TASK_SAMPLE_PRIO															3u
#define TASK_TELEDATA_PRIO														4u
#define TASK_GPS_RECV_PRIO														5u
#define TASK_PM_PRIO																	6u
#define TASK_EDAC_PRIO																(OS_CFG_PRIO_MAX - 5u)
#define TASK_TEST_PRIO																(OS_CFG_PRIO_MAX - 4u)

/*
*********************************************************************************************************
*                                            TASK STACK SIZES
*                             Size of the task stacks (# of OS_STK entries)
*********************************************************************************************************
*/

#define TASK_START_STK_SIZE														128u
#define TASK_SAMPLE_STK_SIZE													128u
#define TASK_TELEDATA_STK_SIZE												128u
#define TASK_GPS_RECV_STK_SIZE												128u
#define TASK_PM_STK_SIZE															128u
#define TASK_EDAC_STK_SIZE														128u
#define TASK_TEST_STK_SIZE														1024u


/*
*********************************************************************************************************
*                                                 uC/SERIAL
*
* Note(s) : (1) Configure SERIAL_CFG_MAX_NBR_IF to the number of interfaces (i.e., UARTs) that will be
*               present.
*
*           (2) Configure SERIAL_CFG_RD_BUF_EN to enable/disable read buffer functionality.  The serial
*               core stores received data in the read buffer until the user requests it, providing a
*               reliable guarantee against receive overrun.
*
*           (3) Configure SERIAL_CFG_WR_BUF_EN to enable/disable write buffer functionality.  The serial
*               core stores line driver transmit data in the write buffer while the serial interface is
*               transmitting application data.
*
*           (4) Configure SERIAL_CFG_ARG_CHK_EXT_EN to enable/disable extended argument checking
*               functionality.
*
*           (5) Configure SERIAL_CFG_TX_DESC_NBR to allow multiple transmit operations (i.e., Serial_Wr,
*               Serial_WrAsync) to be queued.
*********************************************************************************************************
*/

#define  SERIAL_CFG_MAX_NBR_IF                          2u              /* See Note #1.                                         */

#define  SERIAL_CFG_RD_BUF_EN                           DEF_ENABLED     /* See Note #2.                                         */

#define  SERIAL_CFG_WR_BUF_EN                           DEF_ENABLED     /* See Note #3.                                         */

#define  SERIAL_CFG_ARG_CHK_EXT_EN                      DEF_DISABLED    /* See Note #4.                                         */

#define  SERIAL_CFG_TX_DESC_NBR                         1               /* See Note #5.                                         */

/*
*********************************************************************************************************
*                                    uC/SERIAL APPLICATION CONFIGURATION
*********************************************************************************************************
*/

#define  APP_SERIAL_CFG_TRACE_EN                        DEF_ENABLED
#define  APP_SERIAL_CFG_TRACE_PORT_NAME                 "UART1"


/*
*********************************************************************************************************
*                                     TRACE / DEBUG CONFIGURATION
*********************************************************************************************************
*/
#if 0
#define  TRACE_LEVEL_OFF                                0
#define  TRACE_LEVEL_INFO                               1
#define  TRACE_LEVEL_DBG                                2
#endif

#include <cpu.h>
void  DEBUG(CPU_CHAR *format, ...);

#define  APP_TRACE_LEVEL                                TRACE_LEVEL_DBG
#define  APP_TRACE                                      DEBUG

#define  APP_TRACE_INFO(x)               ((APP_TRACE_LEVEL >= TRACE_LEVEL_INFO)  ? (void)(APP_TRACE x) : (void)0)
#define  APP_TRACE_DBG(x)                ((APP_TRACE_LEVEL >= TRACE_LEVEL_DBG)   ? (void)(APP_TRACE x) : (void)0)

#endif

