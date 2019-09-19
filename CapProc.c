/*
 * CapProc.c
 *
 *  Created on: 2015-5-28
 *      Author: Administrator
 *
 */

#include "CapProc.h"
#include "SampProc.H"
#include "Flash.H"
#include "uart.h"

volatile uint8_t old_show_cap_rate = 1;
volatile uint8_t old_re_cap_rate = 1;

uint8_t cap_write_cnt = 0;

volatile __SYS_CAP    g_sys_cap;

// ------------------------------------------
// �ֲ�����
// ------------------------------------------
// 101��������
//const uint16_t  Open_Cell_Valtage[101]=
//{
// 2700, 2824, 2922, 2997, 3057, 3106, 3146, 3185, 3226, 3264,
// 3298, 3330, 3359, 3384, 3412, 3431, 3438, 3447, 3454, 3460,
// 3466, 3473, 3482, 3495, 3508, 3521, 3533, 3543, 3552, 3561,
// 3569, 3577, 3584, 3590, 3595, 3600, 3605, 3610, 3615, 3620,
// 3625, 3630, 3636, 3641, 3647, 3653, 3659, 3665, 3672, 3679,
// 3686, 3694, 3702, 3710, 3718, 3727, 3736, 3745, 3755, 3764,
// 3775, 3785, 3796, 3807, 3819, 3830, 3841, 3852, 3861, 3871,
// 3879, 3887, 3895, 3903, 3911, 3919, 3929, 3938, 3948, 3959,
// 3970, 3982, 3993, 4005, 4015, 4026, 4035, 4044, 4051, 4057,
// 4060, 4063, 4066, 4069, 4073, 4078, 4086, 4096, 4110, 4128,
// 4150 };//samsung 25R

// ������������101��������
const uint16_t  Open_Cell_Valtage[101]=
{
 3395, 3419, 3441, 3456, 3464, 3471, 3477, 3483, 3490, 3497,
 3504, 3512, 3520, 3530, 3539, 3548, 3558, 3566, 3574, 3582,
 3591, 3598, 3605, 3611, 3615, 3619, 3623, 3626, 3630, 3633,
 3637, 3640, 3644, 3648, 3651, 3655, 3659, 3663, 3667, 3671,
 3675, 3680, 3685, 3690, 3695, 3700, 3706, 3712, 3718, 3725,
 3732, 3740, 3747, 3756, 3765, 3775, 3784, 3793, 3802, 3810,
 3818, 3826, 3834, 3842, 3850, 3858, 3866, 3874, 3882, 3890,
 3898, 3906, 3913, 3920, 3927, 3933, 3940, 3947, 3955, 3963,
 3971, 3980, 3990, 4000, 4010, 4021, 4032, 4043, 4054, 4064,
 4075, 4085, 4094, 4102, 4107, 4112, 4117, 4122, 4126, 4132,
 4139 };//samsung 25R


//read the soc from the flash
void InitSoc()
{
    uint8_t i = 0;
    uint16_t soc_temp = 0;

    for(i=0;i<32;i++)
    {
        if(((Flash_Read_2Byte((unsigned int *) SegmentBStart,i)==0xFFFF)&&(i>0)) || (i==31))
        {
            if((i==31)&&(Flash_Read_2Byte((unsigned int *) SegmentBStart,i) != 0xFFFF))
            {
                soc_temp = Flash_Read_2Byte((unsigned int *) SegmentBStart,i);
                cap_write_cnt = 32;
            }
            else
            {
                soc_temp = Flash_Read_2Byte((unsigned int *) SegmentBStart,i-1);
                cap_write_cnt = i;
            }

            g_sys_cap.val.show_cap_rate = soc_temp / 640;
            g_sys_cap.val.cap_val = ((soc_temp % 640) * 4);
            g_sys_cap.val.cap_val = -g_sys_cap.val.cap_val;
            g_sys_cap.val.re_cap_rate = (uint32_t)(g_sys_cap.val.full_cap + g_sys_cap.val.cap_val) * 100 / g_sys_cap.val.full_cap;

            g_cap_flags.val.first_cap_update_flag = TRUE;

            //new ljc 5.10
            if(g_sys_cap.val.show_cap_rate > 100)
            {
                g_cap_flags.val.first_cap_update_flag = FALSE;
                g_sys_cap.val.show_cap_rate = 21;
                g_sys_cap.val.re_cap_rate = 21;
                g_sys_cap.val.cap_val = -1920;
            }
            //end
            old_show_cap_rate = g_sys_cap.val.show_cap_rate;
            old_re_cap_rate = g_sys_cap.val.re_cap_rate;
            break;
        }
        if(Flash_Read_2Byte((unsigned int *) SegmentBStart,i)==0xFFFF && (i==0))
        {
            cap_write_cnt = 0;
            break;
        }
    }
}

