#include "diag_cmd.h"
#include "xyzModem.h"

uint32_t load_addr=EXTERN_SRAM_BASE;

#if (CFG_COMMANDS & CFG_CMD_LOAD)
/***************************************************************************
 * DIAG_CMD	: load(x/y/z...)
 * Help			: load binary file over serial line
 **************************************************************************/
int do_load_serial_bin (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{

}

BOOT_CMD(
	loady, 3, 0,	do_load_serial_bin,
	"loady   - load binary file over serial line (ymodem mode)\r\n",
	"    - load binary file over serial line\r\n"
);
#endif
