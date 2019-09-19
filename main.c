/*---------------------------------------
 * ����˹ZB1873��Ŀ��MSP430G2553 + BQ76930 ��������ŵ���ڣ��ŵ縺�˱�����������˱������������Ӷ����ܡ�
 * ����������������R5640�������ϵ�˲���ɵ�Ƭ��AD��ȡ������ѹ�������жϿ�����������䱣����
 * ����˫ɫLEDָʾ�������˸�����Ƴ����������ʾ������ͨѶ��������9600��ͨѶ����TRIG�߹���һ���ߡ�
 * -------------���ļ�¼-----------------
 * �汾           �޸���               �޸�����                 �޸�����
 * V1.0		������		2018-09-05    ��ʼ�汾�����±�д�߼����ܡ�
 * V1.0     ������             2018-09-14  ������֮ǰ������˱�����־λ�жϡ������߼�ִ�С������߼�ִ�е����ݡ�
 * V1.0     ������             2018-09-18  ���ͨѶЭ�顢�˿����á��ϵ�ʱVBAT��ѹ�����������λ����TRIG��⡣
 * V1.0     ������             2018-09-19  ����¶Ȳ������㣨��λ0.1�棩����ʼ�����̡���ʱ��ʱ�����⡢LED�����߼������ݡ�
 * V1.0     ������             2018-09-28  ����ʱ���ִ��ڲ����ʲ���������鷢�ֲ��������ò��ԣ�MCU���õ�Ƶʱ��Ƶ��Ϊ12kHz��������32768Hz��
 *                                  ��������ʱ�Ӹ�ΪSMCLK���������ú��ͨ���ڡ�
 * V1.0     ������             2018-09-29  �޸�P1.2���ã�ƽʱΪ��ͨIO����Ҫ����ʱ������Ϊ����TX����Ȼƽʱ�޷����͡�
 * V1.0     ������             2018-09-30  �޸ķŵ���±����洢ģʽ����ֹ���ǵ��洢�������������ݡ�
 * V1.0     ������             2018-09-30  �޸�FLASH���䣬����SN��洢λ�ã�����ͨѶЭ�飬�ϴ�SN�롣
 * V1.0     ������             2018-10-09  �޸�FLASH����ʱ��Ƶ�ʣ�����Ƶϵ���޸�Ϊ21,8MHz��Ƶ����Ƶ��Ƶ��ԼΪ380kHz
 * V1.0     ������             2018-10-09  ������ͨѶЭ�������޸ĳ����޸����ص�����ʱ�ɿ��߼�
 * V1.0     ������             2018-10-09  �޸�TRIG����߼���ֻ��TRIG�ж�δ����ʱ����ʹ�ܶ���������ʹ�ܼ��֮����ʱ100ms�ٴ��жϣ���ֹ���š�
 * V1.0     ������             2018-10-11  ȡ��Ƿѹ������ָ���������ֹ�������ѹ�ص����¿������
 * V1.0     ������             2018-10-11  �޸ĵ�о�����Сѹ����㷽�������±��¼����Ϊ��ֵ��¼��
 * V1.0     ������             2018-10-16  ���ͻ������޸�LED���߼�����ص�����20%�󣬳�����ɫ����20%��������ɫ
 * V1.1     ������             2018-10-31  �������������34��������Ϊ100���㡣
 * V1.1     ������             2018-10-31  �����¶��㷨�����ű��Ż�Ϊһ�ű�����ADֵ��ȥ���¶�ֵ����ADֵ���±�ȷ���¶�ֵ��
 * V1.1     ������             2018-11-06  �޸ĳ�ŵ��߼�����������ȫ�̴򿪷ŵ�ܣ��رշŵ�ܶ���ֻ�ᷢ�����������������
 *                                  �����ŵ籣��������1Sû��ͨѶ�����ر�����ͨѶ���յ�ֹͣ������
 * V1.1     ������             2018-11-06  �������λʱ��ϵͳ���ڳ��״̬����Ȼ��ʱ����ŵ磩�����ص����߼���Ч
 * V1.1     ������             2018-11-13  ���������㷨������㷨�ĳ����ܡ�
 * V1.1     ������             2018-11-14  �޸ĺ�����Ƶ�ʣ���ʱ����Ϊ10ms���ڣ����������ڸ�Ϊ2.5s������1S������ȫ��0.25S��������1S������ȫ��0.25S��
 * V1.1     ������             2018-11-14  �޸ķŵ������·�����߼����ŵ��·ֱ�ӵ���LED����ʾ���ŵ�����������������ƿ���6S����
 * V1.1     ������             2018-11-14  �޸ĳ��5H�����߼������ǳ��������ʱ�䣬���������г��������ۼ�ʱ�䣬��;���������ʧ�����¿�ʼ��ʱ��
 *                                  �ﵽ5H֮��Ҳ�����磬����䱥״̬�����Ƴ���5min��Ϩ��
 * V1.1     ������             2018-11-14  �޸���䱣���߼������г�����־���ڵ�����Ż���䱣����δ����֮ǰ���°β��������ᴥ����䱣����
 *                                  ������־λ�ڳ���ѹ���糬��5H�ᱻ��λ���ڣ����ڵ�ѹ����4.1V��SOCֵС�ڵ���97%�����ҳ����δ��λʱ�����
 *                                  ��䱣����־���ó�����־���棨�߼�����ȫ������Ҫ��2018-11-28��
 * V1.1     ������             2018-11-15  �����㷨����ɣ�ʣ���޷��ų���������ֵ��������¶���أ����ð��ջص���ѹ��OCV���ķ�ʽ��ȡ��
 * V1.1     ������             2018-11-15  ����TRIG��⵽λ����ʱ���������ֹ30S�ȴ��ڼ�TRIG�źŽ����������������û����ʱ60ms��
 * V1.2     ������             2018-11-21  ������·�����²��壬����IO�ڣ�������ΪAD������⡣
 * V1.2     ������             2018-11-22  ���ӳ������ѹ�жϣ��������ѹ����32V��ֹ��磬��ƿ���6S���硣�ŵ�ܹر�ʱ���������⵽�ĵ�ѹҪ��ȥ��ذ���ѹ��
 * V1.2     ������             2018-11-23  �޸ĵ�оѹ����߼����ŵ�ʱ�����е�оѹ����жϡ�
 * V1.2     ������             2018-11-23  ��س����������ǵ���س�磨û��ͨѶ�������Ƴ���5min��Ϩ��30s���硣
 * V1.3     ������             2018-11-29  �����������޸ĵ���߼���1���ŵ��·���������������Ϊ��ƿ���6S���硣
 *                                  2�����ţ��ŵ�Ƿѹ����������Ϊ�����ر�����������˸��1S��1S�����Σ�����6S������硣
 *                                  3���������λʱ����Ӧֹͣ�������ͨѶ֡���ݳ���������Χʱ����Ӧ�����ر�����Ϊ1.5H��ȡ�������ֵ��߼���
 * V1.3     ������             2018-11-29  �޸ĳ������ѹ�ж��߼����г�����ʱ���������������ѹ����Ϊ���������λ��
 *                                  �޳�����ʱ�������������ѹ�жϣ�����VBAT����Ϊ�������λ��
 *                                  �������ѹ����ֵ���ڷŵ��δ��ʱ���������ֵ����44V������ֵ��ȥVBAT�Ĳ�ֵ��Ϊ�������ѹ��
 * V1.3     ������             2018-11-29  ���ر�������֮ǰ�ȹس�ŵ�ܣ�1S֮���ٵ��磬��ֹ�ٴμ���ϵͳ��
 * V1.3     ������             2018-12-01  �������޸ĵ�Э�����ӹ���״̬���ݣ�0x1A �����ŵ磻0x0A������磻����
 * V1.3     ������             2018-12-01  �洢�ŵ�Ƿѹ��־�������߼�������Ƿѹ���ٴο������ܷŵ磬�����˸��1S��1S�����κ���硣�����Ƿѹ��־��
 * V1.3     ������             2018-12-02  �޸��¶Ȳ�������BQ�����¶��޸�ΪMCU�����¶ȡ�
 * V1.3     ������             2018-12-02  ����NTC��·/��·��⣬��⵽�¶��쳣ƫ�߻�ƫ�ͣ���ΪNTC��·/��·�������ر������
 * V1.3     ������             2018-12-02  �޸ĳ��������߼���
 *                                  �Ȳɼ� C+ P-������·ADֵ����ADֵ���жϳ������λ�򲻲ɼ�C+ B-��ѹ��·ADֵ������ɼ�C+ B-��ѹ��·ADֵ��
 *                                  ���� C+ P-������·ADֵ��Ч�����ݸ�ֵ�жϣ���������C+ B-��ѹ��·ADֵ�жϡ�
 * V1.3     ������             2018-12-05  У׼�������ѹ����ADֵ��
 * V1.3     ������             2018-12-05  �޸ĳ�����Ƴ��жϣ����ڵ�ѹ����26V��Ϊ��������룬���ڵ�ѹ����VBAT��ѹ����Ϊ������Ƴ���
 * V1.3     ������             2018-12-05  �޸������߼��жϣ�����ѹ�������ʾSOCû�и��µ�100%���������5Sÿ1%�����ʸ��£����µ�100%���ٿ�ʼ����5min��
 * V1.3     ������             2018-12-05  �����¶Ȳ���BUG���л�ADC������׼ʱ��Ҫ������ADC10CTL0�Ĵ������������û�׼��ѹ��
 * V1.3     ������             2018-12-06  �޸ĳ������λ�ж��߼�������100ms��ʱ���������λ����100ms����Ϊ�������λ����ʼ�ж��Ƿ�������ѹ����ֹ��ѹ���С�
 * V1.3     ������             2018-12-06  �޸���������ֵ����֤0%֮�󻹿��Էŵ�5S��CapProc.c�ļ���210�У�ʣ�������㷨��2750��Ϊ2932��
 * V1.3     ������             2018-12-06  ȡ���洢�ŵ�Ƿѹ��־��
 * V1.3     ������             2018-12-06  �޸������߼���û�з�����ѹʱSOCӦ�ñ�����99%����ѹʱ��Ϊ100%��
 *
 ---------------------------------------*/