int8_t Get_Diff_of_SOC(void)
{
    static uint8_t update_cap_cnt = 0;
    static uint8_t follow_cap_cnt = 0;
    int8_t soc_show_diff = 0;

    if(g_sys_state.val.chg_state_flag == TRUE)
    {
        if(g_sys_cap.val.re_cap_rate > old_re_cap_rate)
        {
            if((uint8_t)(sys_250ms_cnt - update_cap_cnt) > 40)   // 10S������SOCs�仯1% ���ʱ
            {   // �г�����ʱ��SOCֻ������µ�99%����ѹ���Ϊ100%�� blust 20181206
                if((g_sys_cap.val.re_cap_rate > g_sys_cap.val.show_cap_rate)&&(g_sys_cap.val.show_cap_rate < 99))
                {
                    soc_show_diff = 1;
                    old_re_cap_rate = g_sys_cap.val.re_cap_rate;
                    update_cap_cnt = sys_250ms_cnt;
                }
            }
        }
        else if((g_sys_cap.val.re_cap_rate > g_sys_cap.val.show_cap_rate)&&(g_sys_cap.val.show_cap_rate < 99))
        {
            if((uint8_t)(sys_250ms_cnt - follow_cap_cnt) > 100)  // 25S����һ�β�ֵ ���ʱ
            {
                if((uint8_t)(sys_250ms_cnt - update_cap_cnt) > 40)   // 10S������SOCs�仯1% ���ʱ
                {
                    soc_show_diff = 1;
                    old_re_cap_rate = g_sys_cap.val.re_cap_rate;
                    update_cap_cnt = sys_250ms_cnt;
                    follow_cap_cnt = sys_250ms_cnt;
                }
            }
        }
    }
    else if((g_sys_state.val.vbat_chg_full_flag == TRUE)&&(g_sys_state.val.charger_on_flag == TRUE))
    {   // �Ѿ��������ҳ������λ������ʾSOC����µ�100%
        if((uint8_t)(sys_250ms_cnt - update_cap_cnt) > 20)   // 5S������SOCs�仯1% ���ʱ
        {
            if(g_sys_cap.val.re_cap_rate > g_sys_cap.val.show_cap_rate)
            {
                soc_show_diff = 1;
                old_re_cap_rate = g_sys_cap.val.re_cap_rate;
                update_cap_cnt = sys_250ms_cnt;
            }
        }
    }
    else if(g_sys_state.val.dch_state_flag == TRUE)
    {
        if(g_sys_cap.val.re_cap_rate < old_re_cap_rate)
        {
            if((uint8_t)(sys_250ms_cnt - update_cap_cnt) > 8)   // 2S������SOCs�仯1% �ŵ�ʱ
            {
                if(g_sys_cap.val.re_cap_rate < g_sys_cap.val.show_cap_rate)
                {
                    soc_show_diff = -1;
                    old_re_cap_rate = g_sys_cap.val.re_cap_rate;
                    update_cap_cnt = sys_250ms_cnt;
                }
            }
        }
        else if(g_sys_cap.val.re_cap_rate < g_sys_cap.val.show_cap_rate)
        {   // ������ŵ�ʱ�����仯8S����ȡ������2S����һ�α仯��
            if(g_sys_signal.val.cur < -10000)
            {
                if((uint8_t)(sys_250ms_cnt - update_cap_cnt) > 8)   // 2S������SOCs�仯1% �ŵ�ʱ
                {
                    soc_show_diff = -1;
                    old_re_cap_rate = g_sys_cap.val.re_cap_rate;
                    update_cap_cnt = sys_250ms_cnt;
                    follow_cap_cnt = sys_250ms_cnt;
                }
            }
            else if((uint8_t)(sys_250ms_cnt - follow_cap_cnt) > 32)  // 8S����һ�β�ֵ �ŵ�ʱ
            {
                if((uint8_t)(sys_250ms_cnt - update_cap_cnt) > 8)   // 2S������SOCs�仯1% �ŵ�ʱ
                {
                    soc_show_diff = -1;
                    old_re_cap_rate = g_sys_cap.val.re_cap_rate;
                    update_cap_cnt = sys_250ms_cnt;
                    follow_cap_cnt = sys_250ms_cnt;
                }
            }
        }
    }
    else
    {
        update_cap_cnt = sys_250ms_cnt;
        follow_cap_cnt = sys_250ms_cnt;
        soc_show_diff = 0;
    }

    return soc_show_diff;
}

