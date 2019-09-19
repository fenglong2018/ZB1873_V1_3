/*
 * BQ76920.c
 *
 *  Created on: 2014-10-28
 *      Author: xxy
 */
#include "BQ76920.H"

RegisterGroup Registers;
float Gain = 0;
int iGain = 0;


void ReConfigBq(void)
{
    I2CClear();
    I2CInitialise();   //重新初始化
    InitialisebqMaximo();
}

int GetADCGainOffset()
{
    int result = -1;
    uint8_t i = 0;

    for(i = 0; i < 5; i ++)
    {
        if(result == -1)
        {
            result = I2CReadRegisterByteWithCRC(BQMAXIMO, ADCGAIN1, &(Registers.ADCGain1.ADCGain1Byte));
            if(result == -1){continue;}

            result = I2CReadRegisterByteWithCRC(BQMAXIMO, ADCGAIN2, &(Registers.ADCGain2.ADCGain2Byte));
            if(result == -1){continue;}

            result = I2CReadRegisterByteWithCRC(BQMAXIMO, ADCOFFSET, &(Registers.ADCOffset));
            if(result == -1){continue;}
        }
        else
        {
            break;
        }
    }
    return result;
}

int ConfigureBqMaximo()
{
    int result = 0;
    uint8_t bqMaximoProtectionConfig[6];

    I2CWriteBlockWithCRC(BQMAXIMO, PROTECT1, &(Registers.Protect1.Protect1Byte), 6);

    I2CReadBlockWithCRC(BQMAXIMO, PROTECT1, bqMaximoProtectionConfig, 6);

    if((bqMaximoProtectionConfig[0] != Registers.Protect1.Protect1Byte)
    || (bqMaximoProtectionConfig[1] != Registers.Protect2.Protect2Byte)
    || (bqMaximoProtectionConfig[2] != Registers.Protect3.Protect3Byte)
    || (bqMaximoProtectionConfig[3] != Registers.OVTrip)
    || (bqMaximoProtectionConfig[4] != Registers.UVTrip)
    || (bqMaximoProtectionConfig[5] != Registers.CCCfg)
    )
    {
        result = -1;
    }
    I2CWriteBlockWithCRC(BQMAXIMO, SYS_CTRL1, &(Registers.SysCtrl1.SysCtrl1Byte), 2);
    I2CReadBlockWithCRC(BQMAXIMO, SYS_CTRL1, bqMaximoProtectionConfig, 2);

    if((bqMaximoProtectionConfig[0] != Registers.SysCtrl1.SysCtrl1Byte)
    || (bqMaximoProtectionConfig[1] != Registers.SysCtrl2.SysCtrl2Byte))
    {
        result = -1;
    }

    return result;

}

int InitialisebqMaximo()
{
    int result = 0;
    uint8_t i = 0;
    I2CReadBlockWithCRC(BQMAXIMO, SYS_CTRL1, &(Registers.SysCtrl1.SysCtrl1Byte), 2);

    Registers.Protect1.Protect1Bit.RSNS = USER_RSNS;
    Registers.Protect1.Protect1Bit.SCD_DELAY = SCDDelay;
    Registers.Protect1.Protect1Bit.SCD_THRESH = SCDThresh;
    Registers.Protect1.Protect1Bit.RSVD = 0;
    Registers.Protect2.Protect2Bit.OCD_DELAY = OCDDelay;
    Registers.Protect2.Protect2Bit.OCD_THRESH = OCDThresh;
    Registers.Protect2.Protect2Bit.RSVD = 0;
    Registers.Protect3.Protect3Bit.OV_DELAY = OVDelay;
    Registers.Protect3.Protect3Bit.UV_DELAY = UVDelay;
    Registers.Protect3.Protect3Bit.RSVD = 0;
    Registers.SysCtrl1.SysCtrl1Byte=0x18;			//ADC_EN  TEMP_S  18
    Registers.SysCtrl2.SysCtrl2Byte|=0x40;			//CC_EN CHG_ON  DSG_ON

    Registers.CCCfg = 0x19;   //must be 0x19 for optimal performance



    result = GetADCGainOffset();

    Gain = (365 + ((Registers.ADCGain1.ADCGain1Byte & 0x0C) << 1) + ((Registers.ADCGain2.ADCGain2Byte & 0xE0)>> 5)) / 1000.0;
    iGain = 365 + ((Registers.ADCGain1.ADCGain1Byte & 0x0C) << 1) + ((Registers.ADCGain2.ADCGain2Byte & 0xE0)>> 5);

    Registers.OVTrip = (unsigned char)((((unsigned short)((OV_CELL_V - Registers.ADCOffset)/Gain + 0.5) - OV_THRESH_BASE) >> 4) & 0xFF);
    Registers.UVTrip = (unsigned char)((((unsigned short)((UV_CELL_V - Registers.ADCOffset)/Gain + 0.5) - UV_THRESH_BASE) >> 4) & 0xFF);

    for(i = 0; i < 5; i ++)
    {
        result = ConfigureBqMaximo();
        if(result == 0)
        {
            break;
        }
    }
    if(result != 0)
    {
        g_sys_state.val.I2C_error_flag = TRUE;
    }
    return result;
}