#include"Global.h"
#include"string.h"

/*
 * main.c
 */

const uint16_t version = 0x0004;    // �汾�ţ�0x0001��Ӧ�汾V1.0

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
    {   // һ��ѭ��һ�εĻ���5000��ѭ���൱��13���
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

void charge_judge(void)  //�������λ�ж�
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
            Vchg_volt();    // ��ȡVCHG��ѹֵ
            if((g_sys_signal.val.vchg_ad_cp > VCHG_CP_LOW)||(g_sys_signal.val.vchg_ad_cb > VCHG_CB_LOW))
            {   // VCHG_CP ���� C+ P-������·ADֵ���ޣ�����VCHG_CB ���� C+ B-��ѹ��·ADֵ���ޣ�����Ϊ�������λ
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

                            g_sys_state.val.dch_on_flag = TRUE;     // ���������������ŵ� blust 20181106
                            uart_continue_cnt = sys_10ms_cnt;
                            P2IE &= ~BIT2;      // ��ֹTRIG�ж�
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
    {   // TRIG�����ʱ�����
        if((uint8_t)(sys_10ms_cnt - trig_on_delay_cnt) >= 6)
        {
            trig_on_delay_cnt = sys_10ms_cnt;
            g_sys_state.val.trig_on_flag = FALSE;
            g_sys_state.val.dch_on_flag = TRUE;     // ����ŵ�
            uart_continue_cnt = sys_10ms_cnt;
        }
    }
    else
    {
        trig_on_delay_cnt = sys_10ms_cnt;
    }

    if((g_sys_state.val.charger_on_flag == FALSE)&&(g_sys_state.val.dch_on_flag == FALSE)&&(g_sys_state.val.trig_on_flag == FALSE))
    {   //�����δ��λ �� �����ڷŵ�����״̬
        if(g_sys_state.val.trig_isr_flag == FALSE)
        {
            delay_ms(50);
            P2IFG &= ~BIT2;         //���IO���жϱ�־
            P2IE  |= BIT2;      // TRIG�ж�ʹ��
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
            asm("  MOV &0xFFFE,PC;");//��PCת�Ƶ�Ӧ�ó���ĸ�λ������..���û���ʱ��ǵ�ǰ���пո񣡣���
        }
    }
    BCSCTL1 = CALBC1_8MHZ;                    					// Set DCO   Ĭ��MCLKΪDCO
    DCOCTL = CALDCO_8MHZ;

    __disable_interrupt();
    InitIO();
    I2CInitialise();
    Flash_init();
    __enable_interrupt();   // �����ж�
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
    Vbat_volt();    // ��ȡVBAT��ѹֵ
    Vchg_volt();    // ��ȡVCHG��ѹֵ
    Temp_Get();     // ��ȡ�¶�ֵ
    if((g_sys_signal.val.vchg_ad_cp > VCHG_CP_LOW)||(g_sys_signal.val.vchg_ad_cb > VCHG_CB_LOW))
    {   // VCHG_CP ���� C+ P-������·ADֵ���ޣ�����VCHG_CB ���� C+ B-��ѹ��·ADֵ���ޣ�����Ϊ�������λ
//        g_sys_state.val.charger_on_flag = TRUE;
        g_sys_state.val.chg_on_checking_flag = TRUE;

//        if((g_sys_signal.val.vchg_ad_cp > VCHG_CP_HIGH)||(g_sys_signal.val.vchg_ad_cb > VCHG_CB_HIGH))
//        {   // VCHG_CP ���� C+ P-������·ADֵ���ޣ�����VCHG_CB ���� C+ B-��ѹ��·ADֵ���ޣ�����Ϊ�������ѹ
//            g_sys_flags.val.chg_val_high_err_flag = TRUE;
//        }
    }
   // ֻҪ����ϵͳ���ʹ�������ŵ�״̬ blust 20181106
    g_sys_state.val.dch_on_flag = TRUE;
    uart_continue_cnt = sys_10ms_cnt;

    DIS_VBAT_CHK();     // ��ֹVbat���

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
    P2IFG &= ~BIT2;			//���IO���жϱ�־

    g_sys_state.val.trig_on_flag = TRUE;     // TRIG��λ
    P2IE &= ~BIT2;      // ��ֹTRIG�ж�
    g_sys_state.val.trig_isr_flag = FALSE;
}

//#pragma vector=WDT_VECTOR       //���Ź��ж����
//interrupt void WDT(void)
//{
//    IFG1 &= ~WDTIFG;            //������Ź��жϱ�־
//    _bic_SR_register_on_exit(LPM3_bits);// �˳��͹���ģʽ
//}