uint16_t Left_CAP_Get(void)
{
    uint16_t left_cap = 0;
    uint16_t left_val = 0;
    int32_t temp = 0;

    temp = 422 - g_sys_signal.val.temperature * 4 / 5;
    if(g_sys_signal.val.temperature > 200)
    {
        temp = 262;
    }
    temp = g_sys_signal.val.cur * temp / 10000;
    left_val = 2932 - temp;
    left_cap = VbatToSoc(left_val, Open_Cell_Valtage);

    return left_cap;
}

/*CapProc  cur
 * ���������㷨
 *
 */
void Cap_Proc(signed long cur)
{
    int tmp_cap = 0;
    /*
     * g_sys_cap
     * tmp_cap  :tmp_cap=(int)(g_sys_cap.val.cap_cnt/CAP_CNT_VAL);  ���ɳɺ���ʱ
     */
    long x = -(long)(g_sys_cap.val.full_cap + 150);
    g_sys_cap.val.cap_cnt += cur;
    tmp_cap = (int)(g_sys_cap.val.cap_cnt / CAP_CNT_VAL);
    g_sys_cap.val.cap_cnt = g_sys_cap.val.cap_cnt - (long)tmp_cap * CAP_CNT_VAL;  //��������

    if(tmp_cap>-35)
    {
        g_sys_cap.val.cap_val += tmp_cap;
        if(g_sys_state.val.chg_state_flag == TRUE)
        {
//            g_sys_cap.val.cap_val2 += tmp_cap;  //�������¼���
            g_sys_cap.val.cycle_cap_val += tmp_cap;  //ѭ��������������
        }
//        if(g_sys_state.val.dch_state_flag == TRUE)
//        {
//            g_sys_cap.val.cycle_dch_cap_val -= tmp_cap;
//        }
    }

    if(g_sys_cap.val.cap_val< x) //�ҵ�ʣ���ǩ���
    {
        g_sys_cap.val.cap_val = x;
    }
    else if(g_sys_cap.val.cap_val > 0)
    {
        g_sys_cap.val.cap_val = 0;
    }
    //end new
}

