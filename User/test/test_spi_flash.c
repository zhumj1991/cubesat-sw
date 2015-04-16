#include <includes.h>

void spi_flash_test(void)
{
#if 1
	uint8_t buf[100] = {1, 2, 3, 4};
	uint8_t read_buf[1024];
	uint32_t write_addr = 0;
	uint32_t read_addr = 0;
	uint16_t size;
	
	bsp_spi_flash_init();
	
//	spi_flash_erase_sector(0);
	spi_flash_read_buffer(read_buf, read_addr, 1024);
//	spi_flash_write_buffer(buf, write_addr, sizeof(buf));
//	spi_flash_read_buffer(read_buf, read_addr, sizeof(read_buf));
#else
	
	FATFS fs;
	FIL file;
	UINT br, bw;
	char buf[20] = {0};
	FRESULT ret;
	
	ret = f_mount(0, &fs);
	
	ret = f_mkfs(0, 1, 4096);
	if (ret == FR_OK) {
		printf("format ok\r\n");
	} else {
		printf("format err: %d\r\n", ret);
	}
	
	ret = f_open(&file, "0:/1.txt", FA_READ | FA_WRITE | FA_CREATE_NEW);
	if (ret == FR_OK) {
		printf("creat file ok\r\n");
	} else {
		printf("creat file err: %d\r\n", ret);
	}
	f_write(&file, "hello", 5, &bw);
	
	f_close(&file);
	
	ret = f_open(&file, "0:/1.txt", FA_READ | FA_WRITE);
	if (ret == FR_OK) {
		printf("open file ok\r\n");
	} else {
		printf("open file err: %d\r\n", ret);
	}
	
	f_read(&file, buf, bw, &br);
	
	f_close(&file);
	f_mount(0, NULL);
#endif
}
