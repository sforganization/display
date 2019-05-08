
#include <string.h>
#include "delay.h"	
#include "usart2.h"

#include"finger.h"
#include "machine.h"
#include "usart3.h"

#include "stm32f10x.h"

#include "stm32f10x_conf.h"

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

//��ʼ��Ϊ��������			
//��������Ӧ״̬(������Ӧʱ����ߵ�ƽ�ź� һ˲�䣬����ʶ���˻�Ҫ��һ��IO��Ϊ��Դ����)
void FingerGPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	FINGER_TOUCH_RCC_CLOCKCMD(FINGER_TOUCH_RCC_CLOCKGPIO, ENABLE); //ʹ��GPIOʱ��

	//��ʼ����״̬����GPIOA
	GPIO_InitStructure.GPIO_Pin = FINGER_TOUCH_PIN; //
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;	//��������ģʽ
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);			//��ʼ��GPIO	



	/* ʹ��(����)���Ŷ�ӦIO�˿�ʱ�� 						+ */
	FINGER_POWER_RCC_CLOCKCMD(FINGER_POWER_RCC_CLOCKGPIO, ENABLE);

	GPIO_InitStructure.GPIO_Pin = FINGER_POWER_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //

	GPIO_Init(FINGER_POWER_GPIO, &GPIO_InitStructure);
}