void UpdateVoltageFromBqMaximo()
{
    uint8_t i = 0;
    uint16_t vbat_temp = 0;

    Get_Cell_Volt(VC5_HI_BYTE, 2, 3);
    Get_Cell_Volt(VC1_HI_BYTE, 0, 2);
    Get_Cell_Volt(VC10_HI_BYTE, 5, 1);

    for(i = 0; i < CELLS_NUM; i ++)
    {
        vbat_temp += g_sys_signal.val.vcell[i];
    }
    if(g_sys_state.val.vbat_update_OK == TRUE)
    {
        g_sys_signal.val.vbat = vbat_temp;
    }
    else
    {
        if(g_sys_signal.val.vbat > vbat_temp)
        {
            if((g_sys_signal.val.vbat - vbat_temp) < 1000)
            {
                g_sys_signal.val.vbat = vbat_temp;
                g_sys_state.val.vbat_update_OK = TRUE;
            }
        }
        else
        {
//            if((vbat_temp - g_sys_signal.val.vbat) < 1000)
            {
                g_sys_signal.val.vbat = vbat_temp;
                g_sys_state.val.vbat_update_OK = TRUE;
            }
        }
    }

    g_sys_signal.val.max_cell_volt = g_sys_signal.val.vcell[0];
    g_sys_signal.val.min_cell_volt = g_sys_signal.val.vcell[0];
    for(i = 0; i < CELLS_NUM; i ++)
    {
        if(g_sys_signal.val.max_cell_volt < g_sys_signal.val.vcell[i])
        {
            g_sys_signal.val.max_cell_volt = g_sys_signal.val.vcell[i];
        }
        if(g_sys_signal.val.min_cell_volt > g_sys_signal.val.vcell[i])
        {
            g_sys_signal.val.min_cell_volt = g_sys_signal.val.vcell[i];
        }
    }
}

void Get_Cell_Volt(uint8_t Start_Address, uint8_t cell_start_num, uint8_t cell_num)
{
    int Result = 0;
    uint8_t i = 0;

    uint8_t bqMaximoProtectionConfig[10];
    uint16_t iTemp = 0;
    uint32_t lTemp = 0;

    WDT_RST_1S();
    Result = I2CReadBlockWithCRC(BQMAXIMO, Start_Address, bqMaximoProtectionConfig, 2*cell_num);

    if(Result == 0)
    {
        for (i = 0; i < cell_num; i ++)
        {
            iTemp = (uint16_t)(bqMaximoProtectionConfig[(i<<1)] << 8) + bqMaximoProtectionConfig[(i<<1) + 1];
            lTemp = ((uint32_t)iTemp * iGain) / 1000;
            lTemp += Registers.ADCOffset;
            g_sys_signal.val.vcell[cell_start_num + i] = lTemp;
        }
    }
    else
    {
        g_sys_state.val.I2C_error_flag = TRUE;
    }
}

