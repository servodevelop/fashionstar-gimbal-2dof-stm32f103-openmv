#include "usart.h"


#if USART1_ENABLE
    uint8_t usart1SendBuf[USART_SEND_BUF_SIZE+1];
	uint8_t usart1RecvBuf[USART_RECV_BUF_SIZE+1];
	RingBufferTypeDef usart1SendRingBuf;
	RingBufferTypeDef usart1RecvRingBuf;
	Usart_DataTypeDef usart1;
	 
#endif

#if USART2_ENABLE
    uint8_t usart2SendBuf[USART_SEND_BUF_SIZE+1];
	uint8_t usart2RecvBuf[USART_RECV_BUF_SIZE+1];
	RingBufferTypeDef usart2SendRingBuf;
	RingBufferTypeDef usart2RecvRingBuf;
	Usart_DataTypeDef usart2;
#endif

#if USART3_ENABLE
    uint8_t usart3SendBuf[USART_SEND_BUF_SIZE+1];
	uint8_t usart3RecvBuf[USART_RECV_BUF_SIZE+1];
	RingBufferTypeDef usart3SendRingBuf;
	RingBufferTypeDef usart3RecvRingBuf;
	Usart_DataTypeDef usart3;
#endif

void Usart_Init(void){
    //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
    // 串口配置结构体
    USART_InitTypeDef USART_InitStructure;
    // 外部中断结构体
    NVIC_InitTypeDef NVIC_InitStructure;

    #if USART1_ENABLE
        // 赋值结构体usart指针
        usart1.pUSARTx = USART1;
		// 初始化缓冲区(环形队列)
		RingBuffer_Init(&usart1SendRingBuf,  USART_SEND_BUF_SIZE, usart1SendBuf);
		RingBuffer_Init(&usart1RecvRingBuf, USART_RECV_BUF_SIZE,  usart1RecvBuf);
		usart1.recvBuf = &usart1RecvRingBuf;
		usart1.sendBuf = &usart1SendRingBuf;
	
		// usart1.recvBuf = RingBuffer_Init(USART_RECV_BUF_SIZE);
		// usart1.sendBuf = RingBuffer_Init(USART_SEND_BUF_SIZE);
        // 使能USART时钟
        USART1_APBxClkCmd(USART1_CLK, ENABLE);
        // 使能GPIO时钟
        USART1_GPIO_APBxClkCmd(USART1_GPIO_CLK, ENABLE);
        // 配置串口USART的发送管脚
        GPIO_InitStructure.GPIO_Pin = USART1_TX_GPIO_PIN;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
        GPIO_Init(USART1_TX_GPIO_PORT, &GPIO_InitStructure);

        // 配置串口USART的接收管脚
        GPIO_InitStructure.GPIO_Pin = USART1_RX_GPIO_PIN;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
        GPIO_Init(USART1_RX_GPIO_PORT, &GPIO_InitStructure); 
		
		//USART 初始化设置
        USART_InitStructure.USART_BaudRate = USART1_BAUDRATE;//串口波特率
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
        USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
        USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
        USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
		
        // NVIC 配置串口外部中断
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 嵌套向量中断控制器组选择
        NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;//抢占优先级
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//子优先级
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
        NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化NVIC寄存器
        
        // 初始化串口
        USART_Init(USART1, &USART_InitStructure);
        //开启串口接收中断 IT: Interupt
        USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
		// 使能串口
        USART_Cmd(USART1, ENABLE);
    #endif
    #if USART2_ENABLE
		// 赋值结构体usart指针
        usart2.pUSARTx = USART2;
		// 初始化缓冲区(环形队列)
		RingBuffer_Init(&usart2SendRingBuf,  USART_SEND_BUF_SIZE, usart2SendBuf);
		RingBuffer_Init(&usart2RecvRingBuf, USART_RECV_BUF_SIZE,  usart2RecvBuf);
		usart2.recvBuf = &usart2RecvRingBuf;
		usart2.sendBuf = &usart2SendRingBuf;
		
        // 使能USART时钟
        USART2_APBxClkCmd(USART2_CLK, ENABLE);
        // 使能GPIO时钟
        USART2_GPIO_APBxClkCmd(USART2_GPIO_CLK, ENABLE);
        // 配置串口USART的发送管脚 TX
        GPIO_InitStructure.GPIO_Pin = USART2_TX_GPIO_PIN;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
        GPIO_Init(USART2_TX_GPIO_PORT, &GPIO_InitStructure);

        // 配置串口USART的接收管脚 RX
        GPIO_InitStructure.GPIO_Pin = USART2_RX_GPIO_PIN;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
        GPIO_Init(USART2_RX_GPIO_PORT, &GPIO_InitStructure);

        // Usar21 NVIC 配置串口外部中断
        NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
        NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化NVIC寄存器
        
        //USART 初始化设置
        USART_InitStructure.USART_BaudRate = USART2_BAUDRATE;//串口波特率
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
        USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
        USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
        USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
        // 初始化串口
        USART_Init(USART2, &USART_InitStructure);
        //开启串口接收中断 IT: Interupt
        USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
		// 使能串口
        USART_Cmd(USART2, ENABLE);
    #endif

    #if USART3_ENABLE
		// 赋值结构体usart指针
        usart3.pUSARTx = USART3;
        // 初始化缓冲区(环形队列)
		RingBuffer_Init(&usart3SendRingBuf,  USART_SEND_BUF_SIZE, usart3SendBuf);
		RingBuffer_Init(&usart3RecvRingBuf, USART_RECV_BUF_SIZE,  usart3RecvBuf);
		usart3.recvBuf = &usart3RecvRingBuf;
		usart3.sendBuf = &usart3SendRingBuf;
		
        // 使能USART时钟
        USART3_APBxClkCmd(USART3_CLK, ENABLE);
        // 使能GPIO时钟
        USART3_GPIO_APBxClkCmd(USART3_GPIO_CLK, ENABLE);
        // 配置串口USART的发送管脚 TX
        GPIO_InitStructure.GPIO_Pin = USART3_TX_GPIO_PIN;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
        GPIO_Init(USART3_TX_GPIO_PORT, &GPIO_InitStructure);

        // 配置串口USART的接收管脚 RX
        GPIO_InitStructure.GPIO_Pin = USART3_RX_GPIO_PIN;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
        GPIO_Init(USART3_RX_GPIO_PORT, &GPIO_InitStructure);

        // Usart NVIC 配置串口外部中断
        NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
        NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化NVIC寄存器
        
        //USART 初始化设置
        USART_InitStructure.USART_BaudRate = USART3_BAUDRATE;//串口波特率
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
        USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
        USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
        USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
        // 初始化串口
        USART_Init(USART3, &USART_InitStructure);
        //开启串口接收中断 IT: Interupt
        USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
		// 使能串口
        USART_Cmd(USART3, ENABLE);
    #endif

}

