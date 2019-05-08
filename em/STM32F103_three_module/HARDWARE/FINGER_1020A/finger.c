
#include <string.h>
#include "delay.h"	
#include "usart2.h"

#include"finger.h"


// software serial #1: RX = digital pin 3, TX = digital pin 2
//ʹ�� 3 �ں� 2��������������
//#if ENABLE_DEBUG
//SoftwareSerial Finger_mySerial(3, 2);
//#endif
#define UART_BUF_LEN			8
#define BUF_N					8

unsigned char	rBuf[UART_BUF_LEN] =
{
	0
};


//���շ�����Ϣ
unsigned char	tBuf[UART_BUF_LEN] =
{
	0
};


//���������������
//unsigned char g_ucUartRxEnd; //���շ�����Ϣ������־
unsigned char	g_ucUartRxLen; //���շ�����Ϣ����
unsigned char	l_ucFPID; //�û����

//��ʼ��PA4Ϊ��������			
//��������Ӧ״̬(������Ӧʱ����ߵ�ƽ�ź�)
void FingerGPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); //ʹ��GPIOAʱ��

	//��ʼ����״̬����GPIOA
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;		//���� TXRX��ָ��������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;	//��������ģʽ
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);			//��ʼ��GPIO	

    

  /* ʹ��(����)���Ŷ�ӦIO�˿�ʱ��                         + */
  FINGER_POWER_RCC_CLOCKCMD(FINGER_POWER_RCC_CLOCKGPIO, ENABLE);

  GPIO_InitStructure.GPIO_Pin =   FINGER_POWER_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    //

  GPIO_Init(FINGER_POWER_GPIO, &GPIO_InitStructure);
}

void Finger_ClearRecvBuf(void)
{
     int i;

     USART2_RX_STA=0;        //����  ����Ҫ���㣬������Ϊ�Ǹ�ָ��ģ��һ�ϵ�ᷢ��һ�Ѷ�������

    for(i =0; i < USART2_MAX_RECV_LEN; i++)
     {
         USART2_RX_BUF[i] = 0;
     }
}
/*******************************************************************	
**���ܣ����ڷ���һ���ֽ�
**������
**���أ���									
*******************************************************************/
static void MYUSART_SendData(u8 data)
{
    Usart2_SendByte(data);
//	while ((USART2->SR & 0X40) == 0)
//		;

//	USART2->DR			= data;
}


/*******************************************************************	
**���ܣ����ڷ������� 
**������
**���أ���									
*******************************************************************/
void UartSend(unsigned char * Datar, unsigned char cLength)
{
	do 
	{
		MYUSART_SendData(* (Datar++));
	}
	while(--cLength != 0);
}


/*******************************************************************	
**���ܣ���ʱ����
**������
**���أ���									
*******************************************************************/
void Delay_ms(unsigned int ms)
{
	delay_ms(ms);
}


//�ж��жϽ��յ�������û��Ӧ���
//waittimeΪ�ȴ��жϽ������ݵ�ʱ�䣨��λ1ms��
//����ֵ�����ݰ��׵�ַ
static u8 * JudgeStr(u16 waittime)
{
	char *			data;
	u8				str[2];

	str[0] 				= 0xf5;
	str[1] 				= '\0';

	USART2_RX_STA		= 0;

	while (--waittime)
	{
		delay_ms(1);

		if (USART2_RX_STA & 0X8000) //���յ�һ������
		{
			USART2_RX_STA		= 0;
			data				= strstr((const char *) USART2_RX_BUF, (const char *) str);

			if (data)
				return (u8 *)
				data;
		}
	}

	return 0;
}


/*******************************************************************************
**���ܣ��ȴ����ݰ��������
**������
**���أ���
*******************************************************************************/
unsigned char WaitFpData(void)
{
	u8				i;

	u8 *			recv;							//

	recv				= JudgeStr(4000);

	if (recv)
	{
		for (i = 0; i < 8; i++)
		{
			rBuf[i] 			= recv[i];
		}

		return TRUE;
	}
	else 
	{
		return FALSE; //ָ��оƬû�з���
	}
}


/*******************************************************************************
**����: ����У��ֵ
**����: Ҫ���͵�����ָ���ַ
**����: У��ֵ
*******************************************************************************/
unsigned char CmdGenCHK(unsigned char wLen, unsigned char * ptr)
{
	unsigned char	i, temp = 0;

	for (i = 0; i < wLen; i++)
	{
		temp				^= * (ptr + i);
	}

	return temp;
}


/*******************************************************************************
**����: ���Ϳ���ָ��оƬָ��
**����: wLen ���ݳ���
	cpPara ���͵�����
**���أ�void
*******************************************************************************/
void UART_SendPackage(unsigned char wLen, unsigned char * ptr)
{
	unsigned int	i	= 0, len = 0;

	tBuf[0] 			= DATA_START;				//ָ���

	for (i = 0; i < wLen; i++) // data in packet 
	{
		tBuf[1 + i] 		= * (ptr + i);
	}

	tBuf[wLen + 1]		= CmdGenCHK(wLen, ptr); 	//Generate checkout data
	tBuf[wLen + 2]		= DATA_END;
	len 				= wLen + 3;

	g_ucUartRxLen		= len;

    Finger_ClearRecvBuf();
	UartSend(tBuf, len);
}


