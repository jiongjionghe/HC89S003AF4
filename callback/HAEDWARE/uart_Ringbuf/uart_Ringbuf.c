#include "uart_Ringbuf.h"


/***************************************************************************************
 * @˵��  	��ʼ�����ζ���
 * @���� 	RingBuff_t *ringbuff : ����ʼ���Ļ��ζ���
 * @����ֵ  ��
 * @ע		��
 ***************************************************************************************/
void RingBuff_Init(RingBuff_t *ringbuff)
{
    //��ʼ�������Ϣ
    ringbuff->Head = 0;
    ringbuff->Tail = 0;
    ringbuff->Lenght = 0;
    ringbuff->FlagByte = 0;
    
    ringbuff->FlagByte |= 0x01; //���ڿ���
}


/***************************************************************************************
 * @˵��  	���ζ���д������
 * @����  	unsigned char mydata : Ҫд�������
 * @���� 	RingBuff_t *ringbuff : д��Ļ��ζ���
 * @����ֵ  ��
 * @ע		��
 ***************************************************************************************/
unsigned char Write_RingBuff(unsigned char mydata,RingBuff_t *ringbuff)
{
    if(ringbuff->Lenght >= RINGBUFF_LEN) //�жϻ������Ƿ�����
    {
        return RINGBUFF_ERR;
    }
    ringbuff->Ring_data[ringbuff->Tail]=mydata;
    ringbuff->Tail = (ringbuff->Tail+1)%RINGBUFF_LEN;//��ֹԽ��Ƿ�����
    ringbuff->Lenght++;

    return RINGBUFF_OK;
}

/***************************************************************************************
 * @˵��  	���ζ��ж�ȡһ���ֽڵ�����
 * @����  	unsigned char *rData : ����������
 * @���� 	RingBuff_t *ringbuff : ��Ҫ��ȡ�Ļ��ζ���
 * @����ֵ  ��
 * @ע		��
 ***************************************************************************************/
unsigned char  Read_RingBuff(unsigned char *rData,RingBuff_t *ringbuff)
{
    if(ringbuff->Lenght == 0)//�жϷǿ�
    {
        return RINGBUFF_ERR;
    }
    *rData = ringbuff->Ring_data[ringbuff->Head];//�Ƚ��ȳ�FIFO���ӻ�����ͷ��
    ringbuff->Head = (ringbuff->Head+1)%RINGBUFF_LEN;//��ֹԽ��Ƿ�����
    ringbuff->Lenght--;
    return RINGBUFF_OK;
}

/***************************************************************************************
 * @˵��  	���ζ���д��һ�����������
 * @����  	unsigned char *myStr : Ҫд�������
 * @���� 	RingBuff_t *ringbuff : д��Ļ��ζ���
 * @����ֵ  ��
 * @ע		��
 ***************************************************************************************/
void WriteStr_RingBuff(unsigned char *myStr,RingBuff_t *ringbuff)
{
    unsigned char i;
    for(i = 0; myStr[i] != '\0'; i++)
    {
        Write_RingBuff(myStr[i],ringbuff);
    }
}


