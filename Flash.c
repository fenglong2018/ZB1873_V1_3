/*
 * Flash.c
 *
 *  Created on: 2015-6-4
 *      Author: Administrator
 */

#include "Config.H"
#include "Flash.H"
#include "Protect.h"

uint8_t info_write_cnt = 0;

void Flash_init(void)
{
	FCTL2 = FWKEY + FSSEL_1 + FN4 + FN2;// flash时钟设置为MCLK/21
//	__delay_cycles(1000);
}

void Falsh_Erase(unsigned char *Addr)   //清除函数
{
	int i = 0;
    while((FCTL3 & BUSY) == BUSY)	    //检测Flash是否在操作
    {
    	i ++;
    	if(i > 65530)
    	{
    		return;
    	}
    }
    i = 0;
    FCTL1 = FWKEY+ERASE;
    FCTL3 = FWKEY;
    *Addr = 0;                            //随便写入一个值进行清除，段A的操作比较特别，一般不对段A进行操作,因为段A中包含一些校验数据
    FCTL1 = FWKEY;
    FCTL3 = FWKEY + LOCK;
    while((FCTL3 & BUSY) == BUSY)         //检测Flash是否在操作，可以不检测
    {
    	i ++;
    	if(i > 35530)
    	{
    		return;
    	}
    }
}

/*---------------------------------------------------------------------------------------------*/
char Flash_Read_Byte(char *start_addr, char index)   //read one byte
{
	  char *flash_ptr = ((char *) start_addr) + index;
	  return *flash_ptr;
}

unsigned int Flash_Read_2Byte(unsigned int *start_addr, unsigned int index)    //read two byte
{
    unsigned int *flash_ptr = ((unsigned int *) start_addr) + index;
      return *flash_ptr;
}
// *****************************************************
//把数组写入数据到片内FLASH，必须先全部擦除再写入
// *****************************************************
void WriteSegmentC(char addr, int valve)
{
    int *flash_ptr = (int *)SegmentCStart;

    if(g_sys_state.val.I2C_error_flag == FALSE)
    {
        FCTL3 = FWKEY;                            // Clear Lock bit
        FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation
        *(flash_ptr + addr) = valve;
        FCTL1 = FWKEY;                            // Clear WRT bit
        FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
        while((FCTL3 & BUSY) == BUSY);
    }
}

void WriteSegmentD(char addr, int valve)
{
    int *flash_ptr = (int *)SegmentDStart;

    if(g_sys_state.val.I2C_error_flag == FALSE)
    {
        FCTL3 = FWKEY;                            // Clear Lock bit
        FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation
        *(flash_ptr + addr) = valve;
        FCTL1 = FWKEY;                            // Clear WRT bit
        FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
        while((FCTL3 & BUSY) == BUSY);
    }
}

void WriteSegmentB(char addr, int valve)
{
    int *flash_ptr = (int *)SegmentBStart;

    if(g_sys_state.val.I2C_error_flag == FALSE)
    {
        FCTL3 = FWKEY;                            // Clear Lock bit
        FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation
        *(flash_ptr + addr) = valve;
        FCTL1 = FWKEY;                            // Clear WRT bit
        FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
        while((FCTL3 & BUSY) == BUSY);
    }
}

//向指定地址写一个数组
void Falsh_Write_Arr(unsigned char *Addr, unsigned char nValue[], unsigned char n)//向指定地址连续写一个数组,n代表字节数,不能超过64
{
	unsigned char i;
	FCTL1 = FWKEY + WRT;                          // 允许写
    FCTL3 = FWKEY;                                // 解锁
    while((FCTL3 & BUSY) == BUSY);                //检测Flash是否在操作，连续写入时最好检测
    for( i = 0; i < n; i ++)
    {
    	*Addr ++ = nValue[i];
    }
    FCTL1 = FWKEY;                                //清除WRT，不能写
    FCTL3 = FWKEY + LOCK;                         // 锁定
    while((FCTL3 & BUSY) == BUSY);
}

void Write_CAP_Info(void)
{   // 写入当前容量值，包含SOCs（显示容量百分比）和CAPu（已用容量值），写入信息区B
    uint16_t test_flash = 0;
    uint16_t temp_flash_soc = 0;

    if(cap_write_cnt >= 32)
    {
        Falsh_Erase((unsigned char *)SegmentBStart);
        cap_write_cnt = 0;
    }

    test_flash = -g_sys_cap.val.cap_val;
    temp_flash_soc = (uint16_t)g_sys_cap.val.show_cap_rate * 640 + (test_flash >> 2);
    WriteSegmentB(cap_write_cnt, temp_flash_soc);
    test_flash = Flash_Read_2Byte((unsigned int *)SegmentBStart, cap_write_cnt);
    if(test_flash != temp_flash_soc)
    {
        Falsh_Erase((unsigned char *)SegmentBStart);
        cap_write_cnt = 0;
    }
    cap_write_cnt ++;
}

void Write_Other_Info(void)
{   // 写入循环次数和循环次数容量值，以及放电高温保护标志位，电池充饱标志位，过放欠压标志位，写入信息区C
    uint16_t test_flash = 0;
    uint16_t cycle_cap_temp = 0;

    if(info_write_cnt >= 16)
    {
        Falsh_Erase((unsigned char *)SegmentCStart);
        info_write_cnt = 0;
    }

    cycle_cap_temp = g_sys_cap.val.cycle_cap_val;
    if(g_sys_flags.val.dch_temp_high_protect_flag == TRUE)
    {
        cycle_cap_temp |= 0x8000;
    }
    if(g_sys_state.val.vbat_chg_full_flag == TRUE)
    {
        cycle_cap_temp |= 0x4000;
    }
//    if(g_sys_flags.val.soft_vcell_low_protect_flag == TRUE)
//    {
//        cycle_cap_temp |= 0x2000;
//    }

    WriteSegmentC(info_write_cnt*2, g_sys_cap.val.bat_cycle_cnt);
    WriteSegmentC(info_write_cnt*2 + 1, cycle_cap_temp);
    test_flash = Flash_Read_2Byte((unsigned int *)SegmentCStart, info_write_cnt*2);
    if(test_flash != g_sys_cap.val.bat_cycle_cnt)
    {
        Falsh_Erase((unsigned char *)SegmentCStart);
        info_write_cnt = 0;
    }
    else
    {
        test_flash = Flash_Read_2Byte((unsigned int *)SegmentCStart, info_write_cnt*2 + 1);
        if(test_flash != cycle_cap_temp)
        {
            Falsh_Erase((unsigned char *)SegmentCStart);
            info_write_cnt = 0;
        }
    }
    info_write_cnt ++;
}

void Write_All_Info(void)
{
    Write_CAP_Info();
    Write_Other_Info();
}


//end
