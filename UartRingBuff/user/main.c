#define ALLOCATE_EXTERN

#include "HC89S003AF4.h"
#include "soft_timer.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "uart.h"

#define VDD 4.0                   // ADC参考电压
#define UART1BUFLEN 100           //串口发送缓存区长度
#define VolCoeFlashAddress 0x3B00 //电压系数存储的Flash地址

RingBuff_t ringBuff;//创建一个ringBuff的缓冲区

/************************************ 函数申明 *****************************************/
void Delay_2us(unsigned int fui_i); // 延时函数
// char putchar(char guc_Uartbuf);      // printf重定向

// 系统初始化
void SystemInit(void);

void IOInitPushpull(unsigned char IOPPx, unsigned char IOPPXx);

// 串口
void usart1_timeover_proc(void);

// FLASH
void FLASH_WriteData(unsigned char fuc_SaveData, unsigned int fui_Address);
void Flash_WriteArr(unsigned int fui_Address, unsigned char fuc_Length,unsigned char *fucp_SaveArr);
void Flash_ReadArr(unsigned int fui_Address, unsigned char fuc_Length,unsigned char *fucp_SaveArr);
void Flash_EraseBlock(unsigned int fui_Address);

/************************************ 变量定义 *****************************************/
// ADC
unsigned int gui_AdcValue_a[4] = {0x00};
unsigned char guc_AdcChannel_a[4] = {0x00, 0x02, 0x04, 0x06}; // ADC通道号
unsigned char guc_Channel_Count = 0,
              guc_Count = 0; // guc_Channel_Count - 通道 guc_Count - 次数

// 串口
struct soft_timer usart1_timer;
unsigned char guc_Uartbuf_p[UART1BUFLEN] = 0; // 用于存放串口发送的数据
UartRec_Struct Uart1_rec;

// 电压系数
float VolCoe[4] = 0; // 电压系数
FLOAT Vol_a, Vol_b;

// FLASH
unsigned char VolCoeStore[16] = 0; // 电压系数缓存区

// PWM
unsigned int PWMOutCycle = 1000,
             PWMWorkTime = 500; // PWMOutCycle - PWM输出周期   PWMWorkTime - 工作时间
unsigned int PWMOut_count = 0; // PWMOut_count - PWM输出时间计数

// 指定波形
const unsigned char SpecifywaveformOut[14] = {1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1
                                             }; // 指定波形
