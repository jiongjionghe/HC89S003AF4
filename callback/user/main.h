#ifndef __MAIN_H__
#define __MAIN_H__

#define ALLOCATE_EXTERN
#include "HC89S003AF4.h"
#include "soft_timer.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "uart_Ringbuf.h"

#define VDD 4.0                     // ADC�ο���ѹ
#define UART1BUFLEN 60  //���ڷ��ͻ���������
#define VolCoeFlashAddress 0x3B00 //��ѹϵ���洢��Flash��ַ

#define FUNCCALLBACK_NULL		((_FUNCCALLBACK*)0)

typedef struct
{
    u8 CMD;
    void (*callback_func)(unsigned char *myStr);
} _FUNCCALLBACK;
/************************************ �������� ****************************************/
void Delay_2us(unsigned int fui_i);  // ��ʱ����
//char putchar(char guc_Uartbuf);      // printf�ض���


//���ܺ���
void callback_QueryVoltage(unsigned char *myStr);
void callback_ModifyVoltageParameters(unsigned char *myStr);
void callback_ModifyPWMParameter(unsigned char *myStr);
void callback_PWMOperatingParameters(unsigned char *myStr);
void callback_SpecifyWaveform(unsigned char *myStr);
void poll_task(u8 cmd);

// ϵͳ��ʼ��
void SystemInit(void);

void IOInitPushpull(unsigned char IOPPx, unsigned char IOPPXx);

// ����
void usart1_timeover_proc(void);

// FLASH
void FLASH_WriteData(unsigned char fuc_SaveData, unsigned int fui_Address);
void Flash_WriteArr(unsigned int fui_Address, unsigned char fuc_Length, unsigned char* fucp_SaveArr);
void Flash_ReadArr(unsigned int fui_Address, unsigned char fuc_Length, unsigned char* fucp_SaveArr);
void Flash_EraseBlock(unsigned int fui_Address);

/************************************ �������� ****************************************/

unsigned char i;  // ����ר��

// ADC
unsigned int  gui_AdcValue_a[4] = {0x00};
unsigned char guc_AdcChannel_a[4]   = {0x00, 0x02, 0x04, 0x06};  // ADCͨ����
unsigned char guc_Channel_Count = 0, guc_Count = 0;              // guc_Channel_Count - ͨ�� guc_Count - ����

// ����
struct soft_timer usart1_timer;
unsigned char     guc_Uartbuf_p[UART1BUFLEN]    = 0;    // ���ڴ�Ŵ��ڷ��͵�����
//Uart_Struct       Uart1;
UartRec_Struct    Uart1_rec;
RingBuff_t ringBuff;//����һ��ringBuff�Ļ�����

// ��ѹϵ��
float VolCoe[4] = 0;  // ��ѹϵ��
FLOAT Vol_a, Vol_b;

// FLASH
unsigned char VolCoeStore[16] = 0;  // ��ѹϵ��������

// PWM
unsigned int PWMOutCycle = 1000, PWMWorkTime = 500;  // PWMOutCycle - PWM�������   PWMWorkTime - ����ʱ��
unsigned int PWMOut_count = 0;                       // PWMOut_count - PWM���ʱ�����

// ָ������
const unsigned char SpecifywaveformOut[14] = {1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1};  // ָ������
unsigned char       SpecifywaveCount       = 0;                                           // ָ�������������
unsigned char       SpecifywaveIOflag      = 0;  // ָ����������ܽű�־ 0-P01  1-P05  2-P07


_FUNCCALLBACK callback_list[] =
{
    {0x01,callback_QueryVoltage},
    {0x02,callback_ModifyVoltageParameters},
    {0x03,callback_ModifyPWMParameter},
    {0x04,callback_PWMOperatingParameters},
    {0x05,callback_SpecifyWaveform},
};

#endif /*__SOFT_TIMER_H__*/