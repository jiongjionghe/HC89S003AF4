#include "uart.h"


/**
* @brief  RingBuff_Init
* @param  *ringbuff ����ʼ���Ļ�����
* @return void
* @note   ��ʼ�����λ�����
*/
void RingBuff_Init(RingBuff_t *ringbuff)
{
    //��ʼ�������Ϣ
    ringbuff->Head = 0;
    ringbuff->Tail = 0;
    ringbuff->Lenght = 0;
    ringbuff->FlagByte = 0;
    ringbuff->FlagByte |= 0x01; //���ڿ���
}


/**
* @brief  Write_RingBuff
* @param  unsigned char mydata     RingBuff_t *ringbuff
* @return FLASE:���λ�����������д��ʧ��;TRUE:д��ɹ�
* @note   �����λ�����д��uint8_t���͵�����
*/
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

/**
* @brief  Read_RingBuff
* @param  unsigned char *rData�����ڱ����ȡ����s��
* @return FLASE:���λ�����û�����ݣ���ȡʧ��;TRUE:��ȡ�ɹ�
* @note   �ӻ��λ�������ȡһ��u8���͵�����
*/
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


void WriteStr_RingBuff(unsigned char *myStr,RingBuff_t *ringbuff)
{
    unsigned char i;
    for(i = 0; myStr[i] != '\0'; i++)
    {
        Write_RingBuff(myStr[i],ringbuff);
    }
}