/* 发送单个字节 */
void Usart_SendByte(USART_TypeDef *pUSARTx, uint8_t ch)
{
    /* 发送一个字节到USART */
    USART_SendData(pUSARTx, ch);
	/* 等待发送寄存器为空 */
    while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);	
}

/* 发送8位的字节流 */
void Usart_SendByteArr(USART_TypeDef *pUSARTx, uint8_t *byteArr, uint16_t size){
    uint16_t bidx;
    for (bidx=0; bidx<size; bidx++){
        Usart_SendByte(pUSARTx, byteArr[bidx]);
    }
    /* 等待发送寄存器为空 */
    // while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);	
}

/* 发送字符串 */
void Usart_SendString(USART_TypeDef *pUSARTx, char *str){
    uint16_t sidx=0;
    do
    {
        Usart_SendByte(pUSARTx, (uint8_t)(*(str + sidx)));
        sidx++;
    } while(*(str + sidx) != '\0');
    /* 等待发送寄存器为空 */
	// USART_FLAG_TXE:  Transmit data register empty flag 发送数据寄存器为空
	// RESET = 0
	// SET = 1
    // while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);
}

// 将串口发送缓冲区的内容全部发出去
void Usart_SendAll(Usart_DataTypeDef *usart){
	uint8_t value;
	while(RingBuffer_GetByteUsed(usart->sendBuf)){
		value = RingBuffer_Pop(usart->sendBuf);
		// printf("Usart_SendAll pop: %d", value);
		Usart_SendByte(usart->pUSARTx, value);
	}
}
#if USART1_ENABLE
    // 定义串口中断处理函数
    void USART1_IRQHandler(void){
        uint8_t ucTemp;
	    if(USART_GetITStatus(USART1,USART_IT_RXNE)!=RESET)
	    {
			// 从串口读取一个字符
            ucTemp = USART_ReceiveData(USART1);
			// 新的字符添加到串口的环形缓冲队列中
			RingBuffer_Push(usart1.recvBuf, ucTemp);
        }
    }
	
	// 重定向c库函数printf到串口，重定向后可使用printf函数
	
	// int fputc(int ch, FILE *f)
	//{
	//	while((USART1->SR&0X40)==0){}
	//	/* 发送一个字节数据到串口 */
	//	USART_SendData(USART1, (uint8_t) ch);
	//	/* 等待发送完毕 */
	//	// while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET);		
	//	return (ch);
	//}

	// 重定向c库函数scanf到串口，重写向后可使用scanf、getchar等函数
	// int fgetc(FILE *f)
	//{
	//	/* 等待串口输入数据 */
	// 	while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
	// 
	// 	return (int)USART_ReceiveData(USART1);
	// }
#endif
	
#if USART2_ENABLE
	// 定义串口中断处理函数
    void USART2_IRQHandler(void){
        uint8_t ucTemp;
	    if(USART_GetITStatus(USART2,USART_IT_RXNE)!=RESET)
	    {
			// 从串口读取一个字符
            ucTemp = USART_ReceiveData(usart2.pUSARTx);
			// 新的字符添加到串口的环形缓冲队列中
            RingBuffer_Push(usart2.recvBuf, ucTemp);
        }
    }
#endif

#if USART3_ENABLE
    void USART3_IRQHandler(void){
        uint8_t ucTemp;
	    if(USART_GetITStatus(USART3,USART_IT_RXNE)!=RESET)
	    {
			// 从串口读取一个字符
            ucTemp = USART_ReceiveData(usart3.pUSARTx);
			// 新的字符添加到串口的环形缓冲队列中
            RingBuffer_Push(usart3.recvBuf, ucTemp);
        }
    }
#endif
