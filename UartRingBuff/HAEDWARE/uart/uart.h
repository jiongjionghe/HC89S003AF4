#ifndef __UART_H__
#define __UART_H__



#define  RINGBUFF_LEN          (60)     //定义最大接收字节数 60
#define  RINGBUFF_OK           1
#define  RINGBUFF_ERR          0

typedef struct
{
    unsigned int  Head;           
    unsigned int  Tail;
    unsigned int  Lenght;
    unsigned char Ring_data[RINGBUFF_LEN];
    unsigned char FlagByte;  
}RingBuff_t;


typedef struct UartRec_StructTypedef {
    unsigned char guc_Uartflag;         // UART判断标志位
    unsigned char guc_Uartcnt;          // UART计数使用
    unsigned char guc_Uartbuf_a[20];    // 用于存放接收的一帧完整命令
    unsigned char guc_Uartbuf_temp[20]; // 串口接受缓存区
} UartRec_Struct;

typedef union test_float {
    float all;
    struct {
        unsigned char u8Byte[4];
    } stByte;
} FLOAT;


void RingBuff_Init(RingBuff_t *ringbuff);
unsigned char Write_RingBuff(unsigned char mydata,RingBuff_t *ringbuff);
unsigned char Read_RingBuff(unsigned char *rData,RingBuff_t *ringbuff);
void WriteStr_RingBuff(unsigned char *myStr,RingBuff_t *ringbuff);

#endif

