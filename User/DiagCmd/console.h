#ifndef _CONSOLE_H_
#define _CONSOLE_H_


int tstc (void);
int ctrlc (void);
int had_ctrlc (void);
void clear_ctrlc (void);

int readline (const char *const prompt);

#endif