unsigned char SpecifywaveCount = 0; // 指定波形输出计数
unsigned char SpecifywaveIOflag = 0; // 指定波形输出管脚标志 0-P01  1-P05  2-P07
/***************************************************************************************
  * @实现效果	阶段三
定时器0： 1ms
定时器1:  25ms

ADC通道：
AN0 - P00
AN2 - P02
AN4 - P04
AN6 - P06

串口：
RXD - P21
TXD - P03

PWM:
PWM0  - P20 P22 P23 P24 P25 P26 P27

指定波形:
P01 P05 P07
***************************************************************************************/
void main() {
    float UARTprintVolCoe = 0;           // 电压缓存区
    unsigned char IOPPx = 0, IOPPXx = 0; // IOPPx - 推完输出管脚是Px组，IOPPXx -
    // 推完输出管脚是PX组的x管脚
    unsigned int PWMCycle = 0,PWMDutycycle = 0; // PWMCycle - 存放PWM波的周期  PWMDutycycle -
    // 存放PWM占空比的具体数值
    unsigned long int PWMFrequency = 0; // PWM频率
    unsigned char ANx = 0;
    unsigned char i; // 计数专用

    SystemInit();
    

    // 读取Flash中的各通道电压系数
    Flash_ReadArr(VolCoeFlashAddress, 16, VolCoeStore);
    for (i = 0; i < 4; i++) {
        memcpy(Vol_b.stByte.u8Byte, &VolCoeStore[i * 4], 4);
        VolCoe[i] = Vol_b.all;
    }

    // 串口超时判断
    soft_timer_list_reset();
    add_timer(&usart1_timer, usart1_timeover_proc, 50); // 50MS
    start_timer(&usart1_timer);

    // 清空串口接收缓存
    memcpy(Uart1_rec.guc_Uartbuf_a, 0, 20);

    RingBuff_Init(&ringBuff);
    ringBuff.FlagByte |= 0x01; //串口空闲
    memcpy(ringBuff.Ring_data, 0, RINGBUFF_LEN);
    
    
    
    while (1) 
    {
        WDTC |= 0x10; // 清狗

        if (Uart1_rec.guc_Uartflag) 
        {
            memcpy(guc_Uartbuf_p, 0, UART1BUFLEN);
            // 功能选择
            switch (Uart1_rec.guc_Uartbuf_a[0]) 
            {
                /*********************** 1 查询通道电压 ***********************/
                // 管脚: P00 P02 P04 P06
                // 缓存区数据：0x01，通道选择（1个字节）
                case 0x01:
                    if (Uart1_rec.guc_Uartbuf_a[1] > 6 || Uart1_rec.guc_Uartbuf_a[1] % 2 == 1) 
                    {
                        sprintf(guc_Uartbuf_p, "该通道不存在！\r\n");
                        break;
                    }
                    ANx = (int)Uart1_rec.guc_Uartbuf_a[1] / 2;
                    UARTprintVolCoe = (float)(gui_AdcValue_a[ANx] / 4096.0) * VDD * VolCoe[ANx];
                    sprintf(guc_Uartbuf_p, "AN%d = %f\r\n", (int)Uart1_rec.guc_Uartbuf_a[1], UARTprintVolCoe);
                    break;

                /*********************** 2 修改指定通道电压参数 ***********************/
                // 管脚: P00 P02 P04 P06
                // 缓存区数据：0x02，通道选择（1个字节），电压系数（4个字节，float）
                case 0x02:
                    if (Uart1_rec.guc_Uartbuf_a[1] == 0x00 ||
                        Uart1_rec.guc_Uartbuf_a[1] == 0x02 ||
                        Uart1_rec.guc_Uartbuf_a[1] == 0x04 ||
                        Uart1_rec.guc_Uartbuf_a[1] == 0x06) 
                    {
                        memcpy(Vol_a.stByte.u8Byte, &Uart1_rec.guc_Uartbuf_a[2], 4);
                        VolCoe[(int)Uart1_rec.guc_Uartbuf_a[1] / 2] = Vol_a.all;

                        for (i = 0; i < 4; i++) {
                            Vol_b.all = VolCoe[i];
                            memcpy(&VolCoeStore[i * 4], Vol_b.stByte.u8Byte, 4);
                        }
                        Flash_EraseBlock(VolCoeFlashAddress); // 擦除地址0x3B00所在扇区
                        Flash_WriteArr(VolCoeFlashAddress, 16,VolCoeStore); // 在地址0x3B00写入数据
                        sprintf(guc_Uartbuf_p, "ok!\r\nVolCoe: %f - %f - %f - %f\r\n",VolCoe[0], VolCoe[1], VolCoe[2], VolCoe[3]);
                    } 
                    else 
                    {
                        sprintf(guc_Uartbuf_p, "设置错误!\r\n");
                    }
                    break;

                /*********************** 3 指定PWM波实现管脚，频率和占空比 ************************/
                // 管脚: P20 P22 P23 P24 P25 P26 P27    频率：489 - 2M   占空比：0 - 100
                // 缓存区数据：0x03，管脚选择（1个字节），频率（3个字节），占空比（1个字节）
                case 0x03:
                    // 判断管脚选择数据是否符合条件
                    if (Uart1_rec.guc_Uartbuf_a[1] == 0x20 ||
                        Uart1_rec.guc_Uartbuf_a[1] == 0x22 ||
                        Uart1_rec.guc_Uartbuf_a[1] == 0x23 ||
                        Uart1_rec.guc_Uartbuf_a[1] == 0x24 ||
                        Uart1_rec.guc_Uartbuf_a[1] == 0x25 ||
                        Uart1_rec.guc_Uartbuf_a[1] == 0x26 ||
                        Uart1_rec.guc_Uartbuf_a[1] == 0x27) 
                    {
                        IOPPx = (Uart1_rec.guc_Uartbuf_a[1] >> 4) & 0x03;
                        IOPPXx = Uart1_rec.guc_Uartbuf_a[1] & 0x07;

                        // 频率是否超过限制
                        PWMFrequency = (Uart1_rec.guc_Uartbuf_a[2] * 65536 + Uart1_rec.guc_Uartbuf_a[3] * 256 + Uart1_rec.guc_Uartbuf_a[4]);
                        if (PWMFrequency < 489) {
                            sprintf(guc_Uartbuf_p, "PWM 频率太小\r\n");
                            break;
                        }

                        if (PWMFrequency > 2000000) {
                            sprintf(guc_Uartbuf_p, "PWM 频率太大\r\n");
                            break;
                        }

                        // 占空比是否超过限制
                        if ((int)Uart1_rec.guc_Uartbuf_a[5] > 100) {
                            sprintf(guc_Uartbuf_p, "PWM 占空比太大\r\n");
                            break;
                        }

                        // 配置IO为推完输出
                        IOInitPushpull(IOPPx, IOPPXx);

                        // PWM输出管脚重映射
                        PWM0_MAP = Uart1_rec.guc_Uartbuf_a[1];

                        // 频率设置（周期）
                        PWMCycle = 2000000 / PWMFrequency;
                        PWM0PH = (PWMCycle >> 8) & 0xff;
                        PWM0PL = PWMCycle & 0xff;

                        // 占空比设置
                        PWMDutycycle = (unsigned int)(PWMCycle * (float)Uart1_rec.guc_Uartbuf_a[5] / 100.0);
                        PWM0DH = (PWMDutycycle >> 8) & 0x0f;
                        PWM0DL = (PWMDutycycle)&0xff;

                        sprintf(guc_Uartbuf_p,"PWM波设置成功,IO:P%d%d, 频率:%ld, 占空比:%u%%\r\n", (int)IOPPx,(int)IOPPXx, (long int)PWMFrequency,(unsigned int)Uart1_rec.guc_Uartbuf_a[5]);
                    }
                    else
                    {
                        sprintf(guc_Uartbuf_p, "指定PWM输出管脚错误!\r\n");
                    }
                    break;

                /*********************** 4 指定PWM输出周期和工作时间 ************************/
                // 缓存区数据：0x04，产生周期（2个字节），工作时间（2个字节）
                case 0x04:
                    // 输出周期 ms
                    PWMOutCycle = Uart1_rec.guc_Uartbuf_a[1] * 256 + Uart1_rec.guc_Uartbuf_a[2];
                    PWMWorkTime = Uart1_rec.guc_Uartbuf_a[3] * 256 + Uart1_rec.guc_Uartbuf_a[4];
                    sprintf(guc_Uartbuf_p, "PWM工作周期 = %dms   工作时间 = %dms\r\n", PWMOutCycle, PWMWorkTime);
                    break;

                /*********************** 5 输出指定波形 P01 P05 P07 ************************/
                // 管脚: P01 P05 P07
                // 缓存区数据：0x05，管脚选择（1个字节）  //fixme 为什么使用PWM实现指定波形？100ms的周期如何实现的？
                case 0x05:
                    IOPPx = (Uart1_rec.guc_Uartbuf_a[1] >> 4) & 0x03;
                    IOPPXx = Uart1_rec.guc_Uartbuf_a[1] & 0x07;

                    if (IOPPx != 0) {
                        sprintf(guc_Uartbuf_p, "指定波形输出管脚错误!\r\n");
                        break;
                    }

                    if (IOPPXx != 0x01 && IOPPXx != 0x05 && IOPPXx != 0x07) {
                        sprintf(guc_Uartbuf_p, "指定波形输出管脚错误!\r\n");
                        break;
                    }

                    // 配置IO为推完输出 P01 P05 P07
                    IOInitPushpull(IOPPx, IOPPXx);
                    SpecifywaveCount = 0;

                    sprintf(guc_Uartbuf_p, "设置指定波形输出管脚为:P%d%d\r\n", (int)IOPPx, (int)IOPPXx);
                    break;

                default:
                    sprintf(guc_Uartbuf_p, "指令错误!\r\n");
                    break;
            }

            // 清空串口接收缓存
            memcpy(Uart1_rec.guc_Uartbuf_a, 0, 20);

            // 接收下一帧数据
            Uart1_rec.guc_Uartflag = 0;
            Uart1_rec.guc_Uartcnt = 0;
            
            WriteStr_RingBuff(guc_Uartbuf_p,&ringBuff);
            /* 串口发送 */
            if (ringBuff.FlagByte & 0x01) 
            {
                ringBuff.FlagByte &= ~0x01;  //串口忙
                TI = 1;
            }
        }

        
    }
}

