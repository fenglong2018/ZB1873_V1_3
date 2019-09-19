/*
 * until.c
 *
 *  Created on: 2018年1月29日
 *      Author: zhao
 */

#include "until.h"

/*--------------------------------------------------------------
 冒泡排序法（从小到大）
结果 去掉两个大的两个小的,取中间值
 --------------------------------------------------------------*/
unsigned int average_data(unsigned int *data_arr)
{
    unsigned char i, j;
     unsigned int temp;
    for(i = 0; i < 9; i ++)    //冒泡法排序
     for(j = 0; j < 9-i; j ++)
     {
         if(data_arr[j] > data_arr[j + 1])
         {
            temp = data_arr[j] ;
            data_arr[j] = data_arr[j + 1] ;
            data_arr[j + 1] = temp ;
         }
     }

    return (data_arr[5]);//取中间值
}




