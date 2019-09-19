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
	FCTL2 = FWKEY + FSSEL_1 + FN4 + FN2;// flashʱ������ΪMCLK/21
//	__delay_cycles(1000);
}

void Falsh_Erase(unsigned char *Addr)   //�������
{
	int i = 0;
    while((FCTL3 & BUSY) == BUSY)	    //���Flash�Ƿ��ڲ���
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
    *Addr = 0;                            //���д��һ��ֵ�����������A�Ĳ����Ƚ��ر�һ�㲻�Զ�A���в���,��Ϊ��A�а���һЩУ������
    FCTL1 = FWKEY;
    FCTL3 = FWKEY + LOCK;
    while((FCTL3 & BUSY) == BUSY)         //���Flash�Ƿ��ڲ��������Բ����
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
//������д�����ݵ�Ƭ��FLASH��������ȫ��������д��
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

//��ָ����ַдһ������
void Falsh_Write_Arr(unsigned char *Addr, unsigned char nValue[], unsigned char n)//��ָ����ַ����дһ������,n�����ֽ���,���ܳ���64
{
	unsigned char i;
	FCTL1 = FWKEY + WRT;                          // ����д
    FCTL3 = FWKEY;                                // ����
    while((FCTL3 & BUSY) == BUSY);                //���Flash�Ƿ��ڲ���������д��ʱ��ü��
    for( i = 0; i < n; i ++)
    {
    	*Addr ++ = nValue[i];
    }
    FCTL1 = FWKEY;                                //���WRT������д
    FCTL3 = FWKEY + LOCK;                         // ����
    while((FCTL3 & BUSY) == BUSY);
}

void Write_CAP_Info(void)
{   // д�뵱ǰ����ֵ������SOCs����ʾ�����ٷֱȣ���CAPu����������ֵ����д����Ϣ��B
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
{   // д��ѭ��������ѭ����������ֵ���Լ��ŵ���±�����־λ����س䱥��־λ������Ƿѹ��־λ��д����Ϣ��C
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