/*******************************************************************************
**���ܣ�������Ϣ����
**������ cmd ��ͬ���ͬ����
**���أ�������
*******************************************************************************/
unsigned char Check_Package(unsigned char cmd)
{
	unsigned char	flag = FALSE;

	if (!WaitFpData())
		return flag; //�ȴ����շ�����Ϣ

	if (rBuf[0] != DATA_START)
		return flag;

	if (rBuf[1] != cmd)
		return flag;

	if (rBuf[6] != CmdGenCHK(g_ucUartRxLen - 3, &rBuf[1]))
		return flag;

	switch (cmd)
	{
		case CMD_ENROLL1:
		case CMD_ENROLL2:
		case CMD_ENROLL3:
			if (ACK_SUCCESS == rBuf[4])
				flag = TRUE;
			else if (ACK_USER_EXIST == rBuf[4])
			{
				//	  Spk_HaveUser();
				//Delay_ms(1500);
				flag = TRUE;
				//return 22; //�û��Ѿ����ڣ�����22
			}

			break;

		case CMD_DELETE: //ɾ��ָ�����ָ��
		case CMD_CLEAR: //�������ָ��
		case CMD_IDENTIFY: //1:1�ȶ�
			if (ACK_SUCCESS == rBuf[4])
				flag = TRUE;

			break;

		case CMD_USERNUMB: //ȡ�û�����
			if (ACK_SUCCESS == rBuf[4])
			{
				flag				= TRUE;
				l_ucFPID			= rBuf[3];
			}

			break;

		case CMD_SEARCH: //1:N�ȶ�
			if ((1 == rBuf[4]) || (2 == rBuf[4]) || (3 == rBuf[4]))
			{
				flag				= TRUE;
				l_ucFPID			= rBuf[3];
			}

			break;

		case CMD_MODE_SET: //ָ��¼��ģʽ����
			if (ACK_SUCCESS == rBuf[4])
			{
				flag				= TRUE;
			}

			break;

		default:
			break;
	}

#if ENABLE_DEBUG

	//	  Finger_mySerial.print("packcheck cmd:  return: ");
	//	Finger_mySerial.println(cmd);
	//	Finger_mySerial.println(flag);
#endif

	return flag;
}


/*******************************************************************************
**���ܣ���CharBuffer1 ��CharBuffer2 �е������ļ����������򲿷�ָ�ƿ�
**������
**���أ���
*******************************************************************************/
void FP_Search(void)
{
	unsigned char	buf[BUF_N];

	*buf				= CMD_SEARCH;				//1:N�ȶ�
	* (buf + 1) 		= 0x00;
	* (buf + 2) 		= 0x00;
	* (buf + 3) 		= 0x00;
	* (buf + 4) 		= 0x00;

	UART_SendPackage(5, buf);
}


/*******************************************************************************
**���ܣ���� flash ָ�ƿ�
**������
**���أ���
*******************************************************************************/
void FP_Clear(void)
{
	unsigned char	buf[BUF_N];

	*buf				= CMD_CLEAR;
	* (buf + 1) 		= 0x00;
	* (buf + 2) 		= 0x00;
	* (buf + 3) 		= 0x00;
	* (buf + 4) 		= 0x00;

	UART_SendPackage(5, buf);
}


/*******************************************************************************
**���ܣ�ɾ��ָ�����ָ��
**������u_id
**���أ�void
*******************************************************************************/
void FP_Delete(unsigned int u_id)
{
	unsigned char	buf[BUF_N];

	*buf				= CMD_DELETE;
	* (buf + 1) 		= u_id >> 8;
	* (buf + 2) 		= u_id & 0xff;
	* (buf + 3) 		= 0x00;
	* (buf + 4) 		= 0x00;
	UART_SendPackage(5, buf);
}


/*******************************************************************************
**���ܣ�1:1�ȶ�
**������u_id
**���أ�void
*******************************************************************************/
void FP_Identify(unsigned int u_id)
{
	unsigned char	buf[BUF_N];

	*buf				= CMD_IDENTIFY;
	* (buf + 1) 		= u_id >> 8;
	* (buf + 2) 		= u_id & 0xff;
	* (buf + 3) 		= 0x00;
	* (buf + 4) 		= 0x00;
	UART_SendPackage(5, buf);
}


/*******************************************************************************
**ע��ָ��
**��������ָ��ע��һ��ָ��ģ��
**������UserID ָ�ƺ�
*******************************************************************************/
void Enroll_Step1(unsigned int u_id)
{
	unsigned char	buf[BUF_N];

	*buf				= CMD_ENROLL1;
	* (buf + 1) 		= u_id >> 8;
	* (buf + 2) 		= u_id & 0xff;
	* (buf + 3) 		= 1;
	* (buf + 4) 		= 0x00;

	UART_SendPackage(5, buf);
}


