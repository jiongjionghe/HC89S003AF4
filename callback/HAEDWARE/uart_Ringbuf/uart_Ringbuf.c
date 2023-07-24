#include "uart_Ringbuf.h"


/***************************************************************************************
 * @说明  	初始化环形队列
 * @参数 	RingBuff_t *ringbuff : 待初始化的环形队列
 * @返回值  无
 * @注		无
 ***************************************************************************************/
void RingBuff_Init(RingBuff_t *ringbuff)
{
    //初始化相关信息
    ringbuff->Head = 0;
    ringbuff->Tail = 0;
    ringbuff->Lenght = 0;
    ringbuff->FlagByte = 0;
    
    ringbuff->FlagByte |= 0x01; //串口空闲
}


/***************************************************************************************
 * @说明  	向环形队列写入数据
 * @参数  	unsigned char mydata : 要写入的数据
 * @参数 	RingBuff_t *ringbuff : 写入的环形队列
 * @返回值  无
 * @注		无
 ***************************************************************************************/
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

/***************************************************************************************
 * @说明  	向环形队列读取一个字节的数据
 * @参数  	unsigned char *rData : 读出的数据
 * @参数 	RingBuff_t *ringbuff : 需要读取的环形队列
 * @返回值  无
 * @注		无
 ***************************************************************************************/
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

/***************************************************************************************
 * @说明  	向环形队列写入一个数组的数据
 * @参数  	unsigned char *myStr : 要写入的数据
 * @参数 	RingBuff_t *ringbuff : 写入的环形队列
 * @返回值  无
 * @注		无
 ***************************************************************************************/
void WriteStr_RingBuff(unsigned char *myStr,RingBuff_t *ringbuff)
{
    unsigned char i;
    for(i = 0; myStr[i] != '\0'; i++)
    {
        Write_RingBuff(myStr[i],ringbuff);
    }
}


