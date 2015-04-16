#include  <includes.h>



/* send to FPGA */
#define init_down()				GPIOC->BSRRL = GPIO_Pin_12
#define	cpu_heartbeat()		GPIOG->ODR ^= GPIO_Pin_8
/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/
                                                               
static  OS_TCB   TASK_START_TCB;
static  CPU_STK  task_start_stk[TASK_START_STK_SIZE];

static  OS_TCB   TASK_SAMPLE_TCB;
static  CPU_STK  task_sample_stk[TASK_SAMPLE_STK_SIZE];

static  OS_TCB   TASK_TELEDATA_TCB;
static  CPU_STK  task_teledata_stk[TASK_TELEDATA_STK_SIZE];

static  OS_TCB   TASK_GPS_RECV_TCB;
static  CPU_STK  task_gps_recv_stk[TASK_GPS_RECV_STK_SIZE];

static  OS_TCB   TASK_PM_TCB;
static  CPU_STK  task_pm_stk[TASK_PM_STK_SIZE];

static  OS_TCB   TASK_EDAC_TCB;
static  CPU_STK  task_edac_stk[TASK_EDAC_STK_SIZE];

static  OS_TCB   TASK_TEST_TCB;
static  CPU_STK  task_test_stk[TASK_TEST_STK_SIZE];


OS_SEM   SEM_SPI1_DEV_MUTEX;	   //用于互斥
static  OS_SEM   SEM_SYNCH;	   //用于同步

/* GPS */
OS_SEM   SEM_GPS;
#define	GPS_BUFFER_SIZE		256
char gps_buf[GPS_BUFFER_SIZE];


struct obc_info obc_status;
struct obc_tele obc_teledata;
/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static void task_start								(void     *p_arg);
static void task_sample								(void     *p_arg);
static void task_teledata        	 		(void     *p_arg);
static void task_gps_recv							(void     *p_arg);
static void task_pm										(void     *p_arg);
static void task_edac									(void     *p_arg);
static void task_test									(void     *p_arg);


/*
*********************************************************************************************************
*	函 数 名: main
*	功能说明: 标准c程序入口。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/

int main(void)
{
	OS_ERR  err;
	SCB->VTOR = 0x68000000;

//	BSP_IntDisAll();                                          /* Disable all interrupts.                              */   
	OSInit(&err);                                               /* Init uC/OS-III.                                      */

	/* 使能轮转调度 设置默认的全局时间片是8 */
	OSSchedRoundRobinCfg(DEF_ENABLED,
												8,
												&err);	

	OSTaskCreate((OS_TCB       *)&TASK_START_TCB,              /* Create the start task                                */
                 (CPU_CHAR     *)"task start",
                 (OS_TASK_PTR   )task_start, 
                 (void         *)0,
                 (OS_PRIO       )TASK_START_PRIO,
                 (CPU_STK      *)&task_start_stk[0],
                 (CPU_STK_SIZE  )TASK_START_STK_SIZE / 10,
                 (CPU_STK_SIZE  )TASK_START_STK_SIZE,
                 (OS_MSG_QTY    )0,
                 (OS_TICK       )0,
                 (void         *)0,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);

	OSStart(&err);                                              /* Start multitasking (i.e. give control to uC/OS-III). */
    
	(void)&err;
    
	return (0);
}

/*
*********************************************************************************************************
*	函 数 名: Apptask_start
*	功能说明: 这是一个启动任务，在多任务系统启动后，必须初始化滴答计数器(在BSP_Init中实现)
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
	优 先 级：2
*********************************************************************************************************
*/

