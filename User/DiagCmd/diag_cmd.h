#ifndef __DIAG_CMD_H_
#define __DIAG_CMD_H_

/**************************************************************
* head files
**************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
 * diag_cmd相关
 */
#include "command.h"
#include "console.h"
#include "hw_cfg.h"
#include "date.h"
/*
 * ucos相关
 */
#include <os.h>
#include <bsp_os.h>
/*
 * hardware相关
 */
#include <bsp.h>


/**************************************************************
* define
**************************************************************/

/*
 * define to enable some functions
 */
#define DEBUG
//#define DEBUG_PARSER
//#define CONFIG_CMDLINE_EDITING


/*
 *
 */
#ifdef	DEBUG
	#define debug(fmt,args...)	printf (fmt ,##args)
	#define debugX(level,fmt,args...) if (DEBUG>=level) printf(fmt,##args);
#else
	#define debug(fmt,args...)
	#define debugX(level,fmt,args...)
#endif	/* DEBUG */

/*
#ifndef	__ASSEMBLY__
	#define __ASSEMBLY__
#endif
*/


/*
 *
 */
#define	CFG_CBSIZE			256			/* Console I/O Buffer Size */
#define	CFG_MAXARGS			16			/* Max number of command args	*/
#define CFG_PROMPT			">"

#define	EXTERN_SRAM_BASE	0x64000000
/* 
 * Enable(1)/Disable(0) command
 */
#define CFG_COMMANDS		1
#define	CFG_OS					1
#define CFG_CMD_EXIT		0
#define CFG_CMD_DATE		1
#define	CFG_CMD_LOAD		1


/*
 *
 */
typedef unsigned char		uchar;


/****************************************************************
 * function
 ***************************************************************/
int run_command (const char *cmd, int flag);

#endif
