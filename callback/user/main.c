#include "main.h"
/***************************************************************************************
  * @ʵ��Ч��	�׶���
��ʱ��0�� 1ms
��ʱ��1:  25ms

ADCͨ����
AN0 - P00
AN2 - P02
AN4 - P04
AN6 - P06

���ڣ�
RXD - P21
TXD - P03

PWM:
PWM0  - P20 P22 P23 P24 P25 P26 P27

ָ������:
P01 P05 P07
***************************************************************************************/
void main(void)
{
    SystemInit();
    RingBuff_Init(&ringBuff);

    while (1)
    {
        WDTC |= 0x10;  // �幷

        if (Uart1_rec.guc_Uartflag)
        {
            memcpy(guc_Uartbuf_p, 0, UART1BUFLEN);
            
            // ����ѡ��
            if (Uart1_rec.guc_Uartbuf_a[0] > 0 && Uart1_rec.guc_Uartbuf_a[0] < 6)
            {
                poll_task(Uart1_rec.guc_Uartbuf_a[0]);
            }
            else
            {
                sprintf(guc_Uartbuf_p,"û�и����\r\n");
            }
            
            // �����������
            memcpy(Uart1_rec.guc_Uartbuf_a, 0, 20);

            // ����֡��������
            Uart1_rec.guc_Uartflag = 0;
            
            //������д�뷢�ͻ�����
            WriteStr_RingBuff(guc_Uartbuf_p,&ringBuff);
            
            
            /* �������ڷ��� */
            if (ringBuff.FlagByte & 0x01) 
            {
                ringBuff.FlagByte &= ~0x01;  //����æ
                TI = 1;
            }
        }
    }
}

/***************************************************************************************
 * @˵��  	��ʼ����������
 *	@����	  ��
 * @����ֵ ��
 * @ע		  ��
 ***************************************************************************************/