/***************************************************************************************
 * @说明  	初始化所有外设
 *	@参数	  无
 * @返回值 无
 * @注		  无
 ***************************************************************************************/

void SystemInit(void) 
{
    /************************************ 系统初始化
     * ****************************************/
    //    WDTCCR = 0x00;						//关闭看门狗
    CLKSWR = 0x51; // 选择内部高频RC为系统时钟，内部高频RC 2分频，Fosc=16MHz
    CLKDIV = 0x01; // Fosc 1分频得到Fcpu，Fcpu=16MHz

    /********************************** 串口1初始化
     * **************************************/
    P2M0 = P2M0 & 0x0F | 0x80; // P21设置为推挽输出
    P0M1 = P0M1 & 0x0F | 0x20; // P03设置为上拉输入

    TXD_MAP = 0x21; // TXD映射P21
    RXD_MAP = 0x03; // RXD映射P03

    BRTSEL = 0X00; // UART1的波特率:00 T4
    T4CON = 0x06;  // T4工作模式：UART1波特率发生器

    TH4 = 0xFF;
    TL4 = 0x98;   // 波特率9600
    SCON2 = 0x02; // 8位UART，波特率可变

    SCON = 0x10; // 允许串行接收
    IE |= 0X10;  // 使能串口中断

    EA = 1; // 打开总中断

    /************************************ TIM0配置初始化 1ms
     * *****************************************/
    TCON1 = 0x00; // Tx0定时器时钟为Fosc
    TMOD = 0x00;  // 16位重装载定时器/计数器

    TH0 = 0xFA;
    TL0 = 0xCB;   // T0定时时间1ms
    IE |= 0x02;   // 打开T0中断
    TCON |= 0x10; // 使能T0

    /********************************** TIM1配置初始化 25ms
     * **************************************/
    TCON1 = 0x00; // T1定时器时钟为Fosc  Timer分频系数 = 12
    TMOD = 0x00;  // 16位重装载定时器/计数器
    TH1 = 0x7d;
    TL1 = 0xca;
    IE |= 0x08;   // 打开T1中断
    TCON |= 0x40; // 使能T1

    /************************************ADC初始化*****************************************/
    P0M0 = P0M0 & 0xF0 | 0x03; // P00设置为模拟输入
    P0M1 = P0M1 & 0xF0 | 0x03; // P02设置为模拟输入
    P0M2 = P0M2 & 0xF0 | 0x03; // P04设置为模拟输入
    P0M3 = P0M3 & 0xF0 | 0x03; // P06设置为模拟输入

    ADCC0 = 0x81;  // 打开ADC转换电源
    Delay_2us(10); // 延时20us，确保ADC系统稳定
    ADCC1 = 0x00;  // 选择外部通道0
    ADCC2 = 0x4D; // 转换结果12位数据，数据右对齐，ADC时钟16分频 Fadc = 1MHz

    IP3 |= 0x0C; // ADC优先级调制最高
    IE1 |= 0x20; // 打开ADC中断
    EA = 1;

    ADCC0 &= ~0x20; // 清除ADC中断标志位
    //    ADCC0 |= 0x40;   // 启动ADC转换

    /********************************** Flash配置初始化
     * *************************************/
    FREQ_CLK = 0x10; // 指明当前系统时钟

    /*********************************** WDT配置初始化
     * **************************************/
    WDTC = 0x57; // 允许WDT复位，允许掉电/空闲模式下运行，1024分频 5.93s
    WDTCCR = 0xFF; // 写入00时，将关闭WDT功能（但不关闭内部低频RC），

    /************************************* PWM初始化
     * ****************************************/
    // 频率设置范围：489 - 2 000 000

    P2M1 = P2M1 & 0xF0 | 0x08; // P22设置为推挽输出
    PWM0_MAP = 0x22;           // PWM0通道映射P22口
    PWM0C = 0x01;              // PWM0高有效，PWM01高有效，时钟8分频
    PWM0PH = 0x03;             // 周期高4位设置为0x03
    PWM0PL = 0xFF;             // 周期低8位设置为0xFF
    PWM0DH = 0x01;             // PWM0高4位占空比0x01
    PWM0DL = 0x55;             // PWM0低8位占空比0x55
    PWM0EN = 0x0B; // 使能PWM0，工作于独立模式 PWM0输出 PWM01关闭 0000 1011

    /************************************* 指定波形
     * ****************************************/
    P0M0 = P0M0 & 0x0F | 0x80; //	P01设置为推完输出
}