void NormalCapacityProc(void)
{
    static uint8_t cap_250ms_tick = 0;
    static uint8_t cap_revise_tick = 0;

    //new by LJC
    uint16_t less_full_cap = g_sys_cap.val.full_cap - 200;   //�������µ���Сֵ,�Ӽ�ֵ�������������ʵ��ĵ�����һ��Ϊ10%
    uint16_t more_full_cap = g_sys_cap.val.full_cap + 200;   //�������µ����ֵ
    //end new

    uint16_t capacity_ad;
    uint16_t left_cap = 0;
    int16_t temp_cap_cal = 0;

    /*start VbatToSoc
     * ���� ����ֵ   ��ѹƽ��ֵ ��cell ��Ա�  100 ���±�
     */
    if((uint8_t)(sys_250ms_cnt - cap_250ms_tick) >= 1)
    {
        cap_250ms_tick = sys_250ms_cnt;

        if(g_cap_flags.val.first_cap_update_flag == FALSE)
        {   // �״θ���
            capacity_ad = g_sys_signal.val.vbat / CELLS_NUM;
            g_sys_cap.val.cap_val = VbatToSoc(capacity_ad, Open_Cell_Valtage);
            left_cap = Left_CAP_Get();
            g_sys_cap.val.cap_val += left_cap;
            g_sys_cap.val.show_cap_rate = g_sys_cap.val.cap_val * 100 / g_sys_cap.val.full_cap;
            g_sys_cap.val.re_cap_rate = g_sys_cap.val.show_cap_rate;
            g_sys_cap.val.cap_val = g_sys_cap.val.cap_val - g_sys_cap.val.full_cap;

            old_show_cap_rate = g_sys_cap.val.show_cap_rate;
            old_re_cap_rate = g_sys_cap.val.re_cap_rate;
            g_cap_flags.val.first_cap_update_flag = TRUE;
        }

        Cap_Proc(g_sys_signal.val.cur);     // �����ظ���
        /*
         * �����ٷֱȸ��´������
         */
        if(g_sys_state.val.chg_state_flag == TRUE)
        {   // ���ʱ�������㣬������left_capֵ��ֻ����ʵ�ʷų�����ֵ��
            temp_cap_cal = g_sys_cap.val.full_cap + g_sys_cap.val.cap_val;
            if(temp_cap_cal < 0)
            {
                temp_cap_cal = 0;
            }
            g_sys_cap.val.re_cap_rate = (uint32_t)(temp_cap_cal) * 100 / g_sys_cap.val.full_cap;
        }
        else
        {   // �ŵ�ʱ����ʱ�������㣬������Ϊ�������赼��ѹ�����Ų��������������۵��ò��֡�
            left_cap = Left_CAP_Get();
            temp_cap_cal = g_sys_cap.val.full_cap + g_sys_cap.val.cap_val - left_cap;
            if(temp_cap_cal < 0)
            {
                temp_cap_cal = 0;
            }
            g_sys_cap.val.re_cap_rate = (uint32_t)(temp_cap_cal) * 100 / g_sys_cap.val.full_cap;
        }
        g_sys_cap.val.show_cap_rate += Get_Diff_of_SOC();
        if((g_sys_flags.val.soft_vcell_low_protect_flag == TRUE)||(g_sys_flags.val.vcell_low_protect_flag == TRUE))
        {   // ����Ƿѹ��������ʾSOC����
            g_sys_cap.val.show_cap_rate = 0;
        }

        /*
         * ���� soc   test_flash_soc
         */
        if(
           ((old_show_cap_rate > g_sys_cap.val.show_cap_rate)&&(old_show_cap_rate - g_sys_cap.val.show_cap_rate >= 2))
         ||((g_sys_cap.val.show_cap_rate > old_show_cap_rate)&&(g_sys_cap.val.show_cap_rate - old_show_cap_rate >= 2))
         )
        {
            Write_CAP_Info();
            old_show_cap_rate = g_sys_cap.val.show_cap_rate;
        }

        if((g_sys_cap.val.show_cap_rate <= 97)&&(g_sys_state.val.charger_on_flag == FALSE))
        {   // �����δ��λ��SOCֵС�ڵ���97%������䱥��־λ blust 20181114
            g_sys_state.val.vbat_chg_full_flag = FALSE;
        }

        BatCycleProc();

        // ��������������
        if((g_sys_flags.val.soft_vcell_high_protect_flag == TRUE)||(g_sys_flags.val.vcell_high_protect_flag == TRUE))     // Ӳ����ѹ�������ѹ
        {
            if((uint8_t)(sys_250ms_cnt - cap_revise_tick) >= 12)
            {
                cap_revise_tick = sys_250ms_cnt;
                if((g_sys_signal.val.max_cell_volt > 4100)&&(g_cap_flags.val.chg_cap_revise_flag == FALSE))
                {
                    g_cap_flags.val.chg_cap_revise_flag = TRUE;
                    g_sys_cap.val.cap_val = 0;
                }
            }
        }
        else
        {
            cap_revise_tick = sys_250ms_cnt;
        }
    }
}

void BatCycleProc(void)
{
    if(g_sys_state.val.chg_state_flag == TRUE)
    {
        if(g_sys_cap.val.cycle_cap_val >= g_sys_cap.val.full_cap)
        {
            g_sys_cap.val.cycle_cap_val = 0;
            g_sys_cap.val.bat_cycle_cnt ++;
//            Write_Other_Info();
        }
    }
}

void Cap_Revise(void)
{
    uint16_t capacity_ad;

    capacity_ad = g_sys_signal.val.vbat / CELLS_NUM;
    g_sys_cap.val.cap_val = VbatToSoc(capacity_ad, Open_Cell_Valtage);
    g_sys_cap.val.cap_val = g_sys_cap.val.cap_val - g_sys_cap.val.full_cap;
}

uint16_t VbatToSoc(uint16_t vbat_val, const uint16_t *p)
{
    uint8_t low = 0;
    uint8_t high = 101;    //�޸�
    uint8_t mid;
    uint16_t re_value = 0;
    while(low < high)
    {
        mid = (low + high) >> 1;
        if(vbat_val == *(p+ mid))
        {break;}    // �������պ���ȵ�ֵ�������Ϸ���
        if(high - low == 1)
        {
            mid = low;                 // ���ڲ��Ǿ�ȷ���ң�����С�����ڣ�ȡСֵ
            break;
        }
        if(vbat_val < *(p + mid))
        { high = mid;}
        else
        {low = mid;}
    }

    if(mid == 100)
    {
        re_value = g_sys_cap.val.full_cap;
    }
    else if(mid == 0)
    {
        re_value = 0;
    }
    else
    {
        re_value = (uint32_t)mid * g_sys_cap.val.full_cap / 100
                   + g_sys_cap.val.full_cap * (uint32_t)(vbat_val - *(p+mid)) / ((*(p+mid+1) - *(p+mid)) * 100);
    }

    return re_value;
}