int UpdateStatFromBq()
{
    int Result = 0;
//    uint8_t *pRawADCData = NULL;

    WDT_RST_1S();
    Result = I2CReadBlockWithCRC(BQMAXIMO, SYS_STAT, &(Registers.SysStatus.StatusByte), 2);

    Result = I2CReadBlockWithCRC(BQMAXIMO, SYS_CTRL1, &(Registers.SysCtrl1.SysCtrl1Byte), 2);

    //VBAT 3 TS
//    Result = I2CReadBlockWithCRC(BQMAXIMO, TS2_HI_BYTE, &(Registers.TS2.TS2Byte.TS2_HI), 2);

//    if(Result == 0)
//    {
        //TS
        //	pRawADCData = &Registers.TS1.TS1Byte.TS1_HI;
        //	iTemp = (unsigned int)(*pRawADCData << 8) + *(pRawADCData + 1);
        //	g_sys_signal.val.ntc1=AdToTemperature(iTemp);

//        pRawADCData = &Registers.TS2.TS2Byte.TS2_HI;
//        g_sys_signal.val.ntc1_adc_value = (unsigned int)(*pRawADCData << 8) + *(pRawADCData + 1);
//        g_sys_signal.val.temperature = AdToTemperature(g_sys_signal.val.ntc1_adc_value);

        //	pRawADCData = &Registers.TS3.TS3Byte.TS3_HI;
        //	iTemp = (unsigned int)(*pRawADCData << 8) + *(pRawADCData + 1);
        //	g_sys_signal.val.ntc3=AdToTemperature(iTemp);
//    }
    return Result;
}

int UpdateCurFromBq()
{
    int Result = 0;
    int iTemp = 0;

    if(Registers.SysStatus.StatusBit.CC_READY == 1)
    {
        WDT_RST_1S();
        Result = I2CReadBlockWithCRC(BQMAXIMO, CC_HI_BYTE, &(Registers.CC.CCByte.CC_HI), 2);

        iTemp = (int)((Registers.CC.CCByte.CC_HI << 8) + Registers.CC.CCByte.CC_LO);
        if(Result == 0)
        {
            g_sys_signal.val.cur = (long)iTemp * 844 / CUR_K;
        }
        else
        {
            g_sys_state.val.I2C_error_flag = TRUE;
        }
    }
    return Result;
}

//NEW BY XXY
void BqShutDown()
{
    __disable_interrupt();
    Registers.SysCtrl2.SysCtrl2Bit.CHG_ON = 0;  // 掉电之前先关闭充放电管  blust 20180913
    Registers.SysCtrl2.SysCtrl2Bit.DSG_ON = 0;
    I2CWriteBlockWithCRC(BQMAXIMO, SYS_CTRL2, &(Registers.SysCtrl2.SysCtrl2Byte), 1);
    delay_ms(5);
    LED_state0();
    Write_All_Info();   // 掉电之前存储所有需要存储的数据
    delay_ms(100);

    while(1)
    {
        WDT_RST_1S();
        Registers.SysCtrl1.SysCtrl1Bit.ADC_EN = 0;			//ADC_EN  TEMP_S  18
        Registers.SysCtrl2.SysCtrl2Bit.CC_EN  = 0;			//CC_EN CHG_ON  DSG_ON
        I2CWriteBlockWithCRC(BQMAXIMO, SYS_CTRL1, &(Registers.SysCtrl1.SysCtrl1Byte), 2);
        delay_ms(1);

        Registers.SysCtrl1.SysCtrl1Bit.SHUT_A = 0;
        Registers.SysCtrl1.SysCtrl1Bit.SHUT_B = 1;
        I2CWriteBlockWithCRC(BQMAXIMO, SYS_CTRL1, &(Registers.SysCtrl1.SysCtrl1Byte), 1);
        delay_us(25);
        Registers.SysCtrl1.SysCtrl1Bit.SHUT_A = 1;
        Registers.SysCtrl1.SysCtrl1Bit.SHUT_B = 0;
        I2CWriteBlockWithCRC(BQMAXIMO, SYS_CTRL1, &(Registers.SysCtrl1.SysCtrl1Byte), 1);
        delay_ms(500);
        ReConfigBq();
    }
}

uint8_t CRC8(uint8_t *ptr, uint8_t len, uint8_t key)
{
    uint8_t i;
    uint8_t crc = 0;

    while(len-- != 0)
    {
        for(i = 0x80; i != 0; i /= 2)
        {
            if((crc & 0x80) != 0)
            {
                crc *= 2;
                crc ^= key;
            }
            else
            {
                crc *= 2;
            }

            if((*ptr & i) != 0)
            {
                crc ^= key;
            }
        }
        ptr ++;
    }
    return(crc);
}


int I2CWriteRegisterByte(unsigned char I2CSlaveAddress, unsigned char Register, unsigned char Data)
{
    unsigned char DataBuffer[2];
    unsigned int SentByte = 0;

    DataBuffer[0] = Register;
    DataBuffer[1] = Data;

    return(I2CSendBytes(I2CSlaveAddress, DataBuffer, 2, &SentByte));
}