/***************************************************************************************
 * @说明  	初始化管脚为推挽输出
 * @参数  	IOPPx ：哪一组IO
 *	@参数 	IOPPXx：某一个的哪一个IO
 * @返回值 无
 * @注		  无
 ***************************************************************************************/

void IOInitPushpull(unsigned char IOPPx, unsigned char IOPPXx) {
    // 初始化管脚
    switch (IOPPx) {
        // P2
    case 0x02:
        switch (IOPPXx / 2) {
            // P20 P21
        case 0:(IOPPXx % 2) ? (P2M0 = P2M0 & 0x0F | 0x80) : (P2M0 = P2M0 & 0xF0 | 0x08);break;

            // P23 P22
        case 1:(IOPPXx % 2) ? (P2M1 = P2M1 & 0x0F | 0x80) : (P2M1 = P2M1 & 0xF0 | 0x08);break;

            // P25 P24
        case 2:(IOPPXx % 2) ? (P2M2 = P2M2 & 0x0F | 0x80) : (P2M2 = P2M2 & 0xF0 | 0x08);break;

        case 3:(IOPPXx % 2) ? (P2M3 = P2M3 & 0x0F | 0x80) : (P2M3 = P2M3 & 0xF0 | 0x08);break;

        default:  break;
        }
        break;

        // P0
    case 0x00:
        switch (IOPPXx / 2) {
            // P00 P01
        case 0:(IOPPXx % 2) ? (P0M0 = P0M0 & 0x0F | 0x80) : (P0M0 = P0M0 & 0xF0 | 0x08);break;

            // P03 P02
        case 1:(IOPPXx % 2) ? (P0M1 = P0M1 & 0x0F | 0x80) : (P0M1 = P0M1 & 0xF0 | 0x08);break;

            // P05 P04
        case 2:(IOPPXx % 2) ? (P0M2 = P0M2 & 0x0F | 0x80) : (P0M2 = P0M2 & 0xF0 | 0x08);break;

        case 3: (IOPPXx % 2) ? (P0M3 = P0M3 & 0x0F | 0x80) : (P0M3 = P0M3 & 0xF0 | 0x08);break;

        default:  break;
        }
        break;

    default:
        break;
    }
}

