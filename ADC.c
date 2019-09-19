/*
 * ADC.c
 *
 *  Created on: 2018年1月29日
 *      Author: zhao
 */

#include "ADC.h"

uint16_t adc_arr[CONVERSION_NUM];

////BT103F3435B
//const unsigned int TEMP_AD_TABLE[TEMP_DEGREE_RANGE] =  \
//{
//    7978, 7944, 7910, 7874, 7837, 7799, 7759, 7718, 7675, 7631,
//    7586, 7545, 7503, 7460, 7415, 7369, 7308, 7246, 7182, 7116,
//    7050, 6988, 6926, 6862, 6796, 6730, 6657, 6583, 6507, 6431,
//    6354, 6279, 6203, 6126, 6048, 5969, 5892, 5814, 5735, 5655,
//    5574, 5471, 5370, 5271, 5175, 5081, 5017, 4950, 4880, 4808,
//    4732, 4649, 4566, 4483, 4401, 4319, 4239, 4159, 4080, 4001,
//    3924, 3846, 3769, 3692, 3616, 3540, 3467, 3394, 3323, 3252,
//    3182, 3113, 3045, 2977, 2911, 2846, 2782, 2719, 2658, 2597,
//    2538, 2480, 2424, 2369, 2315, 2263, 2210, 2158, 2108, 2058,
//    2009, 1962, 1916, 1870, 1826, 1783, 1740, 1699, 1658, 1619,
//    1580, 1543, 1507, 1472, 1437, 1404, 1370, 1337, 1304, 1272,
//    1242, 1211, 1181, 1151, 1123, 1095, 1070, 1046, 1023, 1000,
//     978,  955,  932,  911,  890,  869,  849,  829,  810,  791
//};

/*
 * 103AT volt
 * 3.3V--10K--103AT---GND
 * REF 3.3V
 * ADC 10bit
 */
const unsigned int TEMP_VOLT_TABLE[MCU_TEMP_DEGREE_RANGE] =  \
{
    951,947,943,939,934,930,925,920,915,909,
    904,898,892,886,880,873,866,860,852,845,
    838,830,822,815,806,798,790,781,772,763,
    754,745,736,727,717,707,698,688,678,668,
    658,648,638,628,618,607,597,587,577,567,
    557,546,536,526,516,507,497,487,477,468,
    458,449,440,430,421,412,404,395,386,378,
    370,361,353,345,338,330,323,315,308,301,
    294,287,280,274,267,261,255,249,243,237,
    232,226,221,216,211,206,201,196,191,187,
    182,178,174,170,166,162,158,154,151,147,
    144,140,137,134,131,128,125,122,119,116,
    114,111,109,106,104,101,99,97,95,93,
    90,88,86,85,83,81,79,77,76,74,
//    72,71,69,68,66,65,64,62,61,60,
//    58,57,56,55,54,53,52,51,50,49,
//    48,47,46,45,44,43,42,41,41,40
};

// BQ采样使用温度转换子函数
//int16_t AdToTemperature(uint16_t temp_val)
//{
//    // 折半查表法，最多需要8次找到温度值
//    uint16_t low = 0;
//    uint16_t high = TEMP_DEGREE_RANGE - 1;
//    uint8_t mid = 0;
//    int16_t itemp = 0;
//
//    while(low < high)
//    {
//        mid = (low + high) >> 1;
//
//        if(temp_val == TEMP_AD_TABLE[mid])
//        {
//            break;
//        }                               // 索引到刚好相等的值，则马上返回
//        if(high - low == 1)
//        {
//            mid = low;                 // 由于不是精确查找，若在小区间内，取小值
//            break;
//        }
//        if(temp_val < TEMP_AD_TABLE[mid])
//        {
//            low = mid;
//        }
//        else
//        {
//            high = mid;
//        }
//    }
//
//    if(mid == TEMP_DEGREE_RANGE - 1)
//    {   // 小区间内对温度和AD值进行线性拟合
//        itemp = ((int16_t)mid - TEMP_OFFSET_VAL) * 10 + 10 * (uint32_t)(TEMP_AD_TABLE[mid] - temp_val)/(TEMP_AD_TABLE[mid-1] - TEMP_AD_TABLE[mid]);
//    }
//    else
//    {
//        itemp = ((int16_t)mid - TEMP_OFFSET_VAL) * 10 + 10 * (uint32_t)(TEMP_AD_TABLE[mid] - temp_val)/(TEMP_AD_TABLE[mid] - TEMP_AD_TABLE[mid+1]);
//    }
//
//    return itemp;
//}

