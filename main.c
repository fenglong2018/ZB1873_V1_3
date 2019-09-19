/*---------------------------------------
 * 科沃斯ZB1873项目，MSP430G2553 + BQ76930 方案，充放电异口，放电负端保护，充电正端保护，带防反接二极管。
 * 二级保护采用两颗R5640级联。上电瞬间由单片机AD获取整包电压，用以判断开机保护或起充保护。
 * 单颗双色LED指示，红灯闪烁，蓝灯常亮与呼吸显示；单线通讯，波特率9600；通讯线与TRIG线共用一条线。
 * -------------更改记录-----------------
 * 版本           修改人               修改日期                 修改内容
 * V1.0		耿汉武		2018-09-05    初始版本，重新编写逻辑功能。
 * V1.0     耿汉武             2018-09-14  此日期之前，完成了保护标志位判断、保护逻辑执行、掉电逻辑执行等内容。
 * V1.0     耿汉武             2018-09-18  完成通讯协议、端口配置、上电时VBAT电压采样、充电在位检测和TRIG检测。
 * V1.0     耿汉武             2018-09-19  完成温度查表与计算（单位0.1℃）、初始化流程、定时器时基问题、LED控制逻辑等内容。
 * V1.0     耿汉武             2018-09-28  调试时发现串口波特率不正常，检查发现波特率配置不对，MCU内置低频时钟频率为12kHz，而不是32768Hz。
 *                                  将波特率时钟改为SMCLK并重新配置后调通串口。
 * V1.0     耿汉武             2018-09-29  修改P1.2配置，平时为普通IO，需要发送时再配置为串口TX，不然平时无法拉低。
 * V1.0     耿汉武             2018-09-30  修改放电高温保护存储模式，防止覆盖掉存储区域内其他数据。
 * V1.0     耿汉武             2018-09-30  修改FLASH分配，留出SN码存储位置，增加通讯协议，上传SN码。
 * V1.0     耿汉武             2018-10-09  修改FLASH操作时钟频率，将分频系数修改为21,8MHz主频，分频后频率约为380kHz
 * V1.0     耿汉武             2018-10-09  按照新通讯协议需求修改程序，修改轻载掉电延时可控逻辑
 * V1.0     耿汉武             2018-10-09  修改TRIG检测逻辑，只在TRIG中断未允许时进行使能动作，并且使能检测之后延时100ms再打开中断，防止干扰。
 * V1.0     耿汉武             2018-10-11  取消欠压保护后恢复动作，防止保护后电压回弹重新开输出。
 * V1.0     耿汉武             2018-10-11  修改电芯最大最小压差计算方法，将下标记录更改为极值记录。
 * V1.0     耿汉武             2018-10-16  按客户需求修改LED灯逻辑：电池电量≤20%后，常亮红色，＞20%，常亮绿色
 * V1.1     耿汉武             2018-10-31  扩充容量表格，由34个点扩充为100个点。
 * V1.1     耿汉武             2018-10-31  完善温度算法，两张表优化为一张表，保留AD值表，去除温度值表，由AD值表下标确定温度值。
 * V1.1     耿汉武             2018-11-06  修改充放电逻辑，充电过程中全程打开放电管，关闭放电管动作只会发生在以下四种情况：
 *                                  发生放电保护、超过1S没有通讯、轻载保护、通讯接收到停止输出命令。
 * V1.1     耿汉武             2018-11-06  充电器在位时，系统处于充电状态（虽然此时允许放电），轻载掉电逻辑无效
 * V1.1     耿汉武             2018-11-13  更新容量算法，搭建新算法的程序框架。
 * V1.1     耿汉武             2018-11-14  修改呼吸灯频率，定时器改为10ms周期，呼吸灯周期改为2.5s，灭到亮1S，保持全亮0.25S，亮到灭1S，保持全灭0.25S。
 * V1.1     耿汉武             2018-11-14  修改放电过流短路保护逻辑：放电短路直接掉电LED无显示，放电过流立即关输出，红灯快闪6S掉电
 * V1.1     耿汉武             2018-11-14  修改充电5H掉电逻辑，不是充电器接入时间，而是真正有充电电流的累计时间，中途如果电流消失则重新开始计时。
 *                                  达到5H之后也不掉电，进入充饱状态，蓝灯常亮5min后熄灭。
 * V1.1     耿汉武             2018-11-14  修改起充保护逻辑，在有充满标志存在的情况才会起充保护，未充满之前重新拔插充电器不会触发起充保护。
 *                                  充满标志位在充电过压或充电超过5H会被置位，在（单节电压低于4.1V或SOC值小于等于97%）并且充电器未在位时清除。
 *                                  起充保护标志采用充满标志代替（逻辑上完全满足点检要求，2018-11-28）
 * V1.1     耿汉武             2018-11-15  容量算法已完成，剩余无法放出容量修正值与电流和温度相关，采用按照回弹电压查OCV表格的方式获取。
 * V1.1     耿汉武             2018-11-15  增加TRIG检测到位后延时开输出，防止30S等待期间TRIG信号进入后会立即开输出，没有延时60ms。
 * V1.2     耿汉武             2018-11-21  调整电路，重新布板，调整IO口，充电检测改为AD采样检测。
 * V1.2     耿汉武             2018-11-22  增加充电器过压判断：充电器电压高于32V禁止充电，红灯快闪6S掉电。放电管关闭时，充电器检测到的电压要减去电池包电压。
 * V1.2     耿汉武             2018-11-23  修改电芯压差保护逻辑，放电时不进行电芯压差保护判断。
 * V1.2     耿汉武             2018-11-23  电池充满电后，如果是单电池充电（没有通讯），蓝灯常亮5min后熄灭，30s掉电。
 * V1.3     耿汉武             2018-11-29  根据新需求修改点检逻辑。1、放电短路保护由立即掉电改为红灯快闪6S掉电。
 *                                  2、过放（放电欠压保护）增加为立即关闭输出，红灯闪烁（1S亮1S灭）三次（共计6S）后掉电。
 *                                  3、充电器在位时不响应停止供电命令；通讯帧数据超出正常范围时不响应；轻载保护设为1.5H后取消容量分档逻辑。
 * V1.3     耿汉武             2018-11-29  修改充电器电压判断逻辑，有充电电流时，不采样充电器电压，认为充电器已在位；
 *                                  无充电电流时，采样充电器电压判断，大于VBAT则认为充电器在位。
 *                                  充电器电压采样值，在放电管未打开时，如果采样值大于44V，采样值减去VBAT的差值作为充电器电压。
 * V1.3     耿汉武             2018-11-29  轻载保护掉电之前先关充放电管，1S之后再掉电，防止再次激活系统。
 * V1.3     耿汉武             2018-12-01  按照新修改的协议增加工作状态内容：0x1A 正常放电；0x0A正常充电；……
 * V1.3     耿汉武             2018-12-01  存储放电欠压标志，增加逻辑：过放欠压后再次开机不能放电，红灯闪烁（1S亮1S灭）三次后掉电。充电解除欠压标志。
 * V1.3     耿汉武             2018-12-02  修改温度采样，由BQ采样温度修改为MCU采样温度。
 * V1.3     耿汉武             2018-12-02  增加NTC短路/断路检测，检测到温度异常偏高或偏低，认为NTC短路/短路，立即关闭输出。
 * V1.3     耿汉武             2018-12-02  修改充电器检测逻辑：
 *                                  先采集 C+ P-减法电路AD值，该AD值可判断充电器在位则不采集C+ B-分压电路AD值，否则采集C+ B-分压电路AD值。
 *                                  即： C+ P-减法电路AD值有效则依据该值判断，否则依据C+ B-分压电路AD值判断。
 * V1.3     耿汉武             2018-12-05  校准充电器电压采样AD值。
 * V1.3     耿汉武             2018-12-05  修改充电器移除判断：充电口电压高于26V认为充电器接入，充电口电压低于VBAT电压才认为充电器移除。
 * V1.3     耿汉武             2018-12-05  修改满电逻辑判断：充电过压后如果显示SOC没有更新到100%，则继续以5S每1%的速率更新，更新到100%后再开始常亮5min。
 * V1.3     耿汉武             2018-12-05  修正温度采样BUG，切换ADC采样基准时需要先清零ADC10CTL0寄存器再重新配置基准电压。
 * V1.3     耿汉武             2018-12-06  修改充电器在位判断逻辑，增加100ms延时，充电器在位超过100ms才认为充电器在位并开始判断是否充电器高压，防止高压误判。
 * V1.3     耿汉武             2018-12-06  修改容量保留值，保证0%之后还可以放电5S。CapProc.c文件的210行：剩余容量算法中2750改为2932。
 * V1.3     耿汉武             2018-12-06  取消存储放电欠压标志。
 * V1.3     耿汉武             2018-12-06  修改满电逻辑，没有发生过压时SOC应该保持在99%，过压时变为100%。
 *
 ---------------------------------------*/