/***************************************************************************************
 * @说明  	ADC中断服务函数
 *	@参数	  无
 * @返回值 无
 * @注		  无
 ***************************************************************************************/
void ADC_Rpt() interrupt ADC_VECTOR {
    // 读取数值
    ADCC0 &= ~0x20; // 清除ADC中断标志位

    gui_AdcValue_a[guc_Channel_Count] = ADCR; // 获取ADC数据

    guc_Channel_Count++;
    if (guc_Channel_Count == 4) {
        guc_Count++;
        guc_Channel_Count = 0;
        if (guc_Count == 64) {
            guc_Count = 0;
        }
    }

    // 切换通道
    ADCC1 = (ADCC1 & (~0x07)) | (guc_AdcChannel_a[guc_Channel_Count]); // 选择外部通道
    //    Delay_2us(10);                                                      //
    //    切换通道后建议延时20us
}

/***************************************************************************************
 * @说明  	T0中断服务函数
 *	@参数	  无
 * @返回值 无
 * @注		  无
 ***************************************************************************************/
void TIMER0_Rpt(void) interrupt TIMER0_VECTOR // 1ms
{
    // 串口超时判断
    timer_periodic_refresh();

    // 启动转换
    ADCC0 |= 0x40; // 启动下一次转换

    // PWM工作周期设置
    PWMOut_count++;
    if (PWMOut_count > PWMWorkTime) {
        PWM0EN &= ~0x01; // 关闭PWM模块
    }

    if (PWMOut_count >= PWMOutCycle) {
        PWMOut_count = 0;
        PWM0EN |= 0x01; // 打开PWM模块
    }
}

