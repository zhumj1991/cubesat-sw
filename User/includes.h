/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                             (c) Copyright 2013; Micrium, Inc.; Weston, FL
*
*                   All rights reserved.  Protected by international copyright laws.
*                   Knowledge of the source code may not be used to write a similar
*                   product.  This file may only be used in accordance with a license
*                   and should not be redistributed in any way.
*********************************************************************************************************
*/
#ifndef  INCLUDES_PRESENT
#define  INCLUDES_PRESENT



/*
*********************************************************************************************************
*                                         STANDARD LIBRARIES
*********************************************************************************************************
*/

#include  <stdarg.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <math.h>


/*
*********************************************************************************************************
*                                              LIBRARIES
*********************************************************************************************************
*/

#include  <cpu.h>
#include  <lib_def.h>
#include  <lib_ascii.h>
#include  <lib_math.h>
#include  <lib_mem.h>
#include  <lib_str.h>


/*
*********************************************************************************************************
*                                                 OS
*********************************************************************************************************
*/

#include  <os.h>


/*
*********************************************************************************************************
*                                              APP / BSP
*********************************************************************************************************
*/
#include  "bsp.h"
#include  "app_cfg.h"
#include  "bsp_os.h"

//#include	"ff.h"
//#include	"diskio.h"


#define		SW_ON_ARMFLY					0
#define		CFG_UART_GPS					1
#define		CFG_DEBUG							1
/* EDAC */
#define		SRAM_EDAC_ADDR				0x68100000
#define		EADC_CHECK_LEN				1024
#define		EDAC_ERROR_TOLERANCE	5
/* SPI FLASH: Teledata */
#define		CPU_STATUS_BASE				0
#define		CPU_STATUS_SIZE				4096
#define		TELEDATA_BASE					(CPU_STATUS_BASE + CPU_STATUS_SIZE)



#if CFG_DEBUG
	#define debug(fmt,args...)		printf (fmt ,##args) 
#else
	#define debug(fmt,args...)
#endif

/*
 * task
 */
#include "edac.h"
#include "fipex.h"
#include "diag.h"


__packed struct obc_info {
	uint8_t flag;
	uint8_t power_on;
	struct rtc_time tm;
	uint32_t	p_tele;
};


__packed struct obc_tele {
	uint8_t clk_source;
	uint8_t edac_errors;
	uint8_t rst_pin;
	uint8_t rst_sft;
	uint8_t rst_iwdg;
	uint8_t rst_lpw;
	uint16_t voltage_1_2;
	uint16_t voltage_3_3;
	uint16_t voltage_5_0;
};

/*
 * test
 */
#include "test_spi_flash.h" 

 
/*
*********************************************************************************************************
*                                               SERIAL
*********************************************************************************************************
*/

#if (APP_CFG_SERIAL_EN == DEF_ENABLED)
#include  <app_serial.h>
#endif


/*
*********************************************************************************************************
*                                            INCLUDES END
*********************************************************************************************************
*/


#endif