#include"Global.h"
#include"string.h"

/*
 * main.c
 */

const uint16_t version = 0x0004;    // 版本号，0x0001对应版本V1.0

void KWS_Flash_Init(void)
{
    uint8_t i = 0;
    uint16_t cycle_cap_temp;

    for(i=0;i<16;i++)
    {
        if(((Flash_Read_2Byte((unsigned int *) SegmentCStart,i*2)==0xFFFF)&&(i>0)) || (i==15))
        {
            if((i==15)&&(Flash_Read_2Byte((unsigned int *) SegmentCStart,i*2) != 0xFFFF))
            {
                g_sys_cap.val.bat_cycle_cnt = Flash_Read_2Byte((unsigned int *) SegmentCStart,i*2);
                cycle_cap_temp = Flash_Read_2Byte((unsigned int *) SegmentCStart,i*2 + 1);
                info_write_cnt = 16;
            }
            else
            {
                g_sys_cap.val.bat_cycle_cnt = Flash_Read_2Byte((unsigned int *) SegmentCStart, i*2 - 2);
                cycle_cap_temp = Flash_Read_2Byte((unsigned int *) SegmentCStart, i*2 - 1);
                info_write_cnt = i;
            }
            g_sys_cap.val.cycle_cap_val = cycle_cap_temp & 0x0FFF;
            if(cycle_cap_temp & 0x8000)
            {
                g_sys_flags.val.dch_temp_high_protect_flag = TRUE;
            }
            if(cycle_cap_temp & 0x4000)
            {
                g_sys_state.val.vbat_chg_full_flag = TRUE;
            }
//            if(cycle_cap_temp & 0x2000)
//            {
//                g_sys_flags.val.soft_vcell_low_protect_flag = TRUE;
//            }
            break;
        }
        if(Flash_Read_2Byte((unsigned int *) SegmentCStart,i*2)==0xFFFF && (i==0))
        {
            info_write_cnt = 0;
            break;
        }
    }

    if(info_write_cnt == 0)
    {
        g_sys_cap.val.bat_cycle_cnt = 0;
        g_sys_cap.val.cycle_cap_val = 0;
    }
    else if(g_sys_cap.val.bat_cycle_cnt > 5000)
    {   // 一天循环一次的话，5000次循环相当于13年多
        g_sys_cap.val.bat_cycle_cnt = 0;
        g_sys_cap.val.cycle_cap_val = 0;
        g_sys_flags.val.dch_temp_high_protect_flag = FALSE;
        g_sys_state.val.vbat_chg_full_flag = FALSE;
//        g_sys_flags.val.soft_vcell_low_protect_flag = FALSE;
    }
    else if(g_sys_cap.val.cycle_cap_val > BAT_LEARNNING_CAPACITY_MAX_AD)
    {
        g_sys_cap.val.cycle_cap_val = 0;
        g_sys_flags.val.dch_temp_high_protect_flag = FALSE;
        g_sys_state.val.vbat_chg_full_flag = FALSE;
//        g_sys_flags.val.soft_vcell_low_protect_flag = FALSE;
    }

    g_sys_cap.val.full_cap = BAT_NORMAL_CAP;
}