/**
 * @说明  	延时函数
 * @参数  	fui_i : 延时时间
 * @返回值 无
 * @注 	Fcpu = 16MHz,fui_i = 1时,延时时间约为2us
 */
void Delay_2us(unsigned int fui_i) {
    while (fui_i--)
        ;
}


/***************************************************************************************
 * @说明  	UART1中断服务函数
 *	@参数	  无
 * @返回值 无
 * @注		  无
 ***************************************************************************************/
void UART1_Rpt(void) interrupt UART1_VECTOR {
    static unsigned char UartsdfCnt = 0, UsartRecflg = 0,temp = 0;

    /* 串口接受中断 */
    if (SCON & 0x01) // 判断接收中断标志位
    {
        reload_timer(&usart1_timer,50); // 若有字符收到，装初值并启动定时器，若字节超时，则会自动调用定时器超时函数,超时设为50ms
        start_timer(&usart1_timer);

        if (UsartRecflg) // 开始接收数据
        {
            Uart1_rec.guc_Uartbuf_temp[Uart1_rec.guc_Uartcnt++] = SBUF; // 转存8位串口接收数据
        }

        // 帧头
        if (SBUF == 0xff) // 判断有没有可能是帧头
        {
            UartsdfCnt++;
        } else {
            UartsdfCnt = 0;
        }

        if (UartsdfCnt == 2) // 接收到帧头(2个0xff),开始接收数据
        {
            UsartRecflg = 1;

            Uart1_rec.guc_Uartcnt = 0; // 开始接收数据，清零计数

            // 清空串口接收缓存
            memcpy(Uart1_rec.guc_Uartbuf_temp, 0, 20);
        }


        // 接收的数据太多
        if (Uart1_rec.guc_Uartcnt >= 20) 
        {
//            SCON &= ~0x10;  // 失能UART1接收
            Uart1_rec.guc_Uartflag = 1;
            memcpy(Uart1_rec.guc_Uartbuf_a, 0, 20);
            memcpy(Uart1_rec.guc_Uartbuf_a, Uart1_rec.guc_Uartbuf_temp,Uart1_rec.guc_Uartcnt);
        }

        SCON &= ~0x01; // 清除接收中断标志位
    }

    
    /* 串口发送中断 */
    if (SCON & 0x02) // 判断发送中断标志位
    {
        if (ringBuff.Lenght) //缓存区有数据 
        {
            if (Read_RingBuff(&temp,&ringBuff))
            {
                SBUF = temp;
            }
        }
        else //缓存区没有数据
        {
            ringBuff.FlagByte |= 0x01;    //串口空闲 
        }
        
        SCON &= ~0x02; // 清除发送中断标志位
    }
}

/***************************************************************************************
 * @说明  	串口1定时器字节超时处理
 *	@参数	无
 * @返回值 无
 * @注		无
 ***************************************************************************************/