void Finger_ClearRecvBuf(void)
{
	int 			i;

	USART2_RX_STA		= 0;						//����  ����Ҫ���㣬������Ϊ�Ǹ�ָ��ģ��һ�ϵ�ᷢ��һ�Ѷ�������

	for (i = 0; i < USART2_MAX_RECV_LEN; i++)
	{
		USART2_RX_BUF[i]	= 0;
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

	str[0]				= 0xf5;
	str[1]				= '\0';

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

	//	if (!WaitFpData())
	//		return flag; //�ȴ����շ�����Ϣ
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
			if (F1020_ACK_SUCCESS == rBuf[4])
				flag = TRUE;
			else if (F1020_ACK_USER_EXIST == rBuf[4])
			{
				//	  Spk_HaveUser();
				//Delay_ms(1500);
				flag				= TRUE;

				//return 22; //�û��Ѿ����ڣ�����22
			}

			break;

		case CMD_DELETE: //ɾ��ָ�����ָ��
		case CMD_CLEAR: //�������ָ��
		case CMD_IDENTIFY: //1:1�ȶ�
			if (F1020_ACK_SUCCESS == rBuf[4])
				flag = TRUE;

			break;

		case CMD_USERNUMB: //ȡ�û�����
			if (F1020_ACK_SUCCESS == rBuf[4])
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
			if ((F1020_ACK_SUCCESS == rBuf[4]) && (rBuf[3]) == F1020_MODE_REC) //����Ϊ���ظ�ģʽ
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
**���ܣ���CharBuffer1 ��CharBuffer2 �е������ļ����� 1 : 1 ƥ��
**������
**���أ���
*******************************************************************************/
void FP_Search_One(unsigned int u_id)
{
	unsigned char	buf[BUF_N];

	*buf				= CMD_IDENTIFY; 			//1:1�ȶ�
	* (buf + 1) 		= u_id >> 8;
	* (buf + 2) 		= u_id & 0xff;
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

	//	if (Check_Package(CMD_USERNUMB))
	//	{
	//		return l_ucFPID;
	//	}
	return 0; //
}


/*******************************************************************************
**��ȡ�û�����
**
**����
*******************************************************************************/
unsigned char Finger_Search(int * userID)
{
	FP_Search();
	*userID 			= l_ucFPID;
	return Check_Package(CMD_SEARCH);
}


/*******************************************************************************
**����ָ��¼��ģʽ
**��Ӧ������������ڵ�״̬���������Ϊ���յ�ָ��֮ǰ��״̬�������ԣ���Ҫ�ٷ�һ?
	?���鵽�Ĳ����ϴ�ָ����ĺ��
**����?N
*******************************************************************************/
int Finger_ModeSet(void)
{
	unsigned char	buf[BUF_N];

	*buf				= CMD_MODE_SET;
	* (buf + 1) 		= 0x00;
	* (buf + 2) 		= F1020_MODE_REC; 					// 0�������ظ�1����ֹ�ظ�
	* (buf + 3) 		= 0x00; 					//0 �����µ����ģʽ 1��ȡ
	* (buf + 4) 		= 0x00;


	UART_SendPackage(5, buf);

	//	if (Check_Package(CMD_MODE_SET))
	//	{
	//		if(rBuf[3] == * (buf + 2)) //Ӧ������������ڵ�״̬���������Ϊ���յ�ָ��֮ǰ��״̬�������ԣ���Ҫ�ٷ�һ�Σ��鵽�Ĳ����ϴ�ָ����ĺ��
	//			return 0;
	//		  else
	//			  return -1;
	//	}
	return 0; //
}


/*******************************************************************************
* ����: 
* ����: ʵ�ⷢ�͵�ָ��ģ����ʱ5ms
* �β�:	
* ����: ��
* ˵��: 
*******************************************************************************/

int Finger_Event(char opt, void * data)
{
	u8				str[2];
    static  u8 A1020_cmd;
	int 			i;
	char *			pRev;
	static FingerState_T FingerState = FINGER_STATE_INIT;
	static FingerSub_T fingerSubState = FINGER_SUB_INIT;
	FingerData_t *	pFingerData;
	PollEvent_t *	pEvent;

    static vu32  u32TimeSave = 0;

	str[0]				= 0xf5;
	str[1]				= '\0';

	pEvent				= data;
	pFingerData 		= (FingerData_t *)
	pEvent->MachineCtrl.pData;

	switch (FingerState)
	{
		case FINGER_STATE_INIT:
			pEvent->MachineRun.bFinish = FALSE;

			switch (opt)
			{
				case FINGER_MODE_SET:
#if FINGER_DEBUG
                    u3_printf("finger  ģʽ����\n");
#endif
					Finger_ModeSet();
                    A1020_cmd = CMD_MODE_SET;
					break;

				case FINGER_SEARCHING: //1: N
#if FINGER_DEBUG
                    u3_printf("finger  ����ָ��\n");
#endif

//#if FINGER_DEBUG
//	GPIO_SetBits(GPIOC, GPIO_Pin_13);
//#endif
					FP_Search();
//#if FINGER_DEBUG
//	GPIO_ResetBits(GPIOC, GPIO_Pin_13);
//                    
//#endif
                    A1020_cmd = CMD_SEARCH;
					break;

				case FINGER_READ:
#if FINGER_DEBUG
                    u3_printf("finger  ��ȡָ�Ƹ���\n");
#endif
					Finger_Read();
                    A1020_cmd = CMD_USERNUMB;
					break;

				case FINGER_ENROLL: //ע��ָ��
				    if(pEvent->MachineRun.u32LastTime - u32TimeSave < 100)   //��ʱ
                        break;
                    
#if FINGER_DEBUG
                                        u3_printf("finger  ע��ָ�ƣ� %d \n", fingerSubState);
#endif

                    u32TimeSave = pEvent->MachineRun.u32LastTime;
					switch (fingerSubState)
					{
						case FINGER_SUB_INIT:
							Enroll_Step1(pFingerData->u8FingerID);
                            A1020_cmd = CMD_ENROLL1;
							break;

						case FINGER_SUB_STEP1:
						case FINGER_SUB_STEP2:
						case FINGER_SUB_STEP3:
						case FINGER_SUB_STEP4:
							A1020_cmd= CMD_ENROLL2;
							Enroll_Step2(pFingerData->u8FingerID);
							break;

						case FINGER_SUB_STEP5:
							A1020_cmd = CMD_ENROLL3;
							Enroll_Step3(pFingerData->u8FingerID);
							break;

						default:
							break;
					}
                    fingerSubState++;
					break;

				case FINGER_IDENTIFY: //��ָ֤�� 1: 1
					A1020_cmd = CMD_IDENTIFY;
					break;

				case FINGER_CLEAR: //���ָ��
#if FINGER_DEBUG
                    u3_printf("finger  ��� \n");
#endif
					A1020_cmd = CMD_CLEAR;
					FP_Clear();
					break;

				case FINGER_DELETE: //ɾ��ָ��ָ��
					FP_Delete(pFingerData->u8FingerID);
					A1020_cmd = CMD_DELETE;
					break;
			}

//			pEvent->MachineCtrl.u32Timeout = FINGER_REV_TIME_OUT;
			FingerState = FINGER_STATE_WAIT; //��ʱ�ȴ�����״̬
			break;

		case FINGER_STATE_WAIT: // �ȴ�hal��״̬����
			if (USART2_RX_STA & 0X8000) //���յ�һ������
			{
				USART2_RX_STA		= 0;
				pRev				= strstr((const char *) USART2_RX_BUF, (const char *) str);

				if (data)
				{

					for (i = 0; i < 8; i++)
					{
						rBuf[i] 			= pRev[i];
					}

					FingerState 		= FINGER_STATE_CHECK;
				}
			}
			else if (pEvent->MachineRun.u32LastTime - pEvent->MachineRun.u32StartTime >
				 pEvent->MachineCtrl.u32Timeout) //״̬����ʱ�˳�
			{
			    pEvent->MachineRun.u16RetVal = EVENT_TIME_OUT;
				FingerState 		= FINGER_STATE_EXIT;
			}

			break;

		case FINGER_STATE_CHECK:
//#if FINGER_DEBUG
//	GPIO_SetBits(GPIOC, GPIO_Pin_13);
//#endif
			pEvent->MachineRun.u16RetVal = Check_Package(A1020_cmd); //�����������ظ�ģʽ���ж� 

			if ((opt == FINGER_ENROLL) && (fingerSubState <= FINGER_SUB_STEP5)) //ע��ָ��Ҫ���
			{
				if (pEvent->MachineRun.u16RetVal == TRUE)
					FingerState = FINGER_STATE_INIT;
				else
					FingerState = FINGER_STATE_EXIT;
			}
			else
			{
				FingerState 		= FINGER_STATE_EXIT;
			}
            
#if FINGER_DEBUG
                GPIO_ResetBits(GPIOC, GPIO_Pin_13);
                                A1020_cmd = CMD_SEARCH;
#endif

			break;

		case FINGER_STATE_EXIT:
			pEvent->MachineRun.bFinish = TRUE;
			pFingerData->u8FingerID = l_ucFPID;
			fingerSubState = FINGER_SUB_INIT;
			FingerState = FINGER_STATE_INIT; //Ϊ��һ�γ�ʼ��״̬��
			break;

		default:
			break;
	}

	return 0;
}


