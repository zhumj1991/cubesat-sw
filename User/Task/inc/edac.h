#ifndef __EDAC_H
#define	__EDAC_H

#include "stm32f4xx.h"


typedef enum {
	NO_ERROR       = 0,
	DATA_ERROR     = 1,
	CHECK_ERROR    = 2,
	ERRORS         = 3
}EdacStatus;

void edac_byte2ham(uint8_t *check, uint8_t *data);
EdacStatus edac_ham2byte(uint8_t *data, uint8_t *check);
int8_t edac_generate(uint8_t *check_buf, uint8_t *buf, uint32_t size);
int8_t edac_check(uint8_t *buf, uint8_t *check_buf, uint32_t size);

#endif /* __EDAC_H */