void SysParmInit(void)
{
    memset((void*)&g_sys_state, 0, sizeof(g_sys_state));
    memset((void*)&g_sys_flags, 0, sizeof(g_sys_flags));
    memset((void*)&g_sys_signal,0, sizeof(g_sys_signal));
    memset((void*)&g_cap_flags, 0, sizeof(g_cap_flags));
    memset((void*)&g_sys_cap, 0, sizeof(g_sys_cap));

    KWS_Flash_Init();
    InitSoc();
}

void charge_judge(void)  //充电器在位判断
{
    static uint8_t chg_check_cnt = 0;
    static uint8_t trig_on_delay_cnt = 0;
    static uint8_t charger_on_cnt = 0;
    static uint8_t charger_over_cnt = 0;

    if((uint8_t)(sys_10ms_cnt - chg_check_cnt) >= 10)
    {
        chg_check_cnt = sys_10ms_cnt;
        if (g_sys_state.val.chg_state_flag == TRUE)
        {
            g_sys_state.val.charger_on_flag = TRUE;
            g_sys_state.val.chg_on_checking_flag = FALSE;
        }
        else
        {
            Vchg_volt();    // 获取VCHG电压值
            if((g_sys_signal.val.vchg_ad_cp > VCHG_CP_LOW)||(g_sys_signal.val.vchg_ad_cb > VCHG_CB_LOW))
            {   // VCHG_CP 大于 C+ P-减法电路AD值下限，或者VCHG_CB 大于 C+ B-分压电路AD值下限，则认为充电器在位
                if(g_sys_state.val.charger_on_flag == FALSE)
                {
                    g_sys_state.val.chg_on_checking_flag = TRUE;
                }
                else
                {
                    g_sys_state.val.chg_on_checking_flag = FALSE;
                }
                charger_on_cnt ++;
                if(charger_on_cnt > 5)
                {
                    charger_on_cnt = 0;
                    if(g_sys_state.val.charger_on_flag == FALSE)
                    {
                        if((g_sys_signal.val.vchg_ad_cp > VCHG_CP_HIGH)||(g_sys_signal.val.vchg_ad_cb > VCHG_CB_HIGH))
                        {
                            charger_over_cnt ++;
                            if(charger_over_cnt >= 6)
                            {
                                charger_over_cnt = 0;
                                g_sys_state.val.charger_on_flag = TRUE;
                                g_sys_flags.val.chg_val_high_err_flag = TRUE;
                                g_sys_state.val.chg_on_checking_flag = FALSE;
                            }
                        }
                        else
                        {
                            charger_over_cnt = 0;
                            g_sys_state.val.charger_on_flag = TRUE;
                            g_sys_state.val.chg_on_checking_flag = FALSE;

                            g_sys_state.val.dch_on_flag = TRUE;     // 充电器接入则允许放电 blust 20181106
                            uart_continue_cnt = sys_10ms_cnt;
                            P2IE &= ~BIT2;      // 禁止TRIG中断
                            g_sys_state.val.trig_isr_flag = FALSE;
                            comm_led_state = led_comm_normol;
                        }
                    }
                }
            }
            else
            {
                g_sys_state.val.chg_on_checking_flag = FALSE;
                charger_on_cnt = 0;
                charger_over_cnt = 0;
                if((g_sys_signal.val.vchg_ad_cp < VCHG_CP_LOW)&&(g_sys_signal.val.vchg_val_cb < g_sys_signal.val.vbat))
                {
                    g_sys_state.val.charger_on_flag = FALSE;
                }
            }
        }
    }

    if(g_sys_state.val.trig_on_flag == TRUE)
    {   // TRIG检测延时开输出
        if((uint8_t)(sys_10ms_cnt - trig_on_delay_cnt) >= 6)
        {
            trig_on_delay_cnt = sys_10ms_cnt;
            g_sys_state.val.trig_on_flag = FALSE;
            g_sys_state.val.dch_on_flag = TRUE;     // 允许放电
            uart_continue_cnt = sys_10ms_cnt;
        }
    }
    else
    {
        trig_on_delay_cnt = sys_10ms_cnt;
    }

    if((g_sys_state.val.charger_on_flag == FALSE)&&(g_sys_state.val.dch_on_flag == FALSE)&&(g_sys_state.val.trig_on_flag == FALSE))
    {   //充电器未在位 且 不处于放电允许状态
        if(g_sys_state.val.trig_isr_flag == FALSE)
        {
            delay_ms(50);
            P2IFG &= ~BIT2;         //清除IO口中断标志
            P2IE  |= BIT2;      // TRIG中断使能
            g_sys_state.val.trig_isr_flag = TRUE;
        }
    }
}