void SystemInit(void)
{
    /************************************ ϵͳ��ʼ�� ****************************************/
    //    WDTCCR = 0x00;						//�رտ��Ź�
    CLKSWR = 0x51;  // ѡ���ڲ���ƵRCΪϵͳʱ�ӣ��ڲ���ƵRC 2��Ƶ��Fosc=16MHz
    CLKDIV = 0x01;  // Fosc 1��Ƶ�õ�Fcpu��Fcpu=16MHz

    /********************************** ����1��ʼ�� **************************************/
    P2M0 = P2M0 & 0x0F | 0x80;  // P21����Ϊ�������
    P0M1 = P0M1 & 0x0F | 0x20;  // P03����Ϊ��������

    TXD_MAP = 0x21;  // TXDӳ��P21
    RXD_MAP = 0x03;  // RXDӳ��P03

    BRTSEL = 0X00;  // UART1�Ĳ�����:00 T4
    T4CON  = 0x06;  // T4����ģʽ��UART1�����ʷ�����

    TH4   = 0xFF;
    TL4   = 0x98;  // ������9600
    SCON2 = 0x02;  // 8λUART�������ʿɱ�

    SCON = 0x10;  // �����н���
    IE |= 0X10;   // ʹ�ܴ����ж�

    EA = 1;  // �����ж�

    /************************************ TIM0���ó�ʼ�� 1ms *****************************************/
    TCON1 = 0x00;  // Tx0��ʱ��ʱ��ΪFosc
    TMOD  = 0x00;  // 16λ��װ�ض�ʱ��/������

    TH0 = 0xFA;
    TL0 = 0xCB;    // T0��ʱʱ��1ms
    IE |= 0x02;    // ��T0�ж�
    TCON |= 0x10;  // ʹ��T0

    /********************************** TIM1���ó�ʼ�� 25ms **************************************/
    TCON1 = 0x00;  // T1��ʱ��ʱ��ΪFosc  Timer��Ƶϵ�� = 12
    TMOD  = 0x00;  // 16λ��װ�ض�ʱ��/������
    TH1   = 0x7d;
    TL1   = 0xca;
    IE |= 0x08;    // ��T1�ж�
    TCON |= 0x40;  // ʹ��T1

    /************************************ADC��ʼ��*****************************************/
    P0M0 = P0M0 & 0xF0 | 0x03;  // P00����Ϊģ������
    P0M1 = P0M1 & 0xF0 | 0x03;  // P02����Ϊģ������
    P0M2 = P0M2 & 0xF0 | 0x03;  // P04����Ϊģ������
    P0M3 = P0M3 & 0xF0 | 0x03;  // P06����Ϊģ������

    ADCC0 = 0x81;   // ��ADCת����Դ
    Delay_2us(10);  // ��ʱ20us��ȷ��ADCϵͳ�ȶ�
    ADCC1 = 0x00;   // ѡ���ⲿͨ��0
    ADCC2 = 0x4D;   // ת�����12λ���ݣ������Ҷ��룬ADCʱ��16��Ƶ Fadc = 1MHz

    IP3 |= 0x0C;   //ADC���ȼ��������
    IE1 |= 0x20;  // ��ADC�ж�
    EA = 1;

    ADCC0 &= ~0x20;  // ���ADC�жϱ�־λ
//    ADCC0 |= 0x40;   // ����ADCת��

    /********************************** Flash���ó�ʼ�� *************************************/
    FREQ_CLK = 0x10;  // ָ����ǰϵͳʱ��

    /*********************************** WDT���ó�ʼ�� **************************************/
    WDTC   = 0x57;  // ����WDT��λ���������/����ģʽ�����У�1024��Ƶ 5.93s
    WDTCCR = 0xFF;  // д��00ʱ�����ر�WDT���ܣ������ر��ڲ���ƵRC����

    /************************************* PWM��ʼ�� ****************************************/
    // Ƶ�����÷�Χ��489 - 2 000 000

    P2M1     = P2M1 & 0xF0 | 0x08;  // P22����Ϊ�������
    PWM0_MAP = 0x22;                // PWM0ͨ��ӳ��P22��
    PWM0C    = 0x01;                // PWM0����Ч��PWM01����Ч��ʱ��8��Ƶ
    PWM0PH   = 0x03;                // ���ڸ�4λ����Ϊ0x03
    PWM0PL   = 0xFF;                // ���ڵ�8λ����Ϊ0xFF
    PWM0DH   = 0x01;                // PWM0��4λռ�ձ�0x01
    PWM0DL   = 0x55;                // PWM0��8λռ�ձ�0x55
    PWM0EN   = 0x0B;                // ʹ��PWM0�������ڶ���ģʽ PWM0��� PWM01�ر� 0000 1011

    /************************************* ָ������ ****************************************/
    P0M0 = P0M0 & 0x0F | 0x80;  //	P01����Ϊ�������

    /************************************* ������������ó�ֵ ****************************************/
    // ��ȡFlash�еĸ�ͨ����ѹϵ��
    Flash_ReadArr(VolCoeFlashAddress, 16, VolCoeStore);

    for (i = 0; i < 4; i++) {
        memcpy(Vol_b.stByte.u8Byte, &VolCoeStore[i * 4], 4);
        VolCoe[i] = Vol_b.all;
    }

    // ���ڳ�ʱ�ж�
    soft_timer_list_reset();
    add_timer(&usart1_timer, usart1_timeover_proc, 50);  // 50MS
    start_timer(&usart1_timer);


    // ��մ��ڽ��ջ���
    memcpy(Uart1_rec.guc_Uartbuf_a, 0, 20);

//    Uart1.FlagByte |= 0x01;  //���ڿ���
//    memcpy(Uart1.Data, 0, UART1BUFLEN);
}

/***************************************************************************************
 * @˵��  	��ʼ���ܽ�Ϊ�������
 * @����  	IOPPx ����һ��IO
 *	@���� 	IOPPXx��ĳһ������һ��IO
 * @����ֵ ��
 * @ע		  ��
 ***************************************************************************************/