// MCU采样使用温度转换子函数
int16_t MCU_AdToTemperature(uint16_t temp_val)
{
    // 折半查表法，最多需要8次找到温度值
    uint16_t low = 0;
    uint16_t high = MCU_TEMP_DEGREE_RANGE - 1;
    uint8_t mid = 0;
    int16_t itemp = 0;

    while(low < high)
    {
        mid = (low + high) >> 1;

        if(temp_val == TEMP_VOLT_TABLE[mid])
        {
            break;
        }                               // 索引到刚好相等的值，则马上返回
        if(high - low == 1)
        {
            mid = low;                 // 由于不是精确查找，若在小区间内，取小值
            break;
        }
        if(temp_val < TEMP_VOLT_TABLE[mid])
        {
            low = mid;
        }
        else
        {
            high = mid;
        }
    }

    if(mid == MCU_TEMP_DEGREE_RANGE - 1)
    {   // 小区间内对温度和AD值进行线性拟合
        itemp = ((int16_t)mid - MCU_TEMP_OFFSET_VAL) * 10 + 10 * ((int32_t)(TEMP_VOLT_TABLE[mid]) - temp_val)/(TEMP_VOLT_TABLE[mid-1] - TEMP_VOLT_TABLE[mid]);
    }
    else
    {
        itemp = ((int16_t)mid - MCU_TEMP_OFFSET_VAL) * 10 + 10 * ((int32_t)(TEMP_VOLT_TABLE[mid]) - temp_val)/(TEMP_VOLT_TABLE[mid] - TEMP_VOLT_TABLE[mid+1]);
    }

    return itemp;
}

void InitAdc(uint16_t *adc_data_arr, uint8_t mode)
{
    ADC10CTL0 &= ~ENC;//关闭采样使能
    while(ADC10CTL1 & BUSY);//检测AD是否繁忙
    ADC10CTL0 = 0x0000;//复位该寄存器
    if(mode == MODE_2_5)
    {
        ADC10CTL0 |= ADC10SHT_2 + MSC + ADC10ON + ADC10IE + SREF_1 + REF2_5V + REFON;  //2.5
    }
    else
    {
        ADC10CTL0 |= ADC10SHT_2 + MSC + ADC10ON + ADC10IE + SREF_0;// + REF2_5V + REFON;  //3.3 采样温度时采用
    }
    ADC10DTC1 |= CONVERSION_NUM;
    ADC10SA = (uint16_t)adc_data_arr;//获取a[]的首地址。首先对A0采样，放入a[0]。如此循环下去。
}

void StartAdc(void)
{
    ADC10CTL0 &= ~ENC;//关闭采样使能
    while(ADC10CTL1 & BUSY);//检测AD是否繁忙
    ADC10CTL0 |= ENC + ADC10SC;//启动ADC
    _BIS_SR(LPM0_bits + GIE);//低功耗模式0，开中断
}

void Vbat_volt(void)
{
    uint32_t l_temp;
    uint16_t tmp_adc = 0;
    InitAdc(adc_arr, MODE_2_5);
    ADC10CTL1 = CONSEQ_2 + INCH_5;//单通道多次次转换
    ADC10AE0 = BIT5;
    StartAdc();
    tmp_adc = average_data(adc_arr);
    l_temp = ((uint32_t)tmp_adc * 2500) >> 10;
    g_sys_signal.val.vbat = l_temp * 184 / 11;
}

void Vchg_volt(void)
{
    uint16_t tmp_adc = 0;
    InitAdc(adc_arr, MODE_2_5);
    ADC10CTL1 = CONSEQ_2 + INCH_4;//单通道多次次转换
    ADC10AE0 = BIT4;
    StartAdc();
    tmp_adc = average_data(adc_arr);
    g_sys_signal.val.vchg_ad_cp = tmp_adc;
    g_sys_signal.val.vchg_ad_cb = 0;

    if(g_sys_signal.val.vchg_ad_cp < VCHG_CP_LOW)
    {
        EN_VCHG_CHK();  // 使能充电检测
        delay_ms(2);
        InitAdc(adc_arr, MODE_2_5);
        ADC10CTL1 = CONSEQ_2 + INCH_3;//单通道多次次转换
        ADC10AE0 = BIT3;
        StartAdc();
        tmp_adc = average_data(adc_arr);
        g_sys_signal.val.vchg_ad_cb = tmp_adc;
        DIS_VCHG_CHK(); // 禁止充电器电压检测
    }
    g_sys_signal.val.vchg_val_cb = ((uint32_t)g_sys_signal.val.vchg_ad_cb * 2500 * 17) >> 10;
}

void Temp_Get(void)
{
    uint16_t tmp_adc = 0;
    InitAdc(adc_arr, MODE_3_3);
    ADC10CTL1 = CONSEQ_2 + INCH_0;//单通道多次次转换
    ADC10AE0 = BIT0;
    StartAdc();
    tmp_adc = average_data(adc_arr);
    g_sys_signal.val.temperature = MCU_AdToTemperature(tmp_adc);
}

#pragma vector=ADC10_VECTOR
__interrupt void ADC_ISR(void)
{
     LPM0_EXIT;//退出低功耗模式
}

