#include <diag_cmd.h>

#if (CFG_COMMANDS & CFG_OS)
/***************************************************************************
 * DIAG_CMD	: ps
 * Help			: print ucos usage
 **************************************************************************/
int 
do_ps (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	OS_TCB	*p_tcb;			/* 定义一个任务控制块指针, TCB = TASK CONTROL BLOCK */
	float CPU;

	CPU_SR_ALLOC();

	CPU_CRITICAL_ENTER();
	p_tcb = OSTaskDbgListPtr;
	CPU_CRITICAL_EXIT();
	
	/* 打印标题 */
	printf(" Prio   Used   Free   Per      CPU   Taskname\r\n");

	/* 遍历任务控制块列表(TCB list)，打印所有的任务的优先级和名称 */
	while (p_tcb != (OS_TCB *)0) 
	{
		CPU = (float)p_tcb->CPUUsage / 100;
		printf("   %2d  %5d  %5d   %02d%%   %5.2f%%   %s\r\n", 
		p_tcb->Prio, 
		p_tcb->StkUsed, 
		p_tcb->StkFree, 
		(p_tcb->StkUsed * 100) / (p_tcb->StkUsed + p_tcb->StkFree),
		CPU,
		p_tcb->NamePtr);		
	 	
		CPU_CRITICAL_ENTER();
		p_tcb = p_tcb->DbgNextPtr;
		CPU_CRITICAL_EXIT();
	}	
	
	return 0;
}

BOOT_CMD(
	ps, 1, 1, do_ps,
 	"ps      - print ucos usage\r\n",
	NULL
);

#endif