void IOInitPushpull(unsigned char IOPPx, unsigned char IOPPXx)
{
// ��ʼ���ܽ�
    switch (IOPPx)
    {
        // P2
    case 0x02:
        switch (IOPPXx / 2)
        {
            // P20 P21
        case 0:
            (IOPPXx % 2) ? ( P2M0 = P2M0 & 0x0F | 0x80):(P2M0 = P2M0 & 0xF0 | 0x08);
            break;

            // P23 P22
        case 1:
            (IOPPXx % 2) ? ( P2M1 = P2M1 & 0x0F | 0x80):(P2M1 = P2M1 & 0xF0 | 0x08);
            break;

            // P25 P24
        case 2:
            (IOPPXx % 2) ? ( P2M2 = P2M2 & 0x0F | 0x80):(P2M2 = P2M2 & 0xF0 | 0x08);
            break;

        case 3:
            (IOPPXx % 2) ? ( P2M3 = P2M3 & 0x0F | 0x80):(P2M3 = P2M3 & 0xF0 | 0x08);
            break;

        default:
            break;
        }
        break;

        //P0
    case 0x00:
        switch (IOPPXx / 2)
        {
            // P00 P01
        case 0:
            (IOPPXx % 2) ? ( P0M0 = P0M0 & 0x0F | 0x80):(P0M0 = P0M0 & 0xF0 | 0x08);
            break;

            // P03 P02
        case 1:
            (IOPPXx % 2) ? ( P0M1 = P0M1 & 0x0F | 0x80):(P0M1 = P0M1 & 0xF0 | 0x08);
            break;

            // P05 P04
        case 2:
            (IOPPXx % 2) ? ( P0M2 = P0M2 & 0x0F | 0x80):(P0M2 = P0M2 & 0xF0 | 0x08);
            break;

        case 3:
            (IOPPXx % 2) ? ( P0M3 = P0M3 & 0x0F | 0x80):(P0M3 = P0M3 & 0xF0 | 0x08);
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }
}

/***************************************************************************************
 * @˵��  	ADC�жϷ�����
 *	@����	  ��
 * @����ֵ ��
 * @ע		  ��
 ***************************************************************************************/
void ADC_Rpt() interrupt ADC_VECTOR
{
    // ��ȡ��ֵ
    ADCC0 &= ~0x20;  // ���ADC�жϱ�־λ

    gui_AdcValue_a[guc_Channel_Count] = ADCR;  // ��ȡADC����

    guc_Channel_Count++;
    if (guc_Channel_Count == 4) {
        guc_Count++;
        guc_Channel_Count = 0;
        if (guc_Count == 64) {
            guc_Count = 0;
        }
    }

    // �л�ͨ��
    ADCC1 = (ADCC1 & (~0x07)) | (guc_AdcChannel_a[guc_Channel_Count]);  // ѡ���ⲿͨ��
//    Delay_2us(10);                                                      // �л�ͨ��������ʱ20us
}

/***************************************************************************************
 * @˵��  	T0�жϷ�����
 *	@����	  ��
 * @����ֵ ��
 * @ע		  ��
 ***************************************************************************************/
void TIMER0_Rpt(void) interrupt TIMER0_VECTOR  // 1ms
{
    // ���ڳ�ʱ�ж�
    timer_periodic_refresh();

    // ����ת��
    ADCC0 |= 0x40;  // ������һ��ת��

    // PWM������������
    PWMOut_count++;
    if (PWMOut_count > PWMWorkTime) {
        PWM0EN &= ~0x01;  // �ر�PWMģ��
    }

    if (PWMOut_count >= PWMOutCycle) {
        PWMOut_count = 0;
        PWM0EN |= 0x01;  // ��PWMģ��
    }
}

/**
 * @˵��  	��ʱ����
 * @����  	fui_i : ��ʱʱ��
 * @����ֵ ��
 * @ע 	Fcpu = 16MHz,fui_i = 1ʱ,��ʱʱ��ԼΪ2us
 */
void Delay_2us(unsigned int fui_i)
{
    while (fui_i--)
        ;
}

/***************************************************************************************
 * @˵��  	UART1�жϷ�����
 *	@����	  ��
 * @����ֵ ��
 * @ע		  ��
 ***************************************************************************************/
void UART1_Rpt(void) interrupt UART1_VECTOR
{
    static unsigned char UartsdfCnt = 0, UsartRecflg = 0, temp = 0;

    /* ���ڽ����ж� */
    if (SCON & 0x01)  // �жϽ����жϱ�־λ
    {
        reload_timer(&usart1_timer, 50);  // �����ַ��յ���װ��ֵ��������ʱ�������ֽڳ�ʱ������Զ����ö�ʱ����ʱ����,��ʱ��Ϊ50ms
        start_timer(&usart1_timer);

        if (UsartRecflg)  // ��ʼ��������
        {
            Uart1_rec.guc_Uartbuf_temp[Uart1_rec.guc_Uartcnt++] = SBUF;  // ת��8λ���ڽ�������
        }

        // ֡ͷ
        if (SBUF == 0xff)  // �ж���û�п�����֡ͷ
        {
            UartsdfCnt++;
        }
        else {
            UartsdfCnt = 0;
        }

        if (UartsdfCnt == 2)  // ���յ�֡ͷ(2��0xff),��ʼ��������
        {
            UsartRecflg = 1;

            Uart1_rec.guc_Uartcnt = 0;  // ��ʼ�������ݣ��������

            // ��մ��ڽ��ջ���
            memcpy(Uart1_rec.guc_Uartbuf_temp, 0, 20);
        }

        // ���յ�����̫��
        if (Uart1_rec.guc_Uartcnt >= 20) {
            Uart1_rec.guc_Uartflag = 1;
            memcpy(Uart1_rec.guc_Uartbuf_a, 0, 20);
            memcpy(Uart1_rec.guc_Uartbuf_a, Uart1_rec.guc_Uartbuf_temp,Uart1_rec.guc_Uartcnt);
            Uart1_rec.guc_Uartcnt  = 0;
        }

        SCON &= ~0x01;  // ��������жϱ�־λ
    }

    /* ���ڷ����ж� */
    if (SCON & 0x02)  // �жϷ����жϱ�־λ
    {
        if (ringBuff.Lenght) //������������ 
        {
            if (Read_RingBuff(&temp,&ringBuff))
            {
                SBUF = temp;
            }
        }
        else //������û������
        {
            ringBuff.FlagByte |= 0x01;    //���ڿ��� 
        }

        SCON &= ~0x02;  // ��������жϱ�־λ
    }
}

/***************************************************************************************
 * @˵��  	����1��ʱ���ֽڳ�ʱ����
 *	@����	��
 * @����ֵ ��
 * @ע		��
 ***************************************************************************************/
void usart1_timeover_proc(void)
{
//    SCON &= ~0x10;  // ʧ��UART1����
    memcpy(Uart1_rec.guc_Uartbuf_a, 0, 20);
    memcpy(Uart1_rec.guc_Uartbuf_a, Uart1_rec.guc_Uartbuf_temp,Uart1_rec.guc_Uartcnt); //������յ�������

    Uart1_rec.guc_Uartflag = 1;
    Uart1_rec.guc_Uartcnt  = 0;
}

/**
 * @˵��  	д��һ���ֽ����ݵ�Flash����
 * @����  	fui_Address ��FLASH��ַ
 *	@���� 	fucp_SaveData��д������ݴ��������׵�ַ
 * @����ֵ ��
 * @ע	  	д֮ǰ�����ȶԲ������������в���
 */
#pragma disable  // ȷ������ʱ������жϵ��µ���ʧ��
void FLASH_WriteData(unsigned char fuc_SaveData, unsigned int fui_Address)
{
    IAP_DATA = fuc_SaveData;
    IAP_CMD  = 0xF00F;  // Flash����
    IAP_ADDR = fui_Address;
    IAP_CMD  = 0xB44B;  // �ֽڱ��
    IAP_CMD  = 0xE11E;  // ����һ�β���
}

/**
 * @˵��  	д�����ⳤ�ȵ����ݵ�FLASH����
 * @����  	fui_Address ��FLASH��ʼ��ַ
 *	@���� 	fuc_Length �� д�����ݳ���
 *			    ȡֵ��Χ��0x00-0xFF
 *	@����	 *fucp_SaveArr��д������ݴ��������׵�ַ
 * @����ֵ ��
 * @ע		  д֮ǰ�����ȶԲ������������в���
 */
#pragma disable  // ȷ������ʱ������жϵ��µ���ʧ��
void Flash_WriteArr(unsigned int fui_Address, unsigned char fuc_Length, unsigned char* fucp_SaveArr)
{
    unsigned char fui_i = 0;
    for (fui_i = 0; fui_i < fuc_Length; fui_i++) {
        FLASH_WriteData(*(fucp_SaveArr++), fui_Address++);
    }
}

/**
 * @˵��  	��FLASH�����ȡ���ⳤ�ȵ�����
 * @����  	fui_Address ��FLASH��ʼ��ַ
 * @����	  fuc_Length ����ȡ���ݳ���
 *			    ȡֵ��Χ��0x00-0xFF
 * @����	 *fucp_SaveArr����ȡ���ݴ�ŵ������׵�ַ
 * @����ֵ ��
 * @ע		  ��
 */
void Flash_ReadArr(unsigned int fui_Address, unsigned char fuc_Length, unsigned char* fucp_SaveArr)
{
    while (fuc_Length--) *(fucp_SaveArr++) = *((unsigned char code*)(fui_Address++));  // ��ȡ����
}

/**
 * @˵��  	����������Լ����5ms��ʱ��
 * @����  	fui_Address ���������������ڵ�����һ����ַ
 * @����ֵ ��
 * @ע		  ֻҪ�����������������һ����ַ���Ϳ��Բ���������
 */
#pragma disable  // ȷ������ʱ������жϵ��µ���ʧ��
void Flash_EraseBlock(unsigned int fui_Address)
{
    IAP_CMD  = 0xF00F;       // Flash����
    IAP_ADDR = fui_Address;  // д�������ַ
    IAP_CMD  = 0xD22D;       // ѡ�������ʽ�� ��������
    IAP_CMD  = 0xE11E;       // ������IAP_ADDRL&IAP_ADDRHָ��0xFF��ͬʱ�Զ�����
}

/***************************************************************************************
 * @˵��  	T1�жϷ�����
 *	@����	  ��
 * @����ֵ ��
 * @ע		  ��
 ***************************************************************************************/
void TIMER1_Rpt(void) interrupt TIMER1_VECTOR  // 25ms�ж�
{
    static unsigned int count = 0;
    count++;
    if (count >= 4) 
    {
        count = 0;
        // ���ָ������
        switch (SpecifywaveIOflag) 
        {
            // P01
            case 0:P0_1 = SpecifywaveformOut[SpecifywaveCount];break;

            // P05
            case 1:P0_5 = SpecifywaveformOut[SpecifywaveCount];break;

            // P07
            case 2:P0_7 = SpecifywaveformOut[SpecifywaveCount];break;

            default:break;
        }
        SpecifywaveCount++;
        if (SpecifywaveCount > 14) SpecifywaveCount = 0;
    }
}

/***************************************************************************************
 * @˵��  	ָ����ѯ����
 * @����    ��
 * @����ֵ  ��
 * @ע      ��
 ***************************************************************************************/
void poll_task(u8 cmd)
{
    int cmd_index = 0;

    for (cmd_index = 0; cmd_index < 5; cmd_index++)
    {
        if (callback_list[cmd_index].CMD == cmd)
        {
            callback_list[cmd_index].callback_func(Uart1_rec.guc_Uartbuf_a);
        }
    }

}

/***************************************************************************************
 * @˵��  	���ܣ���ѯͨ����ѹ �Ļص�����
 * @����    ��
 * @����ֵ  ��
 * @ע      ��
 ***************************************************************************************/
/*********************** 1 ��ѯͨ����ѹ ***********************/
// �ܽ�: P00 P02 P04 P06
// ���������ݣ�0x01��ͨ��ѡ��1���ֽڣ�
void callback_QueryVoltage(unsigned char *myStr)
{
    u8 ANx = 0;
    float UARTprintVolCoe = 0;
    if (myStr[1] == 0x00 ||
        myStr[1] == 0x02 ||
        myStr[1] == 0x04 ||
        myStr[1] == 0x06)
    {
        ANx = (int)myStr[1]/2;
        UARTprintVolCoe = (float)(gui_AdcValue_a[ANx] / 4096.0) * VDD * VolCoe[ANx];
        sprintf(guc_Uartbuf_p,"AN%d = %f\r\n", (int)myStr[1],UARTprintVolCoe);
    }
    else
    {
        sprintf(guc_Uartbuf_p,"��ͨ�������ڣ�\r\n");
    }
    
    
//    sprintf(guc_Uartbuf_p,"����1\r\n");
}


/***************************************************************************************
 * @˵��  	���ܣ��޸�ָ��ͨ����ѹ���� �Ļص�����
 * @����    ��
 * @����ֵ  ��
 * @ע      ��
 ***************************************************************************************/
/*********************** 2 �޸�ָ��ͨ����ѹ���� ***********************/
// �ܽ�: P00 P02 P04 P06
// ���������ݣ�0x02��ͨ��ѡ��1���ֽڣ�����ѹϵ����4���ֽڣ�float��
void callback_ModifyVoltageParameters(unsigned char *myStr)
{
    if (myStr[1] == 0x00 || myStr[1] == 0x02 || myStr[1] == 0x04 || myStr[1] == 0x06)
    {
        memcpy(Vol_a.stByte.u8Byte, &myStr[2], 4);
        VolCoe[(int)myStr[1] / 2] = Vol_a.all;

        for (i = 0; i < 4; i++)
        {
            Vol_b.all = VolCoe[i];
            memcpy(&VolCoeStore[i * 4], Vol_b.stByte.u8Byte, 4);
        }
        Flash_EraseBlock(VolCoeFlashAddress);                 // ������ַ0x3B00��������
        Flash_WriteArr(VolCoeFlashAddress, 16, VolCoeStore);  // �ڵ�ַ0x3B00д������
        sprintf(guc_Uartbuf_p,"ok!\r\nVolCoe: %f - %f - %f - %f\r\n", VolCoe[0], VolCoe[1], VolCoe[2], VolCoe[3]);
    }
    else
    {
        sprintf(guc_Uartbuf_p,"���ô���!\r\n");
    }
    
//    sprintf(guc_Uartbuf_p,"����2\r\n");
}

/***************************************************************************************
 * @˵��  	���ܣ�ָ��PWM��ʵ�ֹܽţ�Ƶ�ʺ�ռ�ձ� �Ļص�����
 * @����    ��
 * @����ֵ  ��
 * @ע      ��
 ***************************************************************************************/
/*********************** 3 ָ��PWM��ʵ�ֹܽţ�Ƶ�ʺ�ռ�ձ� ***********************/
// �ܽ�: P20 P22 P23 P24 P25 P26 P27    Ƶ�ʣ�489 - 2M   ռ�ձȣ�0 - 100
// ���������ݣ�0x03���ܽ�ѡ��1���ֽڣ���Ƶ�ʣ�3���ֽڣ���ռ�ձȣ�1���ֽڣ�
void callback_ModifyPWMParameter(unsigned char *myStr)
{
    strChar           IOPP;
    unsigned long int PWMFrequency = 0;
    unsigned int      PWMCycle = 0, PWMDutycycle = 0;  // PWMCycle - ���PWM��������  PWMDutycycle - ���PWMռ�ձȵľ�����ֵ

    // �жϹܽ�ѡ�������Ƿ��������
    IOPP.byte = myStr[1];
    
    if ((myStr[1] & 0xc8) != 0x00) 
    {
        sprintf(guc_Uartbuf_p,"ָ��PWM����ܽŴ���!\r\n");
        return ;
    }


    if (IOPP.half.low != 0x02) 
    {
        sprintf(guc_Uartbuf_p,"ָ��PWM����ܽŴ���!\r\n");
        return ;
    }

    if (IOPP.half.high == 0x01) 
    {
        sprintf(guc_Uartbuf_p,"ָ��PWM����ܽŴ���!\r\n");
        return ;
    }
  

    // Ƶ���Ƿ񳬹�����
    PWMFrequency = (myStr[2] * 65536 + myStr[3] * 256 + myStr[4]);
    
    if (PWMFrequency < 489)
    {
        sprintf(guc_Uartbuf_p,"PWM Ƶ��̫С\r\n");
        return ;
    }
    else if (PWMFrequency > 2000000)
    {
        sprintf(guc_Uartbuf_p,"PWM Ƶ��̫��\r\n");
        return ;
    }

    // ռ�ձ��Ƿ񳬹�����
    if ((int)myStr[5] > 100)
    {
        sprintf(guc_Uartbuf_p,"PWM ռ�ձ�̫��\r\n");
        return ;
    }

    // ����IOΪ�������
    IOInitPushpull(IOPP.half.low, IOPP.half.high);

    // PWM����ܽ���ӳ��
    PWM0_MAP = myStr[1];

    // Ƶ�����ã����ڣ�
    PWMCycle = 2000000 / PWMFrequency;
    PWM0PH   = (PWMCycle >> 8) & 0xff;
    PWM0PL   = PWMCycle & 0xff;

    // ռ�ձ�����
    PWMDutycycle = (unsigned int)(PWMCycle * (float)myStr[5] / 100.0);
    PWM0DH = (PWMDutycycle >> 8) & 0x0f;
    PWM0DL = (PWMDutycycle)&0xff;

    sprintf(guc_Uartbuf_p,"PWM�����óɹ�,IO:P%d%d, Ƶ��:%ld, ռ�ձ�:%u%%\r\n",(int)IOPP.half.low, (int)IOPP.half.high, (long int)PWMFrequency, (unsigned int)myStr[5]);
        
//    sprintf(guc_Uartbuf_p,"����3\r\n");
}

/***************************************************************************************
 * @˵��  	���ܣ�ָ��PWM������ں͹���ʱ�� �Ļص�����
 * @����    ��
 * @����ֵ  ��
 * @ע      ��
 ***************************************************************************************/
/*********************** 4 ָ��PWM������ں͹���ʱ�� ***********************/
// ���������ݣ�0x04���������ڣ�2���ֽڣ�������ʱ�䣨2���ֽڣ�
void callback_PWMOperatingParameters(unsigned char *myStr)
{
    PWMOutCycle = myStr[1] * 256 + myStr[2];
    PWMWorkTime = myStr[3] * 256 + myStr[4];
    sprintf(guc_Uartbuf_p,"PWM�������� = %dms   ����ʱ�� = %dms\r\n", PWMOutCycle, PWMWorkTime);
    
//    sprintf(guc_Uartbuf_p,"����4\r\n");
}

/***************************************************************************************
 * @˵��  	���ܣ����ָ������ �Ļص�����
 * @����    ��
 * @����ֵ  ��
 * @ע      ��
 ***************************************************************************************/
/*********************** 5 ���ָ������ P01 P05 P07 ***********************/
// �ܽ�: P01 P05 P07
// ���������ݣ�0x05���ܽ�ѡ��1���ֽڣ�  //fixme Ϊʲôʹ��PWMʵ��ָ�����Σ�100ms���������ʵ�ֵģ�
void callback_SpecifyWaveform(unsigned char *myStr)
{
    strChar  IOPP;
    
    IOPP.byte = Uart1_rec.guc_Uartbuf_a[1];

    if (myStr[1] == 0x01 ||
        myStr[1] == 0x05 ||
        myStr[1] == 0x07)
    {
        // ����IOΪ������� P01 P05 P07
        IOInitPushpull(IOPP.half.low, IOPP.half.high);
        SpecifywaveCount = 0;

        sprintf(guc_Uartbuf_p,"����ָ����������ܽ�Ϊ:P%d%d\r\n", (int)IOPP.half.low, (int)IOPP.half.high);
    }
    else
    {
        sprintf(guc_Uartbuf_p,"ָ����������ܽŴ���!\r\n");
    }

//    sprintf(guc_Uartbuf_p,"����5\r\n");
}


