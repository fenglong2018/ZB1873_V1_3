/*
 * Flash.H
 *  Created on: 2013-3-26
 *      Author: bear
 *
 *      电流校准系数 4字节
 *      保护参数 12字节
 *      标志位 1
 */

#ifndef FLASH_H_
#define FLASH_H_
#define SegmentDStart  0x1000  //0x01000~0x010FF
#define SegmentCStart  0x1040  //0x01000~0x010FF
#define SegmentBStart  0x1080  //0x01000~0x010FF
#define SysSNStart     0xC000  //store the system serial number   序列号 4字节
#define SegmentSize    255
#define PARM_DATA_LEN  28
#define WRITE_FLAG_ADDR 0
#define PARM_ADDR 1

extern uint8_t info_write_cnt;

void Flash_init(void);
void Falsh_Erase(unsigned char *Addr);
char Flash_Read_Byte(char *start_addr, char index);

unsigned int Flash_Read_2Byte(unsigned int *start_addr, unsigned int index) ;

extern void Write_CAP_Info(void);
extern void Write_Other_Info(void);
extern void Write_All_Info(void);

void WriteSegmentC(char addr, int valve);
void WriteSegmentD(char addr, int valve);
void WriteSegmentB(char addr, int valve);
void Falsh_Write_Arr(unsigned char *Addr, unsigned char nValue[], unsigned char n);

#endif /* FLASH_H_ */