int I2CWriteRegisterByteWithCRC(unsigned char I2CSlaveAddress, unsigned char Register, unsigned char Data)
{
    unsigned char DataBuffer[4];
    unsigned int SentByte = 0;

    DataBuffer[0] = I2CSlaveAddress << 1;
    DataBuffer[1] = Register;
    DataBuffer[2] = Data;
    DataBuffer[3] = CRC8(DataBuffer, 3, CRC8_KEY);

    return(I2CSendBytes(I2CSlaveAddress, DataBuffer + 1, 3, &SentByte));
}

int I2CWriteRegisterWordWithCRC(unsigned char I2CSlaveAddress, unsigned char Register, unsigned int Data)
{
    unsigned char DataBuffer[6];
    unsigned int SentByte = 0;

    DataBuffer[0] = I2CSlaveAddress << 1;
    DataBuffer[1] = Register;
    DataBuffer[2] = LOW_BYTE(Data);
    DataBuffer[3] = CRC8(DataBuffer, 3, CRC8_KEY);
    DataBuffer[4] = HIGH_BYTE(Data);
    DataBuffer[5] = CRC8(DataBuffer + 4, 1, CRC8_KEY);

    return(I2CSendBytes(I2CSlaveAddress, DataBuffer + 1, 5, &SentByte));
}

int I2CWriteBlockWithCRC(unsigned char I2CSlaveAddress, unsigned char StartAddress, unsigned char *Buffer, unsigned char Length)
{
    unsigned char *BufferCRC, *Pointer;
    int i;
    unsigned int SentByte = 0;
    int result;

    BufferCRC = (unsigned char*)malloc(2*Length + 2);
    if (NULL == BufferCRC)
    {
        return -1;
    }

    Pointer = BufferCRC;
    *Pointer = I2CSlaveAddress << 1;
    Pointer ++;
    *Pointer = StartAddress;
    Pointer ++;
    *Pointer = *Buffer;
    Pointer ++;
    *Pointer = CRC8(BufferCRC, 3, CRC8_KEY);

    for(i = 1; i < Length; i ++)
    {
        Pointer ++;
        Buffer ++;
        *Pointer = *Buffer;
        *(Pointer + 1) = CRC8(Pointer, 1, CRC8_KEY);
        Pointer ++;
    }
    WDT_RST_1S();
    result = I2CSendBytes(I2CSlaveAddress, BufferCRC + 1, 2*Length + 1, &SentByte);

    free(BufferCRC);
    BufferCRC = NULL;

    return result;
}

int I2CWriteRegisterWord(unsigned char I2CSlaveAddress, unsigned char Register, unsigned int Data)
{
    unsigned char DataBuffer[3];
    unsigned int SentByte = 0;

    DataBuffer[0] = Register;
    DataBuffer[1] = LOWBYTE(Data);
    DataBuffer[2] = HIGHBYTE(Data);

    return(I2CSendBytes(I2CSlaveAddress, DataBuffer, 3, &SentByte));
}


int I2CReadRegisterByte(unsigned char I2CSlaveAddress, unsigned char Register, unsigned char *Data)
{
    unsigned char TargetRegister = Register;
    unsigned int SentByte = 0;
    unsigned int ReadDataCount = 0;
    int ReadStatus = 0;
    int WriteStatus = 0;
    WDT_RST_1S();
    WriteStatus = I2CSendBytes(I2CSlaveAddress, &TargetRegister, 1, &SentByte);
    WDT_RST_1S();
    ReadStatus = I2CReadBytes(I2CSlaveAddress, Data, 1, &ReadDataCount);

    if ((ReadStatus != 0)||(WriteStatus != 0))
    {
        return -1;
    }
    return 0;
}

int I2CReadBlock(unsigned char I2CSlaveAddress, unsigned char StartRegisterAddress, unsigned char *Buffer, unsigned int BlockSize, unsigned int *NumberOfBytes)
{
    unsigned char TargetRegister = StartRegisterAddress;
    unsigned int SentByte = 0;
    int ReadStatus = 0;
    int WriteStatus = 0;
    WDT_RST_1S();
    WriteStatus = I2CSendBytes(I2CSlaveAddress, &TargetRegister, 1, &SentByte);
    WDT_RST_1S();
    ReadStatus = I2CReadBytes(I2CSlaveAddress, Buffer, BlockSize, NumberOfBytes);

    if((ReadStatus != 0)||(WriteStatus != 0))
    {
        return -1;
    }
    return 0;  //success
}