static void task_start (void *p_arg)
{
	OS_ERR      err;
	(void)p_arg;

	/* 创建互斥信号量 */
	BSP_OS_SemCreate(&SEM_SPI1_DEV_MUTEX,
									1,	
									(CPU_CHAR *)"SEM_SPI_FLASH_MUTEX");
	/* 创建同步信号量 */ 
	BSP_OS_SemCreate(&SEM_SYNCH,
									0,	
									(CPU_CHAR *)"SEM_SYNCH");
	
 	BSP_Init();
	CPU_Init();
	BSP_Tick_Init();

//	Mem_Init();                             
//  Math_Init();                         

#if OS_CFG_STAT_TASK_EN > 0u
     OSStatTaskCPUUsageInit(&err);   
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif
	
	

	/**************创建数据采集任务*********************/
	OSTaskCreate((OS_TCB       *)&TASK_SAMPLE_TCB,             
                 (CPU_CHAR     *)"task sample",
                 (OS_TASK_PTR   )task_sample, 
                 (void         *)0,
                 (OS_PRIO       )TASK_SAMPLE_PRIO,
                 (CPU_STK      *)&task_sample_stk[0],
                 (CPU_STK_SIZE  )TASK_SAMPLE_STK_SIZE / 10,
                 (CPU_STK_SIZE  )TASK_SAMPLE_STK_SIZE,
                 (OS_MSG_QTY    )1,
                 (OS_TICK       )0,
                 (void         *)0,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);
	
	/**************创建数据存盘任务*********************/
	OSTaskCreate((OS_TCB       *)&TASK_TELEDATA_TCB,             
                 (CPU_CHAR     *)"task teledata",
                 (OS_TASK_PTR   )task_teledata, 
                 (void         *)0,
                 (OS_PRIO       )TASK_TELEDATA_PRIO,
                 (CPU_STK      *)&task_teledata_stk[0],
                 (CPU_STK_SIZE  )TASK_TELEDATA_STK_SIZE / 10,
                 (CPU_STK_SIZE  )TASK_TELEDATA_STK_SIZE,
                 (OS_MSG_QTY    )0,
                 (OS_TICK       )0,
                 (void         *)0,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);
	
	/**************创建GPS接收任务**********************/			 
	OSTaskCreate((OS_TCB       *)&TASK_GPS_RECV_TCB,              
                 (CPU_CHAR     *)"task gps recv",
                 (OS_TASK_PTR   )task_gps_recv, 
                 (void         *)0,
                 (OS_PRIO       )TASK_GPS_RECV_PRIO,
                 (CPU_STK      *)&task_gps_recv_stk[0],
                 (CPU_STK_SIZE  )TASK_GPS_RECV_STK_SIZE / 10,
                 (CPU_STK_SIZE  )TASK_GPS_RECV_STK_SIZE,
                 (OS_MSG_QTY    )0,
                 (OS_TICK       )8,
                 (void         *)0,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);
	
	/**************创建电源管理任务*********************/			 
	OSTaskCreate((OS_TCB       *)&TASK_PM_TCB,              
                 (CPU_CHAR     *)"task pm",
                 (OS_TASK_PTR   )task_pm, 
                 (void         *)0,
                 (OS_PRIO       )TASK_PM_PRIO,
                 (CPU_STK      *)&task_pm_stk[0],
                 (CPU_STK_SIZE  )TASK_PM_STK_SIZE / 10,
                 (CPU_STK_SIZE  )TASK_PM_STK_SIZE,
                 (OS_MSG_QTY    )0,
                 (OS_TICK       )8,
                 (void         *)0,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);
								 
	/**************创建EDAC检错纠错任务*********************/			 
	OSTaskCreate((OS_TCB       *)&TASK_EDAC_TCB,              
                 (CPU_CHAR     *)"task edac",
                 (OS_TASK_PTR   )task_edac, 
                 (void         *)0,
                 (OS_PRIO       )TASK_EDAC_PRIO,
                 (CPU_STK      *)&task_edac_stk[0],
                 (CPU_STK_SIZE  )TASK_EDAC_STK_SIZE / 10,
                 (CPU_STK_SIZE  )TASK_EDAC_STK_SIZE,
                 (OS_MSG_QTY    )0,
                 (OS_TICK       )8,
                 (void         *)0,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);

	/**************创建测试任务*********************/	
	OSTaskCreate((OS_TCB       *)&TASK_TEST_TCB,              
                 (CPU_CHAR     *)"task tset",
                 (OS_TASK_PTR   )task_test, 
                 (void         *)0,
                 (OS_PRIO       )TASK_TEST_PRIO,
                 (CPU_STK      *)&task_test_stk[0],
                 (CPU_STK_SIZE  )TASK_TEST_STK_SIZE / 10,
                 (CPU_STK_SIZE  )TASK_TEST_STK_SIZE,
                 (OS_MSG_QTY    )0,
                 (OS_TICK       )8,
                 (void         *)0,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);								 

		init_down();
								 
		printf("CPU_A is running\r\n");
								 
    while(1) {                                         
		cpu_heartbeat();
		iwdg_feed();
			
		BSP_OS_TimeDlyMs(500);
    }
}

/*
*********************************************************************************************************
*	函 数 名: task_sample
*	功能说明: 对传感器等数据进行采集
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
	优 先 级：
*********************************************************************************************************
*/
static void task_sample(void *p_arg)
{
	(void)p_arg;

	while(1) {
		
		
		BSP_OS_TimeDlyMs(1000);      											  
	}   
}

/*
*********************************************************************************************************
*	函 数 名: task_teledata
*	功能说明: 对遥测数据进程存盘
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
	优 先 级：
*********************************************************************************************************
*/