/*******************************************************************************
**ע��ָ��
**��������ָ��ע��һ��ָ��ģ��
**������UserID ָ�ƺ�
*******************************************************************************/
void Enroll_Step2(unsigned int u_id)
{
	unsigned char	buf[BUF_N];

	*buf				= CMD_ENROLL2;
	* (buf + 1) 		= u_id >> 8;
	* (buf + 2) 		= u_id & 0xff;
	* (buf + 3) 		= 1;
	* (buf + 4) 		= 0x00;

	UART_SendPackage(5, buf);
}


/*******************************************************************************
**ע��ָ��
**��������ָ��ע��һ��ָ��ģ��
**������UserID ָ�ƺ�
*******************************************************************************/
void Enroll_Step3(unsigned int u_id)
{
	unsigned char	buf[BUF_N];

	*buf				= CMD_ENROLL3;
	* (buf + 1) 		= u_id >> 8;
	* (buf + 2) 		= u_id & 0xff;
	* (buf + 3) 		= 1;
	* (buf + 4) 		= 0x00;

	UART_SendPackage(5, buf);
}


/*******************************************************************************
**ע��ָ�ƣ���һ�ΰ���
**��������ָ��ע��һ��ָ��ģ�壬1020AС���󴫸�������ɼ�6�Ρ�
**������UserID ָ�ƺ�
*******************************************************************************/
unsigned char Finger_Enroll(unsigned int u_id)
{
	Enroll_Step1(u_id);

	if (FALSE == Check_Package(CMD_ENROLL1))
		return FALSE;

	Delay_ms(100);

	Enroll_Step2(u_id);

	if (FALSE == Check_Package(CMD_ENROLL2))
		return FALSE;

	Delay_ms(100);

	Enroll_Step2(u_id);

	if (FALSE == Check_Package(CMD_ENROLL2))
		return FALSE;

	Enroll_Step2(u_id);

	if (FALSE == Check_Package(CMD_ENROLL2))
		return FALSE;

	Delay_ms(100);

	Enroll_Step2(u_id);

	if (FALSE == Check_Package(CMD_ENROLL2))
		return FALSE;

	Delay_ms(100);

	Enroll_Step3(u_id);
	return Check_Package(CMD_ENROLL3);
}


/*******************************************************************************
**���ָ��
**
**������UserID ָ�ƺ�
*******************************************************************************/
unsigned char Finger_Clear(void)
{
	FP_Clear();

	//	if(FALSE == WaitFpData())return FALSE;
	return Check_Package(CMD_CLEAR);
}


/*******************************************************************************
**ɾ��ָ��ָ��
**
**������UserID ָ�ƺ�
*******************************************************************************/
unsigned char Finger_Delete(unsigned int u_id)
{
	FP_Delete(u_id);

	//	if(FALSE == WaitFpData())return FALSE;
	return Check_Package(CMD_DELETE);
}


/*******************************************************************************
**��ȡ�û�����
**
**����?N
*******************************************************************************/
int Finger_Read(void)
{
	unsigned char	buf[BUF_N];

	*buf				= CMD_USERNUMB;
	* (buf + 1) 		= 0x00;
	* (buf + 2) 		= 0x00;
	* (buf + 3) 		= 0x00;
	* (buf + 4) 		= 0x00;
	UART_SendPackage(5, buf);

	if (Check_Package(CMD_USERNUMB))
	{
		return l_ucFPID;
	}

	return - 1; //����
}


/*******************************************************************************
**��ȡ�û�����
**
**����
*******************************************************************************/
unsigned char Finger_Search(void)
{
	FP_Search();
	return Check_Package(CMD_SEARCH);
}


/*******************************************************************************
**����ָ��¼��ģʽ
**��Ӧ������������ڵ�״̬���������Ϊ���յ�ָ��֮ǰ��״̬�������ԣ���Ҫ�ٷ�һ�Σ��鵽�Ĳ����ϴ�ָ����ĺ��
**����?N
*******************************************************************************/
int Finger_ModeSet(void)
{
	unsigned char	buf[BUF_N];

	*buf				= CMD_MODE_SET;
	* (buf + 1) 		= 0x00;
	* (buf + 2) 		= 0x00; 					// 0�������ظ�1����ֹ�ظ�
	* (buf + 3) 		= 0x00; 					//0 �����µ����ģʽ 1��ȡ
	* (buf + 4) 		= 0x00;

    
	UART_SendPackage(5, buf);

	if (Check_Package(CMD_MODE_SET))
	{
	    if(rBuf[3] == * (buf + 2)) //Ӧ������������ڵ�״̬���������Ϊ���յ�ָ��֮ǰ��״̬�������ԣ���Ҫ�ٷ�һ�Σ��鵽�Ĳ����ϴ�ָ����ĺ��
		    return 0;
        else
            return -1;
	}

	return - 1; //����
}