int I2CReadRegisterByteWithCRC(unsigned char I2CSlaveAddress, unsigned char Register, unsigned char *Data)
{
    unsigned char TargetRegister = Register;
    unsigned int SentByte = 0;
    unsigned char ReadData[2];
    unsigned int ReadDataCount = 0;
    unsigned char CRCInput[2];
    unsigned char CRC = 0;
    int ReadStatus = 0;
    int WriteStatus = 0;
    WDT_RST_1S();
    WriteStatus = I2CSendBytes(I2CSlaveAddress, &TargetRegister, 1, &SentByte);
    WDT_RST_1S();
    ReadStatus = I2CReadBytes(I2CSlaveAddress, ReadData, 2, &ReadDataCount);

    if((ReadStatus != 0)||(WriteStatus != 0))
    {
        return -1;
    }

    CRCInput[0] = (I2CSlaveAddress << 1) + 1;
    CRCInput[1] = ReadData[0];

    CRC = CRC8(CRCInput, 2, CRC8_KEY);

    if (CRC != ReadData[1])
    {
        return -1;
    }
    *Data = ReadData[0];
    return 0;
}

int I2CReadRegisterWordWithCRC(unsigned char I2CSlaveAddress, unsigned char Register, unsigned int *Data)
{
    unsigned char TargetRegister = Register;
    unsigned int SentByte = 0;
    unsigned char ReadData[4];
    unsigned int ReadDataCount = 0;
    unsigned char CRCInput[2];
    unsigned char CRC = 0;
    int ReadStatus = 0;
    int WriteStatus = 0;
    WDT_RST_1S();
    WriteStatus = I2CSendBytes(I2CSlaveAddress, &TargetRegister, 1, &SentByte);
    WDT_RST_1S();
    ReadStatus = I2CReadBytes(I2CSlaveAddress, ReadData, 4, &ReadDataCount);

    if((ReadStatus != 0)||(WriteStatus != 0))
    {
        return -1;
    }

    CRCInput[0] = (I2CSlaveAddress << 1) + 1;
    CRCInput[1] = ReadData[0];

    CRC = CRC8(CRCInput, 2, CRC8_KEY);

    if (CRC != ReadData[1])
    {
        return -1;
    }
    CRC = CRC8(ReadData + 2, 1, CRC8_KEY);

    if (CRC != ReadData[3])
    {
        return -1;
    }

    *Data = ReadData[0];

    *Data = (*Data << 8) + ReadData[2];

    return 0;
}

int I2CReadBlockWithCRC(unsigned char I2CSlaveAddress, unsigned char Register, unsigned char *Buffer, unsigned char Length)
{
    unsigned char TargetRegister = Register;
    unsigned int SentByte = 0;
    unsigned char *ReadData = NULL, *StartData = NULL;
    unsigned int ReadDataCount = 0;
    unsigned char CRCInput[2];
    unsigned char CRC = 0;
    int ReadStatus = 0;
    int WriteStatus = 0;
    int i;

    StartData = (unsigned char *)malloc(2 * Length);

    if (NULL == StartData)
    {
        return -1;
    }

    ReadData = StartData;
    WDT_RST_1S();
    WriteStatus = I2CSendBytes(I2CSlaveAddress, &TargetRegister, 1, &SentByte);
    WDT_RST_1S();
    ReadStatus = I2CReadBytes(I2CSlaveAddress, ReadData, 2 * Length, &ReadDataCount);

    if((ReadStatus != 0)||(WriteStatus != 0))
    {
        free(StartData);
        StartData = NULL;
        return -1;
    }

    CRCInput[0] = (I2CSlaveAddress << 1) + 1;
    CRCInput[1] = *ReadData;

    CRC = CRC8(CRCInput, 2, CRC8_KEY);

    ReadData++;
    if (CRC != *ReadData)
    {
        free(StartData);
        StartData = NULL;
        return -1;
    }
    else
        *Buffer = *(ReadData - 1);

    for(i = 1; i < Length; i ++)
    {
        ReadData ++;
        CRC = CRC8(ReadData, 1, CRC8_KEY);
        ReadData ++;
        Buffer ++;

        if (CRC != *ReadData)
        {
            free(StartData);
            StartData = NULL;

            return -1;
        }
        else
            *Buffer = *(ReadData - 1);
    }

    free(StartData);
    StartData = NULL;

    return 0;
}