void usart1_timeover_proc(void) 
{
    memcpy(Uart1_rec.guc_Uartbuf_a, 0, 20);
    memcpy(Uart1_rec.guc_Uartbuf_a, Uart1_rec.guc_Uartbuf_temp,Uart1_rec.guc_Uartcnt); //保存接收到的数据

    Uart1_rec.guc_Uartflag = 1;
}

/**
 * @说明  	写入一个字节数据到Flash里面
 * @参数  	fui_Address ：FLASH地址
 *	@参数 	fucp_SaveData：写入的数据存放区域的首地址
 * @返回值 无
 * @注	  	写之前必须先对操作的扇区进行擦除
 */
#pragma disable // 确保调整时不会进中断导致调整失败
void FLASH_WriteData(unsigned char fuc_SaveData, unsigned int fui_Address) {
    IAP_DATA = fuc_SaveData;
    IAP_CMD = 0xF00F; // Flash解锁
    IAP_ADDR = fui_Address;
    IAP_CMD = 0xB44B; // 字节编程
    IAP_CMD = 0xE11E; // 触发一次操作
}

/**
 * @说明  	写入任意长度的数据到FLASH里面
 * @参数  	fui_Address ：FLASH起始地址
 *	@参数 	fuc_Length ： 写入数据长度
 *			    取值范围：0x00-0xFF
 *	@参数	 *fucp_SaveArr：写入的数据存放区域的首地址
 * @返回值 无
 * @注		  写之前必须先对操作的扇区进行擦除
 */
#pragma disable // 确保调整时不会进中断导致调整失败
void Flash_WriteArr(unsigned int fui_Address, unsigned char fuc_Length,
                    unsigned char *fucp_SaveArr) {
    unsigned char fui_i = 0;
    for (fui_i = 0; fui_i < fuc_Length; fui_i++) {
        FLASH_WriteData(*(fucp_SaveArr++), fui_Address++);
    }
}

/**
 * @说明  	从FLASH里面读取任意长度的数据
 * @参数  	fui_Address ：FLASH起始地址
 * @参数	  fuc_Length ：读取数据长度
 *			    取值范围：0x00-0xFF
 * @参数	 *fucp_SaveArr：读取数据存放的区域首地址
 * @返回值 无
 * @注		  无
 */
void Flash_ReadArr(unsigned int fui_Address, unsigned char fuc_Length,
                   unsigned char *fucp_SaveArr) {
    while (fuc_Length--)
        *(fucp_SaveArr++) = *((unsigned char code *)(fui_Address++)); // 读取数据
}

/**
 * @说明  	扇区擦除，约消耗5ms的时间
 * @参数  	fui_Address ：被擦除的扇区内的任意一个地址
 * @返回值 无
 * @注		  只要操作扇区里面的任意一个地址，就可以擦除此扇区
 */
#pragma disable // 确保调整时不会进中断导致调整失败
void Flash_EraseBlock(unsigned int fui_Address) {
    IAP_CMD = 0xF00F;       // Flash解锁
    IAP_ADDR = fui_Address; // 写入擦除地址
    IAP_CMD = 0xD22D;       // 选择操作方式， 扇区擦除
    IAP_CMD = 0xE11E; // 触发后IAP_ADDRL&IAP_ADDRH指向0xFF，同时自动锁定
}

/***************************************************************************************
 * @说明  	T1中断服务函数
 *	@参数	  无
 * @返回值 无
 * @注		  无
 ***************************************************************************************/
void TIMER1_Rpt(void) interrupt TIMER1_VECTOR // 25ms中断
{
    static unsigned int count = 0;
    count++;
    if (count >= 4) {
        count = 0;
        // 输出指定波形
        switch (SpecifywaveIOflag) {
            // P01
        case 0:
            P0_1 = SpecifywaveformOut[SpecifywaveCount];
            break;

            // P05
        case 1:
            P0_5 = SpecifywaveformOut[SpecifywaveCount];
            break;

            // P07
        case 2:
            P0_7 = SpecifywaveformOut[SpecifywaveCount];
            break;

        default:
            break;
        }
        SpecifywaveCount++;
        if (SpecifywaveCount > 14)
            SpecifywaveCount = 0;
    }
}


