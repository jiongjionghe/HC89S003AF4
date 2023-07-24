#include "uart.h"


/**
* @brief  RingBuff_Init
* @param  *ringbuff 带初始化的缓冲区
* @return void
* @note   初始化环形缓冲区
*/
void RingBuff_Init(RingBuff_t *ringbuff)
{
    //初始化相关信息
    ringbuff->Head = 0;
    ringbuff->Tail = 0;
    ringbuff->Lenght = 0;
    ringbuff->FlagByte = 0;
    ringbuff->FlagByte |= 0x01; //串口空闲
}


/**
* @brief  Write_RingBuff
* @param  unsigned char mydata     RingBuff_t *ringbuff
* @return FLASE:环形缓冲区已满，写入失败;TRUE:写入成功
* @note   往环形缓冲区写入uint8_t类型的数据
*/
unsigned char Write_RingBuff(unsigned char mydata,RingBuff_t *ringbuff)
{
    if(ringbuff->Lenght >= RINGBUFF_LEN) //判断缓冲区是否已满
    {
        return RINGBUFF_ERR;
    }
    ringbuff->Ring_data[ringbuff->Tail]=mydata;
    ringbuff->Tail = (ringbuff->Tail+1)%RINGBUFF_LEN;//防止越界非法访问
    ringbuff->Lenght++;

    return RINGBUFF_OK;
}

/**
* @brief  Read_RingBuff
* @param  unsigned char *rData，用于保存读取的数s据
* @return FLASE:环形缓冲区没有数据，读取失败;TRUE:读取成功
* @note   从环形缓冲区读取一个u8类型的数据
*/
unsigned char  Read_RingBuff(unsigned char *rData,RingBuff_t *ringbuff)
{
    if(ringbuff->Lenght == 0)//判断非空
    {
        return RINGBUFF_ERR;
    }
    *rData = ringbuff->Ring_data[ringbuff->Head];//先进先出FIFO，从缓冲区头出
    ringbuff->Head = (ringbuff->Head+1)%RINGBUFF_LEN;//防止越界非法访问
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


