/*
 * until.c
 *
 *  Created on: 2018��1��29��
 *      Author: zhao
 */

#include "until.h"

/*--------------------------------------------------------------
 ð�����򷨣���С����
��� ȥ�������������С��,ȡ�м�ֵ
 --------------------------------------------------------------*/
unsigned int average_data(unsigned int *data_arr)
{
    unsigned char i, j;
     unsigned int temp;
    for(i = 0; i < 9; i ++)    //ð�ݷ�����
     for(j = 0; j < 9-i; j ++)
     {
         if(data_arr[j] > data_arr[j + 1])
         {
            temp = data_arr[j] ;
            data_arr[j] = data_arr[j + 1] ;
            data_arr[j + 1] = temp ;
         }
     }

    return (data_arr[5]);//ȡ�м�ֵ
}




