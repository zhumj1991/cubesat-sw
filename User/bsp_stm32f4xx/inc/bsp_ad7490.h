#ifndef _AD7490_H_
#define _AD7490_H_


void bsp_ad7490_init(void);

void ad7490_reset(void);
uint16_t ad7490_normal_convert(uint8_t channel);
void ad7490_program_convert(uint16_t channel_en);
void ad7490_read(uint16_t *value);

#endif