static void task_teledata(void *p_arg)
{
	OS_ERR  err;
	CPU_TS ts;
	(void)p_arg;	               /* 避免编译器报警 */
  
	while(1) {
		/* CPU status */
		OSSemPend(&SEM_SPI1_DEV_MUTEX, 0, OS_OPT_PEND_BLOCKING, &ts, &err);
		spi_flash_write_buffer((uint8_t *)&obc_status, CPU_STATUS_BASE, sizeof(obc_status));
		OSSemPost (&SEM_SPI1_DEV_MUTEX, OS_OPT_POST_1, &err);
		
    BSP_OS_TimeDlyMs(1000);	     
	}
}

/*
*********************************************************************************************************
*	函 数 名: task_gps_recv
*	功能说明: 接收GPS数据		  			  
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
* 优 先 级：
*********************************************************************************************************
*/

static void task_gps_recv(void *p_arg)
{
	nmea_msg gpsx;
	OS_ERR err;
	CPU_TS ts;
	(void)p_arg;		/* 避免编译器告警 */ 

	/* 创建GPS接收同步信号量 */ 
	OSSemCreate(&SEM_GPS, "SEM_GPS", 0, &err);
		
	while(1) {
		OSSemPend(&SEM_GPS, 0, OS_OPT_PEND_BLOCKING, &ts, &err);
		
		if (err == 0) {
			debug("GPS receive data: %s\r\n", gps_buf);
			debug("GPS receive timestamp: %d\r\n", ts);
			
			gps_analyze(gpsx, gps_buf);
		} else {
			printf("SEM_GPS err: %d\r\n", err);
		}
		
	}
}

/*
*********************************************************************************************************
*	函 数 名: task_pm
*	功能说明: 对卫星电源进行管理		  			  
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
* 优 先 级：
*********************************************************************************************************
*/
static void task_pm(void *p_arg)
{
	(void)p_arg;		/* 避免编译器告警 */
		
	while (1) {
		
		BSP_OS_TimeDlyMs(1000); 					 
	}
}
/*
*********************************************************************************************************
*	函 数 名: task_edac
*	功能说明: 对卫星星务程序进行纠错检错	  			  
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
* 优 先 级：(OS_CFG_PRIO_MAX - 5u)
*********************************************************************************************************
*/
static void task_edac(void *p_arg)
{
	extern uint8_t * __code_start;
	extern uint32_t __code_length;
	
	uint8_t * check_buf = (uint8_t *)SRAM_EDAC_ADDR;
	uint8_t * data_buf = __code_start;
	uint32_t check_len, len = __code_length;
	int8_t status;
	uint8_t error_num;
	
	(void)p_arg;		/* 避免编译器告警 */

	/* generate edac checksum in sram when first run this task */	
	memset(check_buf, 0, __code_length);
	edac_generate(check_buf, data_buf, len);
	
	while (1) {
		
		check_buf = (uint8_t *)SRAM_EDAC_ADDR;
		data_buf = (uint8_t *)__code_start;
		len = __code_length;
		error_num = EDAC_ERROR_TOLERANCE;	/* if there are more errors happennig, reset */
		
		do {
			/*
			 * check most EADC_CHECK_LEN bytes at one time
			 */
			check_len = (len > EADC_CHECK_LEN)? EADC_CHECK_LEN : len;
			
			status = edac_check(data_buf, check_buf, check_len);
	
			/*
			* error happens: more than one bit upset, can not correct
			 */
			if (status == -1) {
				obc_teledata.edac_errors = EDAC_ERROR_TOLERANCE - error_num;
				error_num--;
			}
	
			len -= check_len;
			check_buf += check_len;
			data_buf += check_len;
			
			BSP_OS_TimeDlyMs(1000);
		} while (len > 0);
		
		if (!error_num) {
		/*
		 * save task
		 * reset the cpu by setting up the watchdog timer and let him time out
		 */
			debug("edac reset...\r\n");
			system_reset();
		}
		
		//debug("The number of error tolerance left: %d\r\n", error_num);
		BSP_OS_TimeDlyMs(10000); 					 
	}
}


/***************************************************************************
*	函 数 名: task_test
*	功能说明: 软件测试		  			  
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
* 优 先 级：(OS_CFG_PRIO_MAX - 4u)
****************************************************************************/
static void task_test(void *p_arg)
{
#if 0
	uint32_t count;
	float x, y, z;
	uint32_t i;
	
	bsp_tim_count_init(0xFFFF);
	TIM_Cmd(TIM4, ENABLE);
	count = TIM_GetCounter(TIM4);
	printf("current timer count: %d\r\n", count);
	
	for(i = 0; i < 0xFFF; i++) {
		x = 1.238 * i;
		y = x / 3.2;
		z = log(y);
		z -= 1.2; 
	}
	
	count = TIM_GetCounter(TIM4);
	printf("current timer count: %d\r\n", count);
	TIM_Cmd(TIM4, DISABLE);
#endif	
	//spi_flash_test();
	diag();
	
	while (1) {

		
		BSP_OS_TimeDlyMs(1000); 					 
	}
}

