#ifndef __MAIN_H__
#define __MAIN_H__

#define ALLOCATE_EXTERN
#include "HC89S003AF4.h"
#include "soft_timer.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "uart_Ringbuf.h"

#define VDD 4.0                     // ADC参考电压
#define UART1BUFLEN 60  //串口发送缓存区长度
#define VolCoeFlashAddress 0x3B00 //电压系数存储的Flash地址

#define FUNCCALLBACK_NULL		((_FUNCCALLBACK*)0)

typedef struct
{
    u8 CMD;
    void (*callback_func)(unsigned char *myStr);
} _FUNCCALLBACK;
/************************************ 函数申明 ****************************************/
void Delay_2us(unsigned int fui_i);  // 延时函数
//char putchar(char guc_Uartbuf);      // printf重定向


//功能函数
void callback_QueryVoltage(unsigned char *myStr);
void callback_ModifyVoltageParameters(unsigned char *myStr);
void callback_ModifyPWMParameter(unsigned char *myStr);
void callback_PWMOperatingParameters(unsigned char *myStr);
void callback_SpecifyWaveform(unsigned char *myStr);
void poll_task(u8 cmd);

// 系统初始化
void SystemInit(void);

void IOInitPushpull(unsigned char IOPPx, unsigned char IOPPXx);

// 串口
void usart1_timeover_proc(void);

// FLASH
void FLASH_WriteData(unsigned char fuc_SaveData, unsigned int fui_Address);
void Flash_WriteArr(unsigned int fui_Address, unsigned char fuc_Length, unsigned char* fucp_SaveArr);
void Flash_ReadArr(unsigned int fui_Address, unsigned char fuc_Length, unsigned char* fucp_SaveArr);
void Flash_EraseBlock(unsigned int fui_Address);

/************************************ 变量定义 ****************************************/

unsigned char i;  // 计数专用

// ADC
unsigned int  gui_AdcValue_a[4] = {0x00};
unsigned char guc_AdcChannel_a[4]   = {0x00, 0x02, 0x04, 0x06};  // ADC通道号
unsigned char guc_Channel_Count = 0, guc_Count = 0;              // guc_Channel_Count - 通道 guc_Count - 次数

// 串口
struct soft_timer usart1_timer;
unsigned char     guc_Uartbuf_p[UART1BUFLEN]    = 0;    // 用于存放串口发送的数据
//Uart_Struct       Uart1;
UartRec_Struct    Uart1_rec;
RingBuff_t ringBuff;//创建一个ringBuff的缓冲区

// 电压系数
float VolCoe[4] = 0;  // 电压系数
FLOAT Vol_a, Vol_b;

// FLASH
unsigned char VolCoeStore[16] = 0;  // 电压系数缓存区

// PWM
unsigned int PWMOutCycle = 1000, PWMWorkTime = 500;  // PWMOutCycle - PWM输出周期   PWMWorkTime - 工作时间
unsigned int PWMOut_count = 0;                       // PWMOut_count - PWM输出时间计数

// 指定波形
const unsigned char SpecifywaveformOut[14] = {1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1};  // 指定波形
unsigned char       SpecifywaveCount       = 0;                                           // 指定波形输出计数
unsigned char       SpecifywaveIOflag      = 0;  // 指定波形输出管脚标志 0-P01  1-P05  2-P07


_FUNCCALLBACK callback_list[] =
{
    {0x01,callback_QueryVoltage},
    {0x02,callback_ModifyVoltageParameters},
    {0x03,callback_ModifyPWMParameter},
    {0x04,callback_PWMOperatingParameters},
    {0x05,callback_SpecifyWaveform},
};

#endif /*__SOFT_TIMER_H__*/