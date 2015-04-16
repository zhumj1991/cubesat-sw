#include <diag_cmd.h>

#if (CFG_COMMANDS & CFG_OS)
/***************************************************************************
 * DIAG_CMD	: ps
 * Help			: print ucos usage
 **************************************************************************/
int 
do_ps (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	OS_TCB	*p_tcb;			/* ����һ��������ƿ�ָ��, TCB = TASK CONTROL BLOCK */
	float CPU;

	CPU_SR_ALLOC();

	CPU_CRITICAL_ENTER();
	p_tcb = OSTaskDbgListPtr;
	CPU_CRITICAL_EXIT();
	
	/* ��ӡ���� */
	printf(" Prio   Used   Free   Per      CPU   Taskname\r\n");

	/* ����������ƿ��б�(TCB list)����ӡ���е���������ȼ������� */
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