int main(void)
{
    __delay_cycles(200);

    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    BCSCTL3 |= LFXT1S_2; //32k crystal selected
    BCSCTL3 |= XCAP_3;   //~12pF effective capacitance (for G2553)
    BCSCTL1 |= DIVA_0;   //ACLK divided by 0

    if((CALBC1_8MHZ ==0xFF)||(CALDCO_8MHZ == 0xFF))
    {
        while(1)                              // If calibration constants erased	// do not load, trap CPU!!
        {
            delay_ms(500);
            asm("  MOV &0xFFFE,PC;");//将PC转移到应用程序的复位向量处..调用汇编的时候记得前面有空格！！！
        }
    }
    BCSCTL1 = CALBC1_8MHZ;                    					// Set DCO   默认MCLK为DCO
    DCOCTL = CALDCO_8MHZ;

    __disable_interrupt();
    InitIO();
    I2CInitialise();
    Flash_init();
    __enable_interrupt();   // 开总中断
    InitialisebqMaximo();
    if(g_sys_state.val.I2C_error_flag == TRUE)
    {
        g_sys_state.val.I2C_error_flag = FALSE;
        delay_ms(10);
        ReConfigBq();
    }

    Uart_Init();
    SysParmInit();
    timer0_A0_init();
    timer1_A0_init();
    pwm_4k_start(0);

    delay_ms(25);
    Vbat_volt();    // 获取VBAT电压值
    Vchg_volt();    // 获取VCHG电压值
    Temp_Get();     // 获取温度值
    if((g_sys_signal.val.vchg_ad_cp > VCHG_CP_LOW)||(g_sys_signal.val.vchg_ad_cb > VCHG_CB_LOW))
    {   // VCHG_CP 大于 C+ P-减法电路AD值下限，或者VCHG_CB 大于 C+ B-分压电路AD值下限，则认为充电器在位
//        g_sys_state.val.charger_on_flag = TRUE;
        g_sys_state.val.chg_on_checking_flag = TRUE;

//        if((g_sys_signal.val.vchg_ad_cp > VCHG_CP_HIGH)||(g_sys_signal.val.vchg_ad_cb > VCHG_CB_HIGH))
//        {   // VCHG_CP 大于 C+ P-减法电路AD值上限，或者VCHG_CB 大于 C+ B-分压电路AD值上限，则认为充电器过压
//            g_sys_flags.val.chg_val_high_err_flag = TRUE;
//        }
    }
   // 只要激活系统，就处于允许放电状态 blust 20181106
    g_sys_state.val.dch_on_flag = TRUE;
    uart_continue_cnt = sys_10ms_cnt;

    DIS_VBAT_CHK();     // 禁止Vbat检测

    while(1)
    {
        WDT_RST_1S();     //set WDT
        charge_judge();
        UartDataPro();
        Update_data();
        SysInfoProc();
        SysCtrlProc();
        ShutDownProc();
        NormalCapacityProc();
        LED_task();
        BQErrorClear();
    }
}

#pragma vector=PORT2_VECTOR
interrupt void PORT2_ISR (void)
{
    P2IFG &= ~BIT2;			//清除IO口中断标志

    g_sys_state.val.trig_on_flag = TRUE;     // TRIG在位
    P2IE &= ~BIT2;      // 禁止TRIG中断
    g_sys_state.val.trig_isr_flag = FALSE;
}

//#pragma vector=WDT_VECTOR       //看门狗中断入口
//interrupt void WDT(void)
//{
//    IFG1 &= ~WDTIFG;            //清除看门狗中断标志
//    _bic_SR_register_on_exit(LPM3_bits);// 退出低功耗模式
//}
