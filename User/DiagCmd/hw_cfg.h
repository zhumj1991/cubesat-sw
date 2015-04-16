#ifndef __HW_CFG_H_
#define __HW_CFG_H_

int serial_putc (int ch);
int serial_getc (void);
void serial_puts (const char *s);
int serial_tstc (void);

#endif	/* __HW_CFG_H_ */
