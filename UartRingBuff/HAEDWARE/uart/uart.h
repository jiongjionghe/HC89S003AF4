#ifndef __UART_H__
#define __UART_H__



#define  RINGBUFF_LEN          (60)     //�����������ֽ��� 60
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
    unsigned char guc_Uartflag;         // UART�жϱ�־λ
    unsigned char guc_Uartcnt;          // UART����ʹ��
    unsigned char guc_Uartbuf_a[20];    // ���ڴ�Ž��յ�һ֡��������
    unsigned char guc_Uartbuf_temp[20]; // ���ڽ��ܻ�����
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

