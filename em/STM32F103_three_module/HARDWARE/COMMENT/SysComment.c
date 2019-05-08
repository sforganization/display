
#include <stdio.h>

#include <string.h>

#include "usart1.h" 
#include "usart3.h" 
#include "delay.h"

#include "SysComment.h"
#include "stm32f10x_it.h"
#include "stm32f10x_iwdg.h"
#include "san_flash.h"

#include "as608.h"


#define SLAVE_WAIT_TIME 		3000	//3s �ӻ�Ӧ��ʱ��

#define LOCK_SET_L_TIME 		50	  //�͵�ƽʱ�� 25 ~ 60ms
#define LOCK_SET_H_TIME 		50	  //�ߵ�ƽʱ��
#define LOCK_SET_OFF_TIME		1200	  //����Ƭ����Ӧʱ�䣬�������֮����ת


#define MOTO_UP_TIME 		    800	        //�������ʱ��
#define MOTO_DOWN_TIME 		    1500	    //����½�ʱ��




#define MOTO_TRUN_OVER 		    500	        //MOTO 3  �����ƽ��תʱ��


#define MOTO_ANGLE_TIME         (750 * PWM_PERIOD / PWM_POSITIVE)  //���ת��һ���Ƕȵ�ʱ��
#define MOTO_DEVIATION_TIME 	700  //���ת��ʱ��ƫ�� 200ms ����ƫ��������дflashʱ����Ϊдflash��ֹһ�� flash����������Ķ�ȡ���������жϣ�
//ע�⣬���ڴ��ڽ��ջ�ռ��һ����ʱ�䣬�������ʱҪ��Ӧ�ӳ����ر��ǵ���׿����ֹͣ���������ʱ��������ڼ�����ת����תʱ�򷢣�����Ѿ�������һ�Σ���������Ҫ��Ӧ�ӳ�ʱ��
//��Ϊ

#define LOCK_OPEN_TIME 		    500	        //�������͵�ƽʱ��


static bool  g_bSensorState = TRUE;

typedef enum 
{
    ACK_SUCCESS = 0,									//�ɹ� 
    ACK_FAILED, 										//ʧ��
    ACK_AGAIN,											//�ٴ�����
    ACK_ERROR,											//�������
    ACK_DEF = 0xFF, 
} Finger_Ack;


//�ڲ�����
static vu16 	mDelay;
u8				g_GlassSendAddr = 0; //Ҫ���͵��ֱ��ַ
u8				g_FingerAck = 0; //Ӧ����������
u8				g_AckType = 0; //Ӧ������

char			priBuf[120] =
{
	0
};


u32 			g_au32DataSaveAddr[6] =
{
//103ze 512K
//	0x0807D000, 
//	0x0807D800, 
//	0x0807E000, 
//	0x0807E800, 
//	0x0807F000, 
//	0x0807F800

//103c8  64K
	0X0800E800, 
	0X0800EC00, 
	0x0800F000, 
	0x0800F400, 
	0x0800F800, 
	0x0800FC00
};


vu32			g_au16MoteTime[][3] =
{
	//{
	//	MOTO_TIME_OFF, 0, 0
	//},    
	{
		MOTO_TIME_TPD, 43200000, 43200000
	},
	{
		MOTO_TIME_650, 120000, 942000
	},
	{
		MOTO_TIME_750, 120000, 800000
	},
	{
		MOTO_TIME_850, 120000, 693000
	},
	{
		MOTO_TIME_1000, 120000, 570000
	},
	{
		MOTO_TIME_1950, 120000, 234000
	},
    {
        MOTO_TIME_ANGLE, MOTO_ANGLE_TIME, 2
    },
};


//�ڲ�����
void SysTickConfig(void);


/*******************************************************************************
* ����: SysTick_Handler
* ����: ϵͳʱ�ӽ���1MS
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void SysTick_Handler(void)
{
	static u16		Tick_1S = 0;

	mSysTick++;
	mSysSec++;
	mTimeRFRX++;

	if (mDelay)
		mDelay--;

	if (++Tick_1S >= 1000)
	{
		Tick_1S 			= 0;

		if (mSysIWDGDog)
		{
			IWDG_ReloadCounter();					/*ιSTM32����Ӳ����*/

			if ((++mSysSoftDog) > 5) /*��system  DOG 2S over*/
			{
				mSysSoftDog 		= 0;
				NVIC_SystemReset();
			}
		}
	}
}


/*******************************************************************************
* ����: 
* ����: 
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void DelayUs(uint16_t nCount)
{
	u32 			del = nCount * 5;

	//48M 0.32uS
	//24M 0.68uS
	//16M 1.02us
	while (del--)
		;
}


/*******************************************************************************
* ����: 
* ����: 
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void DelayMs(uint16_t nCount)
{
	unsigned int	ti;

	for (; nCount > 0; nCount--)
	{
		for (ti = 0; ti < 4260; ti++)
			; //16M/980-24M/1420 -48M/2840
	}
}


/*******************************************************************************
* ����: Strcpy()
* ����: 
* �β�:		
* ����: ��
* ˵��: 
******************************************************************************/
void Strcpy(u8 * str1, u8 * str2, u8 len)
{
	for (; len > 0; len--)
	{
		*str1++ 			= *str2++;
	}
}


/*******************************************************************************
* ����: Strcmp()
* ����: 
* �β�:		
* ����: ��
* ˵��: 
******************************************************************************/
bool Strcmp(u8 * str1, u8 * str2, u8 len)
{
	for (; len > 0; len--)
	{
		if (*str1++ != *str2++)
			return FALSE;
	}

	return TRUE;
}


/*******************************************************************************
* ����: Sys_DelayMS()
* ����: ϵͳ�ӳٺ���
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void Sys_DelayMS(uint16_t nms)
{
	mDelay				= nms + 1;

	while (mDelay != 0x0)
		;
}


/*******************************************************************************
* ����: Sys_LayerInit
* ����: ϵͳ��ʼ��
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void Sys_LayerInit(void)
{
	SysTickConfig();

	mSysSec 			= 0;
	mSysTick			= 0;
	SysTask.mUpdate 	= TRUE;
}


/*******************************************************************************
* ����: 
* ����: 
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void Sys_IWDGConfig(u16 time)
{
	/* д��0x5555,�����������Ĵ���д�빦�� */
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

	/* ����ʱ�ӷ�Ƶ,40K/64=0.625K()*/
	IWDG_SetPrescaler(IWDG_Prescaler_64);

	/* ι��ʱ�� TIME*1.6MS .ע�ⲻ�ܴ���0xfff*/
	IWDG_SetReload(time);

	/* ι��*/
	IWDG_ReloadCounter();

	/* ʹ�ܹ���*/
	IWDG_Enable();
}

/*******************************************************************************
* ����: Sys_IWDGReloadCounter
* ����: 
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void Sys_IWDGReloadCounter(void)
{
	mSysSoftDog 		= 0;						//ι��
}


/*******************************************************************************
* ����: 
* ����: 
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void SysTickConfig(void)
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);

	/* Setup SysTick Timer for 1ms interrupts  */
	if (SysTick_Config(SystemCoreClock / 1000))
	{
		/* Capture error */
		while (1)
			;
	}

	/* Configure the SysTick handler priority */
	NVIC_SetPriority(SysTick_IRQn, 0x0);

#if (								SYSINFOR_PRINTF == 1)
	printf("SysTickConfig:Tick=%d/Second\r\n", 1000);
#endif
}


/*******************************************************************************
* ����: 
* ����: 
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void LedInit(void) //
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(LED_RCC_CLOCKGPIO, ENABLE);

	GPIO_InitStructure.GPIO_Pin = LED_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(LED_GPIO, &GPIO_InitStructure);
	GPIO_SetBits(LED_GPIO, LED_PIN);
}


/*******************************************************************************
* ����: 
* ����: 
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void FingerTouchTask(void)
{
	SearchResult seach;
	u8 ensure;
	static u8 u8MatchCnt = 0;						//ƥ��ʧ�ܴ�����Ĭ��ƥ��MATCH_FINGER_CNT�� 

	if ((SysTask.bEnFingerTouch == TRUE))
	{
		switch (SysTask.TouchState)
		{
			case TOUCH_MATCH_INIT:
				if (PS_Sta) //��ָ�ư���
				{
					SysTask.nWaitTime	= 10;		//һ��ʱ���ټ��
					SysTask.TouchState	= TOUCH_MATCH_AGAIN;
					u8MatchCnt			= 0;
				}

				break;

			case TOUCH_MATCH_AGAIN:
				if (SysTask.nWaitTime == 0) //��ָ�ư���
				{
					if (PS_Sta) //��ָ�ư���
					{
						ensure				= PS_GetImage();

						if (ensure == 0x00) //���������ɹ�
						{
							//PS_ValidTempleteNum(&u16FingerID); //����ָ�Ƹ���
							ensure				= PS_GenChar(CharBuffer1);

							if (ensure == 0x00) //���������ɹ�
							{
								ensure				= PS_Search(CharBuffer1, 0, FINGER_MAX_CNT, &seach);

								if (ensure == 0) //ƥ��ɹ�
								{
									SysTask.u16FingerID = seach.pageID;
									SysTask.nWaitTime	= SysTask.nTick + 2000; //��ʱһ��ʱ�� �ɹ�Ҳ����һֱ��
									SysTask.TouchSub	= TOUCH_SUB_INIT;
									SysTask.TouchState	= TOUCH_MATCH_INIT; //����ƥ��

									//								  SysTask.bEnFingerTouch = FALSE;	 //�ɹ������ָֹ��ģ������
									//ƥ��ɹ������͵�ƽ��ˣ�Ӧ��
									g_FingerAck 		= ACK_SUCCESS;
									g_AckType			= FINGER_ACK_TYPE;

									if (SysTask.SendState == SEND_IDLE)
										SysTask.SendState = SEND_TABLET_FINGER;
								}
								else if (u8MatchCnt >= MATCH_FINGER_CNT) // 50 * 3 ��Լ200MS��
								{
									//								  SysTask.bEnFingerTouch = FALSE;	 //�ɹ������ָֹ��ģ������
									//ƥ��ʧ�ܣ����͵�ƽ��ˣ�Ӧ��
									g_FingerAck 		= ACK_FAILED;
									g_AckType			= FINGER_ACK_TYPE;

									if (SysTask.SendState == SEND_IDLE)
										SysTask.SendState = SEND_TABLET_FINGER;

									SysTask.TouchState	= TOUCH_MATCH_INIT; //����ƥ��
									SysTask.nWaitTime	= SysTask.nTick + 300; //��ʱһ��ʱ�� ����ֳ�ʶ���ˣ�������һ���˼�⣬
								}
							}
						}

						if (ensure) //ƥ��ʧ��
						{
							SysTask.nWaitTime	= SysTask.nTick + 10; //��ʱһ��ʱ�� �����⵽��Ӧ����ͬһ���˵�ָ��
							u8MatchCnt++;
						}
					}
				}

				break;

			case TOUCH_ADD_USER:
				{
					switch (SysTask.TouchSub)
					{
						case TOUCH_SUB_INIT: // *
							if (SysTask.nWaitTime == 0)
							{
								SysTask.TouchSub	= TOUCH_SUB_ENTER;
							}

							break;

						case TOUCH_SUB_ENTER: //
							if ((PS_Sta) && (SysTask.nWaitTime == 0)) //�����ָ�ư���
							{
								ensure				= PS_GetImage();

								if (ensure == 0x00)
								{
									ensure				= PS_GenChar(CharBuffer1); //��������

									if (ensure == 0x00)
									{
										SysTask.nWaitTime	= 3000; //��ʱһ��ʱ����ȥ�ɼ�
										SysTask.TouchSub	= TOUCH_SUB_AGAIN; //�����ڶ���				

										//Please press	agin
										//�ɹ������͵�ƽ��ˣ�Ӧ������ٰ�һ��
										g_FingerAck 		= ACK_AGAIN;
										g_AckType			= FINGER_ACK_TYPE;

										if (SysTask.SendState == SEND_IDLE)
											SysTask.SendState = SEND_TABLET_FINGER;

									}
								}
							}

							break;

						case TOUCH_SUB_AGAIN:
							if ((PS_Sta) && (SysTask.nWaitTime == 0)) //�����ָ�ư���
							{
								ensure				= PS_GetImage();

								if (ensure == 0x00)
								{
									ensure				= PS_GenChar(CharBuffer2); //��������

									if (ensure == 0x00)
									{
										ensure				= PS_Match(); //�Ա�����ָ��

										if (ensure == 0x00) //�ɹ�
										{

											ensure				= PS_RegModel(); //����ָ��ģ��

											if (ensure == 0x00)
											{

												ensure				= PS_StoreChar(CharBuffer2, SysTask.u16FingerID); //����ģ��

												if (ensure == 0x00)
												{
													SysTask.TouchSub	= TOUCH_SUB_WAIT;
													SysTask.nWaitTime	= 3000; //��ʱһ��ʱ���˳�

													//													  SysTask.bEnFingerTouch = FALSE;	 //�ɹ������ָֹ��ģ������
													//�ɹ������͵�ƽ��ˣ�Ӧ��
													g_FingerAck 		= ACK_SUCCESS;
													g_AckType			= FINGER_ACK_TYPE;

													if (SysTask.SendState == SEND_IDLE)
														SysTask.SendState = SEND_TABLET_FINGER;
												}
												else 
												{
													//ƥ��ʧ�ܣ����͵�ƽ��ˣ�Ӧ��
													SysTask.TouchSub	= TOUCH_SUB_ENTER;
													SysTask.nWaitTime	= 3000; //��ʱһ��ʱ���˳�
												}
											}
											else 
											{
												//ʧ�ܣ����͵�ƽ��ˣ�Ӧ��
												g_FingerAck 		= ACK_FAILED;
												g_AckType			= FINGER_ACK_TYPE;

												if (SysTask.SendState == SEND_IDLE)
													SysTask.SendState = SEND_TABLET_FINGER;

												SysTask.TouchSub	= TOUCH_SUB_ENTER;
												SysTask.nWaitTime	= 3000; //��ʱһ��ʱ���˳�
											}
										}
										else 
										{
											//ʧ�ܣ����͵�ƽ��ˣ�Ӧ��
											g_FingerAck 		= ACK_FAILED;
											g_AckType			= FINGER_ACK_TYPE;

											if (SysTask.SendState == SEND_IDLE)
												SysTask.SendState = SEND_TABLET_FINGER;

											SysTask.TouchSub	= TOUCH_SUB_ENTER;
											SysTask.nWaitTime	= 3000; //��ʱһ��ʱ���˳�
										}
									}
								}
							}

							break;

						case TOUCH_SUB_WAIT: // *
							if ((SysTask.nWaitTime == 0))
							{
								SysTask.TouchState	= TOUCH_IDLE;
								SysTask.bEnFingerTouch = FALSE; //�ɹ������ָֹ��ģ������
								SysTask.TouchSub	= TOUCH_SUB_INIT;

							}

							break;

						default:
							break;
					}
				}
				break;

			case TOUCH_DEL_USER:
				{
					switch (SysTask.TouchSub)
					{
						case TOUCH_SUB_INIT: // *
							if (SysTask.nWaitTime == 0)
							{
								u8MatchCnt			= 0;
								SysTask.TouchSub	= TOUCH_SUB_ENTER;
							}

							break;

						case TOUCH_SUB_ENTER: //
							if (SysTask.nWaitTime)
							{
								break;
							}

							ensure = PS_DeletChar(SysTask.u16FingerID, 1);

							if (ensure == 0)
							{
								//���͵�ƽ�����
								g_FingerAck 		= ACK_SUCCESS;
								g_AckType			= FINGER_ACK_TYPE;

								if (SysTask.SendState == SEND_IDLE)
									SysTask.SendState = SEND_TABLET_FINGER;

								SysTask.TouchSub	= TOUCH_SUB_WAIT;
								SysTask.nWaitTime	= 100; //��ʱһ��ʱ���˳�
							}
							else //ɾ��ʧ�� ,
							{
								u8MatchCnt++;
								SysTask.nWaitTime	= 500; //��ʱһ��ʱ���ٴ�ɾ��

								if (u8MatchCnt >= 3) //ʧ���˳�
								{
									//���͵�ƽ�����
									g_FingerAck 		= ACK_FAILED;
									g_AckType			= FINGER_ACK_TYPE;

									if (SysTask.SendState == SEND_IDLE)
										SysTask.SendState = SEND_TABLET_FINGER;

									SysTask.TouchSub	= TOUCH_SUB_WAIT;
								}
							}

							break;

						case TOUCH_SUB_WAIT: // *
							if ((SysTask.nWaitTime == 0))
							{
								SysTask.TouchState	= TOUCH_IDLE;
								SysTask.bEnFingerTouch = FALSE; //�ɹ������ָֹ��ģ������
								SysTask.TouchSub	= TOUCH_SUB_INIT;
							}

							break;

						default:
							break;
					}
				}
				break;

			case TOUCH_IDLE:
			default:
				break;
		}
	}
}


/*******************************************************************************
* ����: 
* ����: 
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void LedTask(void)
{
	if (SysTask.LED_Mode != SysTask.LED_ModeSave)
	{
		if (SysTask.LED_Mode == LED_MODE_ON)
			GPIO_ResetBits(LED_GPIO, LED_PIN);
		else 
			GPIO_SetBits(LED_GPIO, LED_PIN);

		SysTask.LED_ModeSave = SysTask.LED_Mode;
	}
}


/*******************************************************************************
* ����: 
* ����: У���
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
u8 CheckSum(u8 * pu8Addr, u8 u8Count)
{
	u8 i				= 0;
	u8 u8Result 		= 0;

	for (i = 0; i < u8Count; i++)
	{
		u8Result			+= pu8Addr[i];
	}

	return ~ u8Result;
}


/*******************************************************************************
* ����: 
* ����: �ӻ�������
*		��ͷ + �ӻ���ַ + 	 �ֱ��ַ + ���� + ���ݰ�[3] + У��� + ��β
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void SlavePackageSend(u8 u8SlaveAddr, u8 u8GlassAddr, u8 u8Cmd, u8 * u8Par)
{
	u8 i				= 0;
	u8 u8SendArr[9] 	=
	{
		0
	};

	u8SendArr[0]		= 0xAA; 					//��ͷ
	u8SendArr[1]		= u8SlaveAddr;				//�ӻ���ַ
	u8SendArr[2]		= u8GlassAddr;				//�ֱ��ַ
	u8SendArr[3]		= u8Cmd;					//����
	u8SendArr[4]		= u8Par[0]; 				//����1
	u8SendArr[5]		= u8Par[1]; 				//����2
	u8SendArr[6]		= u8Par[2]; 				//����3

	u8SendArr[7]		= CheckSum(u8SendArr, 7);	//У���
	u8SendArr[8]		= 0x55; 					//��β


	for (i = 0; i < 8; i++)
	{
		USART_SendData(USART3, u8SendArr[i]);
	}
}


#define RECV_PAKAGE_CHECK		10	 //   10�����ݼ���

/*******************************************************************************
* ����: �����հ���������ƽ�巢�����������������
* ����: �ж��жϽ��յ�������û�и������ݰ� ���߾�����
* �β�:		
* ����: -1, ʧ�� 0:�ɹ�
* ˵��: 
*******************************************************************************/
static int Usart1JudgeStr(void)
{
	//	  ��ͷ + ���� +����[3]		+����[3]	  +  У��� + ��β
	//	  ��ͷ��0XAA
	//	  ��� CMD_UPDATE	 �� CMD_READY 
	//	  ������ ��ַ��LEDģʽ������ɾ��ָ��ID,
	//	  ���ݣ��������� + ���� + ʱ�䣩
	//	  У��ͣ�
	//	  ��β�� 0X55
	//��������ƽ��İ���Ϊ���֡���һ���ǵ�������һ����ȫ��������
	const char * data;

	u8 u8Cmd;
	u8 i;
  int tmp = 0;
	u8 str[8]			=
	{
		0
	};
	u8 u8CheckSum		= 0;
	str[0]				= 0xAA;

	data				= strstr((const char *) USART1_RX_BUF, (const char *) str);

	if (!data)
		return - 1;

	if (* (data + 9) != 0X55) //��β
		return - 1;

	for (i = 0; i < RECV_PAKAGE_CHECK; i++)
		u8CheckSum += * (data + i);

	if (u8CheckSum != 0x55 - 1) //����Ͳ����ڰ�β ����ʧ��
		return - 1;


	u8Cmd				= * (data + 1);

	switch (u8Cmd)
	{
		case CMD_READY:
			SysTask.bTabReady = TRUE;
			SysTask.SendState = SEND_INIT;
			break;

		case CMD_UPDATE:
			if (* (data + 2) == 0xFF) //��ȫ���ֱ�
			{
				for (i = 0; i < GLASS_MAX_CNT; i++)
				{
					//SysTask.WatchState[i].u8LockState  = *(data + 5);  ////��״̬���ڴ˸��£���Ϊ������Ҫ����֮��
					SysTask.WatchState[i].u8Dir = * (data + 6);
					SysTask.WatchState[i].MotoTime = * (data + 7);

					SysTask.Moto_Mode[i] = (MotoFR) * (data + 6);
					SysTask.Moto_Time[i] = (MotoTime_e) * (data + 7);
					SysTask.Moto_RunTime[i] = g_au16MoteTime[SysTask.Moto_Time[i]][1];
					SysTask.Moto_WaitTime[i] = g_au16MoteTime[SysTask.Moto_Time[i]][2];

				}
			}
			else //�����ֱ��״̬����
			{
				//				  SysTask.WatchState[*(data + 2)].u8LockState  = *(data + 5);//��״̬���ڴ˸��£���Ϊ������Ҫ����֮��
				SysTask.WatchState[* (data + 2)].u8Dir = * (data + 6);
				SysTask.WatchState[* (data + 2)].MotoTime = * (data + 7);
				SysTask.Moto_Mode[* (data + 2)] = (MotoFR) * (data + 6);
				SysTask.Moto_Time[* (data + 2)] = (MotoTime_e) * (data + 7);

                tmp = SysTask.Moto_Time[* (data + 2)];
				SysTask.Moto_RunTime[* (data + 2)] = g_au16MoteTime[tmp][1];
				SysTask.Moto_WaitTime[* (data + 2)] = g_au16MoteTime[tmp][2];
			}

			SysTask.u16SaveTick = SysTask.nTick + SAVE_TICK_TIME; //���ݸ��±���
			break;

		case CMD_ADD_USER:
			if (* (data + 2) <= FINGER_MAX_CNT)
			{
				SysTask.bEnFingerTouch = TRUE;
				SysTask.TouchState	= TOUCH_ADD_USER;
				SysTask.TouchSub	= TOUCH_SUB_INIT; //�����ɾ��ָ�Ƶ�����ָ��
				SysTask.u16FingerID = * (data + 2);
			}

			break;

		case CMD_DEL_USER:
			if (* (data + 2) <= FINGER_MAX_CNT)
			{
				SysTask.bEnFingerTouch = TRUE;
				SysTask.TouchState	= TOUCH_DEL_USER;
				SysTask.TouchSub	= TOUCH_SUB_INIT; //�����ɾ��ָ�Ƶ�����ָ��
				SysTask.u16FingerID = * (data + 2);
			}

			break;

		case CMD_LOCK_MODE: //
			if (* (data + 2) == 0xFF) //ȫ����״̬
			{
				for (i = 0; i < GLASS_MAX_CNT; i++)
				{
					SysTask.LockMode[i] = * (data + 5);
				}
			}
			else //������״̬����
			{
				SysTask.LockMode[* (data + 2)] = * (data + 5); //���ת������״̬����OFF?
			}

			break;

		//	 
		//			case CMD_RUN_MOTO:
		//				if (* (data + 2) == 0xFF) //��ȫ���ֱ�
		//				{
		//					for (i = 0; i < GLASS_MAX_CNT; i++)
		//					{
		//						SysTask.LockMode[i] = LOCK_OFF; //���ת������״̬����OFF?
		//						SysTask.Moto_State[i] = MOTO_STATE_INIT;
		//					}
		//				}
		//				else //�����ֱ��״̬����
		//				{
		//					SysTask.LockMode[* (data + 2)] = LOCK_OFF; //���ת������״̬����OFF?
		//					SysTask.Moto_State[* (data + 2)] = MOTO_STATE_INIT;
		//				}
		//	 
		//				break;
		//	 
		case CMD_LED_MODE:
			if ((* (data + 2) == LED_MODE_ON) || (* (data + 2) == LED_MODE_OFF)) //LEDģʽ��Ч
			{
				SysTask.LED_Mode	= * (data + 2);
			}

			SysTask.u16SaveTick = SysTask.nTick + SAVE_TICK_TIME; //���ݸ��±���
			break;

		case CMD_PASSWD_ON: // ����ģʽ������ָ��
			SysTask.bEnFingerTouch = TRUE;
			SysTask.TouchState = TOUCH_MATCH_INIT;
			break;

		case CMD_PASSWD_OFF: // ����ģʽ���ر�ָ��
			SysTask.bEnFingerTouch = FALSE;
			SysTask.TouchState = TOUCH_IDLE;
			break;

		default:
			//���͵�ƽ��ˣ�Ӧ��	 �������
			g_FingerAck = ACK_ERROR;
			g_AckType = ERROR_ACK_TYPE;

			if (SysTask.SendState == SEND_IDLE)
				SysTask.SendState = SEND_TABLET_FINGER;

			break;
	}

	return 0; //		   
}


/*******************************************************************************
* ����: 
* ����: ƽ����������͵����������
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void TabletToHostTask(void)
{
	if (USART1_RX_STA & 0X8000) //���յ�ƽ������
	{
		Usart1JudgeStr();
		USART1_RX_STA		= 0;					//���״̬
	}

	//	  if(USART3_RX_STA & 0X8000)//���յ�ƽ������
	//	  {
	//		  Usart3JudgeStr();
	//		  USART3_RX_STA   =   0; //���״̬
	//	  }
}


/*******************************************************************************
* ����: 
* ����: ��������ȫ��״̬��ƽ��  ���
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void SendToTablet_Zero(u8 u8GlassAddr)
{
	//	  ��ͷ + ��ַ + ���� + ���ݰ�[48] + У��� + ��β
	//	  ��ͷ��0XAA
	//	  �ֱ��ַ��0X8~�����λΪ1��ʾȫ��						~:��ʾ�м����ֱ�
	//				0x01����һ���ֱ�
	//	  ���
	//	  ���ݣ��������� + ���� + ʱ�䣩* 16 = 48
	//	  У��ͣ�
	//	  ��β�� 0X55
	u8 i;
	u8 u8aSendArr[53]	=
	{
		0
	};
	SendArrayUnion_u SendArrayUnion;

	u8aSendArr[1]		= MCU_ACK_ADDR; 			//Ӧ���ַ

	u8aSendArr[2]		= ZERO_ACK_TYPE;			//Ӧ���������
	u8aSendArr[3]		= u8GlassAddr;				//�ֱ��ַ

	u8aSendArr[0]		= 0xAA;

	for (i = 0; i < 47; i++)
	{
		u8aSendArr[4 + i]	= SendArrayUnion.u8SendArray[i];
	}

	u8aSendArr[51]		= CheckSum(u8aSendArr, 51); //У���
	u8aSendArr[52]		= 0x55; 					//��βs 

	for (i = 0; i < 53; i++)
	{
		Usart1_SendByte(u8aSendArr[i]);
	}
}


/*******************************************************************************
* ����: 
* ����: ��������ȫ��״̬��ƽ��
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void SendToTablet(u8 u8GlassAddr)
{
	//	  ��ͷ + ��ַ + ���� + ���ݰ�[48] + У��� + ��β
	//	  ��ͷ��0XAA
	//	  �ֱ��ַ��0X8~�����λΪ1��ʾȫ��						~:��ʾ�м����ֱ�
	//				0x01����һ���ֱ�
	//	  ���
	//	  ���ݣ��������� + ���� + ʱ�䣩* 16 = 48
	//	  У��ͣ�
	//	  ��β�� 0X55
	u8 i;
	u8 u8aSendArr[53]	=
	{
		0
	};
	SendArrayUnion_u SendArrayUnion;

	if (u8GlassAddr == MCU_ACK_ADDR) //Ӧ���ַ
	{
		u8aSendArr[2]		= g_AckType;			//Ӧ������
		u8aSendArr[3]		= g_FingerAck;			//��������	Ӧ���������
	}
	else 
	{
		memcpy(SendArrayUnion.WatchState, SysTask.WatchState, sizeof(SysTask.WatchState));

		u8aSendArr[2]		= 0x11; 				//�������ݱ����� 

		for (i = 0; i < 48; i++)
		{
			u8aSendArr[3 + i]	= SendArrayUnion.u8SendArray[i];
		}
	}

	u8aSendArr[0]		= 0xAA;
	u8aSendArr[1]		= u8GlassAddr;
	u8aSendArr[51]		= CheckSum(u8aSendArr, 51); //У���
	u8aSendArr[52]		= 0x55; 					//��βs 

	for (i = 0; i < 53; i++)
	{
		Usart1_SendByte(u8aSendArr[i]);
	}
}


/*******************************************************************************
* ����: 
* ����: �������ʹ�����״̬��ƽ��
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void SendToTabletDetect(u8 u8GlassAddr)
{
	//	  ��ͷ + ��ַ + ���� + ���ݰ�[48] + У��� + ��β
	//	  ��ͷ��0XAA
	//	  �ֱ��ַ��0X8~�����λΪ1��ʾȫ��						~:��ʾ�м����ֱ�
	//				0x01����һ���ֱ�
	//	  ���
	//	  ���ݣ��������� + ���� + ʱ�䣩* 16 = 48
	//	  У��ͣ�
	//	  ��β�� 0X55
	u8 i;
	u8 u8aSendArr[53]	=
	{
		0
	};

	u8aSendArr[0]		= 0xAA;
    u8aSendArr[2]       = g_AckType;            //Ӧ������
    u8aSendArr[3]       = g_FingerAck;          //��������  Ӧ���������
	u8aSendArr[1]		= u8GlassAddr;
	u8aSendArr[51]		= CheckSum(u8aSendArr, 51); //У���
	u8aSendArr[52]		= 0x55; 					//��βs 

	for (i = 0; i < 53; i++)
	{
		Usart1_SendByte(u8aSendArr[i]);
	}
}

/*******************************************************************************
* ����: 
* ����: ���͸�ƽ�������
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void SendTabletTask(void)
{
	switch (SysTask.SendState)
	{
		case SEND_INIT:
			if (SysTask.bTabReady == TRUE)
			{
				//SLAVE_SEND_ALL; 
				//				  g_GlassSendAddr = GLASS_SEND_ALL | GLASS_MAX_CNT;
				SendToTablet(GLASS_SEND_ALL | GLASS_MAX_CNT); //�����ֱ����
				SysTask.SendState	= SEND_IDLE;
			}

			break;

		case SEND_TABLET_FINGER: //����ģ��Ӧ���ƽ��
			SendToTablet(MCU_ACK_ADDR); //
			SysTask.SendState = SEND_IDLE;
			break;
            
        case SEND_TABLET_DETECT: //���ʹ�����״̬��ƽ��
            SendToTablet(MCU_ACK_ADDR); //
            SysTask.SendState = SEND_IDLE;
            break;

		case SEND_TABLET_ZERO: //����ģ������ƽ��,����˾Ͳ����ˣ�ƽ����г�ʱ����
			SendToTablet_Zero(g_GlassSendAddr); //	�����Ļ�ֻ�ܷ������һ�����ˣ�����������߳�ͻ�������˵��������ٴε��������200ms���ϣ����Ե�������˵��ͻ�Ļ��ʻ��Ǻ�С��
			SysTask.SendState = SEND_IDLE;
			break;

		/* ���ڽ׶�û��ʹ���ⲿ�ֵ����ݣ���Ϊ����Ӧ����飬���������ⲿ����ʱ�ò�?
			?
		case SEND_TABLET_SINGLE://���͵����ӻ�״̬��ƽ��
			SendToTablet(g_SlaveSendAddr, g_GlassSendAddr);
			break;
		*/
		case SEND_IDLE:
		default:
			SysTask.SendState = SEND_IDLE;
			break;
	}
}


/*******************************************************************************
* ����: 
* ����: �����ӷ��͸��ӻ����͵����������
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void HostToTabletTask(void)
{
	//	  static u8 i = 0;
	//	  static u8 j = 0;
	//	  static u8 state = 0;
	//SysTask.WatchStateSave, SysTask.SlaveState
	//	  switch(state)
	//		  {
	//		  case HtoS_CHECK:
	//			  if(	  (SysTask.WatchStateSave.MotoTime != SysTask.WatchState.MotoTime)
	//				 ||   (SysTask.WatchStateSave.MotoTime != SysTask.WatchState.MotoTime)
	//				 ||   (SysTask.WatchStateSave.MotoTime != SysTask.WatchState.MotoTime))
	//			  {
	//				  state = HtoS_SEND;
	//			  }
	//			  else
	//			  {
	//				  if(i == 7){
	//					  j++;
	//					  if(j == 4) j = 0;
	//					  i = 0;
	//				   }
	//				  else
	//					  i++;
	//			  }
	//			  break;
	//		  case HtoS_SEND:
	//			  SysTask.WatchState.MotoTime = SysTask.WatchStateSave.MotoTime;
	//			  SysTask.WatchState.MotoTime = SysTask.WatchStateSave.MotoTime;
	//			  SysTask.WatchState.MotoTime = SysTask.WatchStateSave.MotoTime;
	//			  state = HtoS_WAIT;
	//			  break;
	//		  case HtoS_WAIT:
	//			  if(SysTask.u16HtoSWaitTime == 0)
	//				  state = HtoS_CHECK;
	//			  break;
	//		  default:
	//			  break;
	//	  }
}

/*******************************************************************************
* ����: 
* ����: SOFT PWM ���Ƶ��ת��
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void PWM_01_Task(void)
{
	//A���
	switch (SysTask.PWM_State[GLASS_01])
	{
		case PWM_STATE_INIT:
            break;
		case PWM_STATE_FEW_P:
            if(SysTask.PWM_RunTime[GLASS_01] != 0)
                break;
            if(SysTask.Lock_State[GLASS_01] == LOCK_STATE_DEBOUNSE) //׼�������������κδ���
                break;
            GPIO_ResetBits(MOTO_P_GPIO, MOTO_01_PIN_P); //����ͣ���ת��
            GPIO_ResetBits(MOTO_N_GPIO, MOTO_01_PIN_N); //����ͣ���ת��
            SysTask.PWM_State[GLASS_01]     = PWM_STATE_FEW_N;
            SysTask.PWM_RunTime[GLASS_01]   = PWM_PERIOD - PWM_POSITIVE;
            break;
            
		case PWM_STATE_FEW_N:
//            if(SysTask.Lock_State[GLASS_01] == LOCK_STATE_DEBOUNSE) //׼����������ת��ȥ��ԭ�� ����. �ⲿ��Ҫ��moto����ģ���Ϊ��ת�������뷴ת�����޿�����ͬһ�������ʱ�� ��Ҫ
                                                                      //ʹ����ȷ������ת��ת
//                GPIO_ResetBits(MOTO_P_GPIO, MOTO_01_PIN_P); //�ȷ�ת
//        		GPIO_SetBits(MOTO_N_GPIO, MOTO_01_PIN_N); //
//                SysTask.PWM_State[GLASS_01]   = PWM_STATE_REV_P;
//                SysTask.PWM_RunTime[GLASS_01] = 0;
//                break;
//            }
            if(SysTask.Lock_State[GLASS_01] == LOCK_STATE_DEBOUNSE) //׼����������ת��ȥ��ԭ�� ����. �ⲿ��Ҫ��moto����ģ���Ϊ��ת�������뷴ת�����޿�����ͬһ�������ʱ�� ��Ҫ
                                                                      //ʹ����ȷ������ת��ת
            {
                break;
            }

            if(SysTask.PWM_RunTime[GLASS_01] != 0)
                break;
            GPIO_SetBits(MOTO_P_GPIO, MOTO_01_PIN_P); //����ת
    		GPIO_ResetBits(MOTO_N_GPIO, MOTO_01_PIN_N); //
            SysTask.PWM_State[GLASS_01]     = PWM_STATE_FEW_P;
            SysTask.PWM_RunTime[GLASS_01]   = PWM_POSITIVE;
            break;
		case PWM_STATE_REV_P:
            if(SysTask.PWM_RunTime[GLASS_01] != 0)
                break;
            if(SysTask.Lock_State[GLASS_01] == LOCK_STATE_DEBOUNSE) //׼�������������κδ���
                break;
            GPIO_ResetBits(MOTO_P_GPIO, MOTO_01_PIN_P); //����ͣ���ת��
            GPIO_ResetBits(MOTO_N_GPIO, MOTO_01_PIN_N); //����ͣ���ת��
            SysTask.PWM_State[GLASS_01]     = PWM_STATE_REV_N;
            SysTask.PWM_RunTime[GLASS_01]   = PWM_PERIOD - PWM_POSITIVE;
            break;
            
		case PWM_STATE_REV_N:
//            if(SysTask.Lock_State[GLASS_01] == LOCK_STATE_DEBOUNSE) //׼����������ת��ȥ��ԭ�� ����. �ⲿ��Ҫ��moto����ģ���Ϊ��ת�������뷴ת�����޿�����ͬһ�������ʱ�� ��Ҫ
                                                                      //ʹ����ȷ������ת��ת
//            {
//                GPIO_SetBits(MOTO_P_GPIO, MOTO_01_PIN_P); //����ת
//        		GPIO_ResetBits(MOTO_N_GPIO, MOTO_01_PIN_N); //
//                SysTask.PWM_State[GLASS_01]   = PWM_STATE_FEW_P;
//                SysTask.PWM_RunTime[GLASS_01] = 0;
//                break;
//            }
            if(SysTask.Lock_State[GLASS_01] == LOCK_STATE_DEBOUNSE) //׼����������ת��ȥ��ԭ�� ����. �ⲿ��Ҫ��moto����ģ���Ϊ��ת�������뷴ת�����޿�����ͬһ�������ʱ�� ��Ҫ
                                                                      //ʹ����ȷ������ת��ת
            {
                break;
            }

            if(SysTask.PWM_RunTime[GLASS_01] != 0)
                break;
            GPIO_ResetBits(MOTO_P_GPIO, MOTO_01_PIN_P); //�ȷ�ת
    		GPIO_SetBits(MOTO_N_GPIO, MOTO_01_PIN_N); //
            SysTask.PWM_State[GLASS_01]     = PWM_STATE_REV_P;
            SysTask.PWM_RunTime[GLASS_01]   = PWM_POSITIVE;
            break;
            
        case PWM_STATE_STOP:
            GPIO_ResetBits(MOTO_P_GPIO, MOTO_01_PIN_P); //����ͣ���ת��
            GPIO_ResetBits(MOTO_N_GPIO, MOTO_01_PIN_N); //����ͣ���ת��
        default:
            break;
    }
}

/*******************************************************************************
* ����: 
* ����: ֻ������ת�ģ�ת��Ȧ ��Ϊ����Լ��������ź��߲��ܸ���
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void Moto_01_Task(void)
{
	//A���
	switch (SysTask.Moto_State[GLASS_01])
	{
		case MOTO_STATE_INIT:
            if(SysTask.Moto_Mode[GLASS_01] == MOTO_FR_FWD_REV){
			    SysTask.Moto_Time[GLASS_01] = MOTO_TIME_ANGLE; //תһ���Ƕ�
				SysTask.Moto_RunTime[GLASS_01] = SysTask.nTick + g_au16MoteTime[SysTask.Moto_Time[GLASS_01]][1] + MOTO_DEVIATION_TIME; //��ϵͳƫ��
				SysTask.Moto_State[GLASS_01] = MOTO_STATE_FIND_ZERO;
                SysTask.Moto_ModeSave[GLASS_01] = SysTask.Moto_Mode[GLASS_01];
                
#if DEBUG 
                u3_printf("MOTO_STATE_INIT");
                u3_printf("��������ת ȥ��ԭ�㡣��\n");
#endif
                SysTask.Moto_SubState[GLASS_01] = MOTO_SUB_STATE_UP; 
    			//GPIO_SetBits(MOTO_P_GPIO, MOTO_01_PIN_P); //����ת
    			//GPIO_ResetBits(MOTO_N_GPIO, MOTO_01_PIN_N); //
                SysTask.PWM_State[GLASS_01]     = PWM_STATE_FEW_N;//����ת
                SysTask.PWM_RunTime[GLASS_01]   = 0;
            }else{
                //�����ʱ����û�лص�ԭ�㣿ֻ���ڿ�ʼ��û�н���ת��״̬ʱ����ƽ���stop���Ż���룬��������������ǳ��ټ�������û��
                //���flash�����������״̬����Ҳ�����ֿ��� 
    			SysTask.Moto_State[GLASS_01] = MOTO_STATE_STOP;
                SysTask.Moto_SubState[GLASS_01] = MOTO_SUB_STATE_DOWN;
                SysTask.Moto_RunTime[GLASS_01] = 0;
                SysTask.Moto_ModeSave[GLASS_01] = SysTask.Moto_Mode[GLASS_01];
            }
			break;
            
		case MOTO_STATE_FIND_ZERO:
            switch (SysTask.Moto_SubState[GLASS_01])
			{
				case MOTO_SUB_STATE_UP: //��������ת
                    if (GPIO_ReadInputDataBit(LOCK_ZERO_GPIO, LOCK_ZERO_01_PIN) == 0)  // �ҵ�ԭ�㣬
                    {                
#if DEBUG 
                        u3_printf("MOTO_STATE_FIND_ZERO ");
                        u3_printf("��ת �ҵ�ԭ�㣬");
                        u3_printf("\n");
#endif
    			        SysTask.Moto_State[GLASS_01] = MOTO_STATE_RUN_CHA;
        				SysTask.Moto_SubState[GLASS_01] = MOTO_SUB_STATE_UP_F; //������ת forward
				        SysTask.Moto_RunTime[GLASS_01] = SysTask.nTick + g_au16MoteTime[SysTask.Moto_Time[GLASS_01]][1];
                    }
					else if (SysTask.Moto_RunTime[GLASS_01] == 0)   //ת��һ���ĽǶ�û���ҵ�ԭ�㣬����ȥ��ԭ��
					{           
#if DEBUG 
                        u3_printf("MOTO_STATE_FIND_ZER");
                        u3_printf("��ת û�е�ԭ��");
                        u3_printf("\n");
#endif
			            SysTask.Moto_SubState[GLASS_01] = MOTO_SUB_STATE_DOWN;
                        //���ٽ���ת�յ�Ϊ��ʼ������ǰ���Ѿ���תһ���Ƕȣ�����Ҫ�Ⱦ����ص�֮ǰλ���ٷ�ת������ʱ��Ҫ���� 2
                        //Ĭ����һ�ιػ���û�м�⵽���ʱ�����MOTO_DEVIATION_TIME
			            SysTask.Moto_RunTime[GLASS_01]  = SysTask.nTick + (g_au16MoteTime[SysTask.Moto_Time[GLASS_01]][1] + MOTO_DEVIATION_TIME + MOTO_DEVIATION_TIME) * 2 + MOTO_DEVIATION_TIME; 
						//GPIO_SetBits(MOTO_N_GPIO, MOTO_01_PIN_N); //��ת
						//GPIO_ResetBits(MOTO_P_GPIO, MOTO_01_PIN_P); //
                        SysTask.PWM_State[GLASS_01]     = PWM_STATE_REV_N;
                        SysTask.PWM_RunTime[GLASS_01]   = 0;
					}

					break;
                    
                case MOTO_SUB_STATE_DOWN:  //�½��� ��ת
                    if (GPIO_ReadInputDataBit(LOCK_ZERO_GPIO, LOCK_ZERO_01_PIN) == 0)  // �ҵ�ԭ�㣬
                    {        
#if DEBUG 
                        u3_printf("MOTO_STATE_FIND_ZERO");
                        u3_printf("��ת �ҵ�ԭ�㣬\n");
#endif
    			        SysTask.Moto_State[GLASS_01] = MOTO_STATE_RUN_CHA;
        				SysTask.Moto_SubState[GLASS_01] = MOTO_SUB_STATE_DOWN_F;  //������ת forward
				        SysTask.Moto_RunTime[GLASS_01] = SysTask.nTick + g_au16MoteTime[SysTask.Moto_Time[GLASS_01]][1];
                    }
					else if (SysTask.Moto_RunTime[GLASS_01] == 0)   //ת��һ���ĽǶ�û���ҵ�ԭ�㣬���ʧЧ���Ե�ǰ�Ƕ�Ϊ��ת�յ�
					{     
#if DEBUG 
                        u3_printf("MOTO_STATE_FIND_ZERO");
                        u3_printf("��תû�е�ԭ��\n");
#endif			
        				SysTask.Moto_State[GLASS_01] = MOTO_STATE_CHANGE_DIR; //��ͷ��ʼ��ת

                        g_bSensorState      = FALSE;
                        //����״̬��ƽ���
    					g_FingerAck 		= ACK_FAILED;
    					g_AckType			= DETECT_ACK_TYPE;
			            g_GlassSendAddr 	= GLASS_01;

    					if (SysTask.SendState == SEND_IDLE)
    						SysTask.SendState = SEND_TABLET_DETECT;
					}
                    break;
                   
				default:
					break;
                }
            break;

		case MOTO_STATE_CHANGE_DIR:
			SysTask.Moto_State[GLASS_01] = MOTO_STATE_RUN_CHA;
            SysTask.Moto_SubState[GLASS_01] = MOTO_SUB_STATE_UP_F; 
		    SysTask.Moto_ModeSave[GLASS_01] = SysTask.Moto_Mode[GLASS_01];
#if DEBUG 
            u3_printf("\n---------------��������״̬-----------\n");
#endif
			break;

		case MOTO_STATE_RUN_CHA: //����ת����״̬�� ֻ������״̬ �������ģʽ����
			switch (SysTask.Moto_SubState[GLASS_01])
			{
				case MOTO_SUB_STATE_UP: //��������ת
                    if(SysTask.Lock_State[GLASS_01] == LOCK_STATE_DEBOUNSE) //׼����������ת��ȥ��ԭ�� ����. �ⲿ��Ҫ��moto����ģ���������PWM�����
                    {
                        GPIO_SetBits(MOTO_P_GPIO, MOTO_01_PIN_P); //����ת
                		GPIO_ResetBits(MOTO_N_GPIO, MOTO_01_PIN_N); //
                        SysTask.PWM_State[GLASS_01]   = PWM_STATE_FEW_P;
                        SysTask.PWM_RunTime[GLASS_01] = 0;
                        break;
                    }
                    
					if (SysTask.Moto_RunTime[GLASS_01] == 0)   // ��ʱ����� ��˵���Ѿ�ת�˹̶��Ƕ�
					{
#if DEBUG 
                        u3_printf("MOTO_SUB_STATE_UP");
                        u3_printf("��ת\n");
#endif
			            SysTask.Moto_SubState[GLASS_01] = MOTO_SUB_STATE_ZERO_R;
                        if(g_bSensorState == TRUE)
			                SysTask.Moto_RunTime[GLASS_01]  = SysTask.nTick + g_au16MoteTime[SysTask.Moto_Time[GLASS_01]][1]  + MOTO_DEVIATION_TIME;   //��һ����ȥ����㣬����Ҫ����һ��ϵͳƫ���ʱ��
                        else
                            SysTask.Moto_RunTime[GLASS_01]  = SysTask.nTick + g_au16MoteTime[SysTask.Moto_Time[GLASS_01]][1]; //û�и�Ӧ�������������ʱ��Ļ�׼�����ü���ƫ��
            			//GPIO_SetBits(MOTO_P_GPIO, MOTO_01_PIN_P); //����ת
            			//GPIO_ResetBits(MOTO_N_GPIO, MOTO_01_PIN_N); //
                        SysTask.PWM_State[GLASS_01]     = PWM_STATE_FEW_N;//����ת
                        SysTask.PWM_RunTime[GLASS_01]   = 0;
					}
					break;
                    
                case MOTO_SUB_STATE_ZERO_R:  //�����
                    if(SysTask.Lock_State[GLASS_01] == LOCK_STATE_DEBOUNSE) //׼����������ת��ȥ��ԭ�� ����. �ⲿ��Ҫ��moto����ģ���������PWM�����
                    {
                        GPIO_SetBits(MOTO_P_GPIO, MOTO_01_PIN_P); //����ת
                		GPIO_ResetBits(MOTO_N_GPIO, MOTO_01_PIN_N); //
                        SysTask.PWM_State[GLASS_01]   = PWM_STATE_FEW_P;
                        SysTask.PWM_RunTime[GLASS_01] = 0;
                    }
                    
                    if (GPIO_ReadInputDataBit(LOCK_ZERO_GPIO, LOCK_ZERO_01_PIN) == 0)  // �ҵ�ԭ�㣬
                    {
#if DEBUG 
                        u3_printf(" MOTO_SUB_STATE_ZERO_R");
                        u3_printf("��ת �ҵ�ԭ�㣬\n");
#endif
                        if(SysTask.Lock_State[GLASS_01] == LOCK_STATE_DEBOUNSE) //׼�������������κδ���
                            break;
                        
                        if (SysTask.Moto_Mode[GLASS_01] != SysTask.Moto_ModeSave[GLASS_01])  //״̬�ı䣬��ת������ֹͣ״̬ʱ�����������֧ 
                        {
#if DEBUG 
                            u3_printf(" MOTO_SUB_STATE_ZERO_R");
                            u3_printf("״̬�ı�\n");
#endif
                            //GPIO_ResetBits(MOTO_P_GPIO, MOTO_01_PIN_P); //����ͣ���ת��
                            //GPIO_ResetBits(MOTO_N_GPIO, MOTO_01_PIN_N); //����ͣ���ת��
                            SysTask.PWM_State[GLASS_01]     = PWM_STATE_STOP;
                            SysTask.PWM_RunTime[GLASS_01]   = 0;

                            SysTask.Moto_State[GLASS_01] = MOTO_STATE_INIT; //����ʱһ��ʱ���ٷ�ת
                            break;
                        }
                        
                        SysTask.Moto_SubState[GLASS_01] = MOTO_SUB_STATE_UP_F;
			            SysTask.Moto_RunTime[GLASS_01]  = SysTask.nTick + g_au16MoteTime[SysTask.Moto_Time[GLASS_01]][1];
                    }
                    else if (SysTask.Moto_RunTime[GLASS_01] == 0)   //ת��һ���ĽǶ�û���ҵ�ԭ��
                    {
#if DEBUG 
                        u3_printf(" MOTO_SUB_STATE_ZERO_R");
                        u3_printf("��ת   û���ҵ�ԭ�㣬\n");
#endif
                        SysTask.Moto_SubState[GLASS_01] = MOTO_SUB_STATE_UP_F;
                        SysTask.Moto_RunTime[GLASS_01]  = SysTask.nTick + g_au16MoteTime[SysTask.Moto_Time[GLASS_01]][1];
                    }
                    break;
                    
				case MOTO_SUB_STATE_UP_F: //������ת forward
                    if(SysTask.Lock_State[GLASS_01] == LOCK_STATE_DEBOUNSE) //׼����������ת��ȥ��ԭ�� ����. �ⲿ��Ҫ��moto����ģ���������PWM�����
                    {
                        GPIO_ResetBits(MOTO_P_GPIO, MOTO_01_PIN_P); //�ȷ�ת
                		GPIO_SetBits(MOTO_N_GPIO, MOTO_01_PIN_N); //
                        SysTask.PWM_State[GLASS_01]     = PWM_STATE_REV_P;
                        SysTask.PWM_RunTime[GLASS_01] = 0;
                        break;
                    }
                    
					if (SysTask.Moto_RunTime[GLASS_01] == 0)   // ��ʱ����� ��˵���Ѿ�ת�˹̶��Ƕ�
					{
#if DEBUG 
                        u3_printf("MOTO_SUB_STATE_UP_F");
                        u3_printf("������ת��\n");
#endif
			            SysTask.Moto_SubState[GLASS_01] = MOTO_SUB_STATE_DOWN;
					}

					break;
                    
                case MOTO_SUB_STATE_DOWN:  //�½��� ��ת
#if DEBUG 
                    u3_printf("MOTO_SUB_STATE_DOWN");
                    u3_printf("��ת��\n");
#endif
                    if(SysTask.Lock_State[GLASS_01] == LOCK_STATE_DEBOUNSE) //׼����������ת��ȥ��ԭ�� ����. �ⲿ��Ҫ��moto����ģ���������PWM�����
                    {
                        GPIO_ResetBits(MOTO_P_GPIO, MOTO_01_PIN_P); //�ȷ�ת
                		GPIO_SetBits(MOTO_N_GPIO, MOTO_01_PIN_N); //
                        SysTask.PWM_State[GLASS_01]     = PWM_STATE_REV_P;
                        SysTask.PWM_RunTime[GLASS_01] = 0;
                        break;
                    }
                    
                    if(g_bSensorState == TRUE)   //
		                SysTask.Moto_RunTime[GLASS_01]  = SysTask.nTick + g_au16MoteTime[SysTask.Moto_Time[GLASS_01]][1]  + MOTO_DEVIATION_TIME * 3;
                    else
                        SysTask.Moto_RunTime[GLASS_01]  = SysTask.nTick + g_au16MoteTime[SysTask.Moto_Time[GLASS_01]][1];
                    //GPIO_SetBits(MOTO_N_GPIO, MOTO_01_PIN_N); //��ת
                    //GPIO_ResetBits(MOTO_P_GPIO, MOTO_01_PIN_P); //
                    
#if DEBUG
                    u3_printf("�½��� ��ת");
                    snprintf(priBuf, 50, "SysTask.Moto_RunTime[GLASS_01] %d\n", SysTask.Moto_RunTime[GLASS_01]);
                    u3_printf(priBuf);
                    u3_printf("\n");
#endif
                    SysTask.PWM_State[GLASS_01]     = PWM_STATE_REV_N;
                    SysTask.PWM_RunTime[GLASS_01]   = 0;

			        SysTask.Moto_SubState[GLASS_01] = MOTO_SUB_STATE_ZERO_L;
					break;
                    
                case MOTO_SUB_STATE_ZERO_L:  //�����

#if DEBUG
                    snprintf(priBuf, 50, "%d\n", SysTask.Moto_RunTime[GLASS_01]);
                    u3_printf(priBuf);
#endif
                    if(SysTask.Lock_State[GLASS_01] == LOCK_STATE_DEBOUNSE) //׼����������ת��ȥ��ԭ�� ����. �ⲿ��Ҫ��moto����ģ���������PWM�����
                    {
                        GPIO_ResetBits(MOTO_P_GPIO, MOTO_01_PIN_P); //�ȷ�ת
                		GPIO_SetBits(MOTO_N_GPIO, MOTO_01_PIN_N); //
                        SysTask.PWM_State[GLASS_01]     = PWM_STATE_REV_P;
                        SysTask.PWM_RunTime[GLASS_01] = 0;
                    }
                    
                    if (GPIO_ReadInputDataBit(LOCK_ZERO_GPIO, LOCK_ZERO_01_PIN) == 0)  // �ҵ�ԭ�㣬
                    {
#if DEBUG 
                        u3_printf(" MOTO_SUB_STATE_ZERO_L");
                        u3_printf("�ҵ�ԭ�㣬\n");
#endif
                        if(SysTask.Lock_State[GLASS_01] == LOCK_STATE_DEBOUNSE) //׼�������������κδ���
                            break;
                        
                        if (SysTask.Moto_Mode[GLASS_01] != SysTask.Moto_ModeSave[GLASS_01])  //״̬�ı䣬��ת������ֹͣ״̬ʱ�����������֧ 
                        {
#if DEBUG 
                            u3_printf(" MOTO_SUB_STATE_ZERO_L");
                            u3_printf("״̬�ı�\n");
#endif
                            //GPIO_ResetBits(MOTO_P_GPIO, MOTO_01_PIN_P); //����ͣ���ת��
                            //GPIO_ResetBits(MOTO_N_GPIO, MOTO_01_PIN_N); //����ͣ���ת��
                            SysTask.PWM_State[GLASS_01]     = PWM_STATE_STOP;
                            SysTask.PWM_RunTime[GLASS_01]   = 0;

                            SysTask.Moto_State[GLASS_01] = MOTO_STATE_INIT; //����ʱһ��ʱ���ٷ�ת
                            break;
                        }
                        
                        SysTask.Moto_SubState[GLASS_01] = MOTO_SUB_STATE_DOWN_F;
    		            SysTask.Moto_RunTime[GLASS_01]  = SysTask.nTick + g_au16MoteTime[SysTask.Moto_Time[GLASS_01]][1]  + MOTO_DEVIATION_TIME;
                    }
                    else if (SysTask.Moto_RunTime[GLASS_01] == 0)   //ת��һ���ĽǶ�û���ҵ�ԭ�㣬
                    {
#if DEBUG 
                        u3_printf("MOTO_SUB_STATE_ZERO_L");
                        u3_printf("û���ҵ� ԭ�㣬\n");
#endif
                        if(SysTask.Lock_State[GLASS_01] == LOCK_STATE_DEBOUNSE) //׼�������������κδ���
                            break;
                        
                        if (SysTask.Moto_Mode[GLASS_01] != SysTask.Moto_ModeSave[GLASS_01])  //״̬�ı䣬��ת������ֹͣ״̬ʱ�����������֧ 
                        {
                            //GPIO_ResetBits(MOTO_P_GPIO, MOTO_01_PIN_P); //����ͣ���ת��
                            //GPIO_ResetBits(MOTO_N_GPIO, MOTO_01_PIN_N); //����ͣ���ת��
                            SysTask.PWM_State[GLASS_01]     = PWM_STATE_STOP;
                            SysTask.PWM_RunTime[GLASS_01]   = 0;
                        
                            SysTask.Moto_State[GLASS_01] = MOTO_STATE_INIT; //����ʱһ��ʱ���ٷ�ת
                            break;
                        }
                        
                        SysTask.Moto_SubState[GLASS_01] = MOTO_SUB_STATE_DOWN_F;
                        SysTask.Moto_RunTime[GLASS_01]  = SysTask.nTick + g_au16MoteTime[SysTask.Moto_Time[GLASS_01]][1];
                    }
                    break;
                    
                case MOTO_SUB_STATE_DOWN_F: //������ת forward
                    if(SysTask.Lock_State[GLASS_01] == LOCK_STATE_DEBOUNSE) //׼����������ת��ȥ��ԭ�� ����. �ⲿ��Ҫ��moto����ģ���������PWM�����
                    {
                        GPIO_SetBits(MOTO_P_GPIO, MOTO_01_PIN_P); //����ת
                		GPIO_ResetBits(MOTO_N_GPIO, MOTO_01_PIN_N); //
                        SysTask.PWM_State[GLASS_01]   = PWM_STATE_FEW_P;
                        SysTask.PWM_RunTime[GLASS_01] = 0;
                        break;
                    }
                    
                    if (SysTask.Moto_RunTime[GLASS_01] == 0)   // ��ʱ����� ��˵�� �Ѿ�ת�˹̶��Ƕ�
                    {
#if DEBUG 
                        u3_printf("MOTO_SUB_STATE_DOWN_F");
                        u3_printf("������ת��\n");
                        u3_printf("\n------------- �ظ�-------------\n");
#endif
                        SysTask.Moto_SubState[GLASS_01] = MOTO_SUB_STATE_UP;
                    }
                    break;

				default:
					break;
			}

			break;

		case MOTO_STATE_STOP:
			if ((GPIO_ReadInputDataBit(LOCK_ZERO_GPIO, LOCK_ZERO_01_PIN) == 0) || (SysTask.PWM_State[GLASS_01] == PWM_STATE_STOP )) //Ҫ��ص�ԭ��ſ���
			{
#if DEBUG 
                        u3_printf(" MOTO_STATE_STOP");
                        u3_printf("ֹͣ\n");
#endif
                //GPIO_ResetBits(MOTO_P_GPIO, MOTO_01_PIN_P); //����ͣ���ת��
                //GPIO_ResetBits(MOTO_N_GPIO, MOTO_01_PIN_N); //����ͣ���ת��
                SysTask.PWM_State[GLASS_01]     = PWM_STATE_STOP;
                SysTask.PWM_RunTime[GLASS_01]   = 0;
                
				SysTask.Moto_State[GLASS_01] = MOTO_STATE_IDLE;
			}

			break;

		case MOTO_STATE_WAIT:
			break;

		case MOTO_STATE_IDLE:
            if (SysTask.Moto_Mode[GLASS_01] != SysTask.Moto_ModeSave[GLASS_01])
            {
                SysTask.Moto_State[GLASS_01] = MOTO_STATE_INIT; //����ʱһ��ʱ���ٷ�ת
                break;
            }
			break;

		default:
			break;
	}
}


/*******************************************************************************
* ����: 
* ����: �м�� ������ �½�ʱ�и������ ������ʹ��ʱ�����жϡ���ת������ת��
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void Moto_02_Task(void)
{
	//_02���
	switch (SysTask.Moto_State[GLASS_02])
	{
		case MOTO_STATE_INIT:
			if (SysTask.Moto_StateTime[GLASS_02] == 0)
			{
				SysTask.Moto_State[GLASS_02] = MOTO_STATE_CHANGE_DIR;
			}

			break;

		case MOTO_STATE_CHANGE_TIME:
			break;

		case MOTO_STATE_CHANGE_DIR:
			if (SysTask.Moto_Mode[GLASS_02] == MOTO_FR_FWD)
			{
				GPIO_SetBits(MOTO_P_GPIO, MOTO_02_PIN_P); //��ת
				GPIO_ResetBits(MOTO_N_GPIO, MOTO_02_PIN_N); //
				SysTask.Moto_SubState[GLASS_02] = MOTO_SUB_STATE_UP;
                
			    //TODO
				SysTask.Moto_RunTime[GLASS_02] = MOTO_UP_TIME;  //������������õ�ʱ��
				SysTask.Moto_State[GLASS_02] = MOTO_STATE_RUN_NOR;
			}
			else if (SysTask.Moto_Mode[GLASS_02] == MOTO_FR_REV)
			{
				GPIO_SetBits(MOTO_N_GPIO, MOTO_02_PIN_N); //��ת
				GPIO_ResetBits(MOTO_P_GPIO, MOTO_02_PIN_P); //
				SysTask.Moto_State[GLASS_02] = MOTO_STATE_RUN_NOR;
				SysTask.Moto_RunTime[GLASS_02] = MOTO_DOWN_TIME;  //��������½��õ�ʱ��  ������Զ�����
				SysTask.Moto_SubState[GLASS_02] = MOTO_SUB_STATE_DOWN;
			}

			SysTask.Moto_ModeSave[GLASS_02] = SysTask.Moto_Mode[GLASS_02];
			break;

		case MOTO_STATE_RUN_NOR: //��ͨ����״̬
			if ((SysTask.Moto_Mode[GLASS_02] != SysTask.Moto_ModeSave[GLASS_02])
                && (SysTask.Moto_Mode[GLASS_02] == MOTO_FR_REV))//��;ģʽ�ı䣬����������Ϊ�½������������½���Ϊ��������Ϊ��������ʱ����Ƶ� ����֪����ǰ��λ�ò�֪��Ҫ��������ʱ��
			{
				GPIO_ResetBits(MOTO_P_GPIO, MOTO_02_PIN_P); //����ͣ���ת��
				GPIO_ResetBits(MOTO_N_GPIO, MOTO_02_PIN_N); //����ͣ���ת��
				SysTask.Moto_State[GLASS_02] = MOTO_STATE_INIT; //����ʱһ��ʱ���ٷ�ת
				SysTask.Moto_StateTime[GLASS_02] = 10;
				break;
			}

			switch (SysTask.Moto_SubState[GLASS_02])
			{
				case MOTO_SUB_STATE_UP:
					if (SysTask.Moto_RunTime[GLASS_02] == 0)
					{
						SysTask.Moto_SubState[GLASS_02] = MOTO_SUB_STATE_WAIT;
						GPIO_ResetBits(MOTO_P_GPIO, MOTO_02_PIN_P); //ֹͣ
						GPIO_ResetBits(MOTO_N_GPIO, MOTO_02_PIN_N); //
				        SysTask.Moto_State[GLASS_02] = MOTO_STATE_STOP;  //���Կ���״̬
					}

					break;

				case MOTO_SUB_STATE_DOWN:
					if ((SysTask.Moto_RunTime[GLASS_02] == 0) || (GPIO_ReadInputDataBit(LOCK_ZERO_GPIO, LOCK_ZERO_02_PIN) == 0))
					{
						SysTask.Moto_SubState[GLASS_02] = MOTO_SUB_STATE_WAIT;
						GPIO_ResetBits(MOTO_P_GPIO, MOTO_02_PIN_P); //ֹͣ
						GPIO_ResetBits(MOTO_N_GPIO, MOTO_02_PIN_N); //
				        SysTask.Moto_State[GLASS_02] = MOTO_STATE_IDLE;  //�����Կ������ɰ�׿�˱�֤
					}

					break;
                    
                case MOTO_SUB_STATE_DEF:
                    break;

				default:
					break;
			}

			break;

		case MOTO_STATE_STOP:
            if (SysTask.Moto_Mode[GLASS_02] != SysTask.Moto_ModeSave[GLASS_02])
            {
                SysTask.Moto_State[GLASS_02] = MOTO_STATE_INIT; //����ʱһ��ʱ���ٷ�ת
                break;
            }

			break;

		case MOTO_STATE_WAIT:
			break;

		case MOTO_STATE_IDLE:
            if (SysTask.Moto_Mode[GLASS_02] != SysTask.Moto_ModeSave[GLASS_02])
            {
                SysTask.Moto_State[GLASS_02] = MOTO_STATE_INIT; //����ʱһ��ʱ���ٷ�ת
                break;
            }
			break;

		default:
			break;
	}
}


/*******************************************************************************
* ����: 
* ����: 
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void Moto_03_Task(void)
{
	static u8 u8State	= 0;

	//_03���
	switch (SysTask.Moto_State[GLASS_03])
	{
		case MOTO_STATE_INIT:
			if (SysTask.Moto_StateTime[GLASS_03] == 0)
			{
				SysTask.Moto_RunTime[GLASS_03] = g_au16MoteTime[SysTask.Moto_Time[GLASS_03]][1];
				SysTask.Moto_State[GLASS_03] = MOTO_STATE_CHANGE_DIR;
				SysTask.Moto_SubState[GLASS_03] = MOTO_SUB_STATE_RUN;
			}

			break;

		case MOTO_STATE_CHANGE_TIME:
			break;

		case MOTO_STATE_CHANGE_DIR:
			if (SysTask.Moto_Mode[GLASS_03] == MOTO_FR_FWD)
			{
				GPIO_SetBits(MOTO_P_GPIO, MOTO_03_PIN_P); //��ת
				GPIO_ResetBits(MOTO_N_GPIO, MOTO_03_PIN_N); //
				SysTask.Moto_State[GLASS_03] = MOTO_STATE_RUN_NOR;
			}
			else if (SysTask.Moto_Mode[GLASS_03] == MOTO_FR_REV)
			{
				GPIO_SetBits(MOTO_N_GPIO, MOTO_03_PIN_N); //��ת
				GPIO_ResetBits(MOTO_P_GPIO, MOTO_03_PIN_P); //
				SysTask.Moto_State[GLASS_03] = MOTO_STATE_RUN_NOR;
			}
			else if (SysTask.Moto_Mode[GLASS_03] == MOTO_FR_FWD_REV)
			{ //����ת
				u8State 			= 0;
				GPIO_SetBits(MOTO_P_GPIO, MOTO_03_PIN_P); //����ת
				GPIO_ResetBits(MOTO_N_GPIO, MOTO_03_PIN_N); //
				SysTask.Moto_State[GLASS_03] = MOTO_STATE_RUN_CHA;
			}

			SysTask.Moto_ModeSave[GLASS_03] = SysTask.Moto_Mode[GLASS_03];
			break;

		case MOTO_STATE_RUN_NOR: //��ͨ����״̬
			if (SysTask.Moto_Mode[GLASS_03] != SysTask.Moto_ModeSave[GLASS_03])
			{
				GPIO_ResetBits(MOTO_P_GPIO, MOTO_03_PIN_P); //����ͣ���ת��
				GPIO_ResetBits(MOTO_N_GPIO, MOTO_03_PIN_N); //����ͣ���ת��
				SysTask.Moto_State[GLASS_03] = MOTO_STATE_INIT; //����ʱһ��ʱ���ٷ�ת
				SysTask.Moto_StateTime[GLASS_03] = 200;
				break;
			}

			switch (SysTask.Moto_SubState[GLASS_03])
			{
				case MOTO_SUB_STATE_RUN:
					if (SysTask.Moto_RunTime[GLASS_03] == 0)
					{
						SysTask.Moto_WaitTime[GLASS_03] = g_au16MoteTime[SysTask.Moto_Time[GLASS_03]][2];
						SysTask.Moto_SubState[GLASS_03] = MOTO_SUB_STATE_WAIT;
						GPIO_ResetBits(MOTO_P_GPIO, MOTO_03_PIN_P); //ֹͣ
						GPIO_ResetBits(MOTO_N_GPIO, MOTO_03_PIN_N); //
					}

					break;

				case MOTO_SUB_STATE_WAIT:
					if (SysTask.Moto_WaitTime[GLASS_03] == 0)
					{
						SysTask.Moto_RunTime[GLASS_03] = g_au16MoteTime[SysTask.Moto_Time[GLASS_03]][1];
						SysTask.Moto_SubState[GLASS_03] = MOTO_SUB_STATE_RUN;

						if (SysTask.Moto_Mode[GLASS_03] == MOTO_FR_FWD)
						{
							GPIO_SetBits(MOTO_P_GPIO, MOTO_03_PIN_P); //��ת
							GPIO_ResetBits(MOTO_N_GPIO, MOTO_03_PIN_N); //
							SysTask.Moto_State[GLASS_03] = MOTO_STATE_RUN_NOR;
						}
						else if (SysTask.Moto_Mode[GLASS_03] == MOTO_FR_REV)
						{
							GPIO_SetBits(MOTO_N_GPIO, MOTO_03_PIN_N); //��ת
							GPIO_ResetBits(MOTO_P_GPIO, MOTO_03_PIN_P); //
							SysTask.Moto_State[GLASS_03] = MOTO_STATE_RUN_NOR;
						}
					}

					break;

				default:
					break;
			}

			break;

		case MOTO_STATE_RUN_CHA: //����ת����״̬
			if (SysTask.Moto_Mode[GLASS_03] != SysTask.Moto_ModeSave[GLASS_03])
			{
				GPIO_ResetBits(MOTO_P_GPIO, MOTO_03_PIN_P); //����ͣ���ת��
				GPIO_ResetBits(MOTO_N_GPIO, MOTO_03_PIN_N); //����ͣ���ת��
				SysTask.Moto_State[GLASS_03] = MOTO_STATE_INIT; //����ʱһ��ʱ���ٷ�ת
				break;
			}

			switch (SysTask.Moto_SubState[GLASS_03])
			{
				case MOTO_SUB_STATE_RUN:
					if (SysTask.Moto_RunTime[GLASS_03] == 0)
					{
						SysTask.Moto_WaitTime[GLASS_03] = g_au16MoteTime[SysTask.Moto_Time[GLASS_03]][2];
						SysTask.Moto_SubState[GLASS_03] = MOTO_SUB_STATE_WAIT;
						GPIO_ResetBits(MOTO_P_GPIO, MOTO_03_PIN_P); //ֹͣ
						GPIO_ResetBits(MOTO_N_GPIO, MOTO_03_PIN_N); //
					}
					else if (SysTask.Moto_StateTime[GLASS_03] == 0)
					{
						if (GPIO_ReadInputDataBit(LOCK_ZERO_GPIO, LOCK_ZERO_03_PIN) == 0) // //��⵽���	
						{

							SysTask.Moto_StateTime[GLASS_03] = SysTask.nTick + MOTO_TRUN_OVER + 500; //��⵽0�㣬һ��ʱ����ٴμ�⣬�ܿ�

                            SysTask.Moto_SubState[GLASS_03] = MOTO_SUB_STATE_DELAY;
            				GPIO_ResetBits(MOTO_N_GPIO, MOTO_03_PIN_N); //ֹͣ
            				GPIO_ResetBits(MOTO_P_GPIO, MOTO_03_PIN_P); //
            				SysTask.Moto3_SubStateTime = MOTO_TRUN_OVER;
						}

					}

					break;
                    
                case MOTO_SUB_STATE_DELAY:
				    if(SysTask.Moto3_SubStateTime != 0)   //��ʱһ��ʱ���ٷ�ת
                        break;
                    
                    if (u8State == 0)
                    {
                        u8State             = 1;
                        GPIO_SetBits(MOTO_N_GPIO, MOTO_03_PIN_N); //��ת
                        GPIO_ResetBits(MOTO_P_GPIO, MOTO_03_PIN_P); //
                    }
                    else 
                    {
                        
                        u8State             = 0;
                        GPIO_SetBits(MOTO_P_GPIO, MOTO_03_PIN_P); //����ת
                        GPIO_ResetBits(MOTO_N_GPIO, MOTO_03_PIN_N); //
                    }
                    SysTask.Moto_SubState[GLASS_03] = MOTO_SUB_STATE_RUN;

                    break;

				case MOTO_SUB_STATE_WAIT:
					if (SysTask.Moto_WaitTime[GLASS_03] == 0)
					{
						u8State 			= 0;
						SysTask.Moto_RunTime[GLASS_03] = g_au16MoteTime[SysTask.Moto_Time[GLASS_03]][1];
						SysTask.Moto_SubState[GLASS_03] = MOTO_SUB_STATE_RUN;
						GPIO_SetBits(MOTO_P_GPIO, MOTO_03_PIN_P); //����ת
						GPIO_ResetBits(MOTO_N_GPIO, MOTO_03_PIN_N); //
					}

					break;

				default:
					break;
			}

			break;

		case MOTO_STATE_STOP:
			if (SysTask.Moto_WaitTime[GLASS_03] == 0)
			{
				GPIO_ResetBits(MOTO_N_GPIO, MOTO_03_PIN_N); //ֹͣ
				GPIO_ResetBits(MOTO_P_GPIO, MOTO_03_PIN_P); //

				SysTask.Moto_State[GLASS_03] = MOTO_STATE_IDLE;
			}

			break;

		case MOTO_STATE_WAIT:
			break;

		case MOTO_STATE_IDLE:
			break;

		default:
			break;
	}
}

/*******************************************************************************
* ����: 
* ����: 
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void Lock_01_Task(void)
{

	switch (SysTask.Lock_State[GLASS_01]) //_01��
	{
		case LOCK_STATE_DETECT:
			if (SysTask.LockMode[GLASS_01] != SysTask.LockModeSave[GLASS_01])
			{
				if (SysTask.LockMode[GLASS_01] == LOCK_ON) //��Ҫ����
				{
					SysTask.Lock_StateTime[GLASS_01] = MOTO_TIME_OUT; //��ʱ
					SysTask.Lock_State[GLASS_01] = LOCK_STATE_DEBOUNSE;
					SysTask.LockModeSave[GLASS_01] = SysTask.LockMode[GLASS_01];
					SysTask.Lock_OffTime[GLASS_01] = LOCK_OFFTIME;
				}
				else //��Ҫ����
				{
					SysTask.LockModeSave[GLASS_01] = SysTask.LockMode[GLASS_01];
					SysTask.Lock_StateTime[GLASS_01] = SysTask.nTick + LOCK_SET_L_TIME;
					SysTask.Lock_State[GLASS_01] = LOCK_STATE_SENT;
				}
			}

			break;

		case LOCK_STATE_DEBOUNSE: //
			if (SysTask.Lock_StateTime[GLASS_01] != 0)
			{
                
                if (GPIO_ReadInputDataBit(LOCK_ZERO_GPIO, LOCK_ZERO_01_PIN) != 0) //Ҫ��ص�ԭ��ſ���
                    break;

                if(g_bSensorState == TRUE)
                {
                    if (GPIO_ReadInputDataBit(LOCK_ZERO_GPIO, LOCK_ZERO_01_PIN) != 0) //Ҫ��ص�ԭ��ſ���
                        break;
                }else if (   ((SysTask.Moto_SubState[GLASS_01] != MOTO_SUB_STATE_ZERO_R) && (SysTask.Moto_SubState[GLASS_01] != MOTO_SUB_STATE_ZERO_R)) //
    			          || (SysTask.Moto_RunTime[GLASS_01] != 0) )// û�м�⵽��������£�Ĭ��ת�����м俪��
    			    
    			{
                    break;
                }
                          
#if DEBUG 
                u3_printf(" lock ��������������\n");
#endif
				SysTask.Moto_WaitTime[GLASS_01] = MOTO_DEBOUNCE;
				SysTask.Moto_State[GLASS_01] = MOTO_STATE_STOP; //ֹͣת��
				
				SysTask.Lock_State[GLASS_01] = LOCK_STATE_OPEN;

					
				GPIO_ResetBits(LOCK_GPIO, LOCK_01_PIN); //����
				SysTask.Lock_StateTime[GLASS_01] = SysTask.nTick + LOCK_HAL_OPEN;
			}
            else{
                     
#if DEBUG 
                u3_printf(" ǿ�ƿ������˳�����������\n");
#endif
				SysTask.Moto_WaitTime[GLASS_01] = MOTO_DEBOUNCE;
				SysTask.Moto_State[GLASS_01] = MOTO_STATE_STOP; //ֹͣת��
				
				SysTask.Lock_State[GLASS_01] = LOCK_STATE_OPEN;

					
				GPIO_ResetBits(LOCK_GPIO, LOCK_01_PIN); //����
				SysTask.Lock_StateTime[GLASS_01] = SysTask.nTick + LOCK_HAL_OPEN;
            }

			break;

        case LOCK_STATE_OPEN:
			if (SysTask.Lock_StateTime[GLASS_01] != 0)
				break;
            
#if DEBUG 
                        u3_printf(" lock ֹͣ�����źŷ���\n");
#endif
    		GPIO_SetBits(LOCK_GPIO, LOCK_01_PIN); //ֹͣ�����źŷ���
    		SysTask.Lock_State[GLASS_01] = LOCK_STATE_DETECT;
        
			if (SysTask.SendState == SEND_IDLE)
				SysTask.SendState = SEND_TABLET_ZERO;

			g_FingerAck 		= ACK_SUCCESS;
			g_AckType			= ZERO_ACK_TYPE;
			g_GlassSendAddr 	= GLASS_01;
    
            break;
		case LOCK_STATE_SENT: 
			if (SysTask.Lock_StateTime[GLASS_01] != 0)
				break;
            
#if DEBUG 
                        u3_printf(" lock ����������������\n");
#endif
                GPIO_ResetBits(LOCK_GPIO, LOCK_01_PIN); //�����͹����� �Ƿ���һ����ʱ��ĸߵ�ƽ 

				SysTask.Lock_State[GLASS_01] = LOCK_STATE_OFF;
				SysTask.Lock_StateTime[GLASS_01] = LOCK_HAL_OPEN; ////��ʱ


			break;

		case LOCK_STATE_OFF: //
			if (SysTask.Lock_StateTime[GLASS_01] != 0)
				break;
            
    		GPIO_SetBits(LOCK_GPIO, LOCK_01_PIN); //ֹͣ�����źŷ���
			SysTask.Lock_StateTime[GLASS_01] = SysTask.nTick + LOCK_SET_L_TIME;
			SysTask.Lock_State[GLASS_01] = LOCK_STATE_DETECT;

			// ��Ϊת���ĵ�Ƭ��û��Ӧ���������  ��������������ϴ���ƽ��ˣ�Ĭ����ת��һ��ʼ���ǳɹ���
			//			if (SysTask.SendState == SEND_IDLE)
			//				SysTask.SendState = SEND_TABLET_FINGER;
			//			g_FingerAck = ACK_SUCCESS;
			//			g_AckType = ZERO_ACK_TYPE;
			//			g_GlassSendAddr = GLASS_01;
			//	�����ת������������ת������,��Ҫ��ʱһ��ʱ�䣬��������ȫ ֮����ת��
			
			SysTask.Moto_StateTime[GLASS_01] = MOTO_LOCK_SUCCESS;
			SysTask.Moto_State[GLASS_01] = MOTO_STATE_INIT; //���¿�ʼת��
			break;

		default:
			break;
	}
}


/*******************************************************************************
* ����: 
* ����: 
* �β�:	ֻ�ÿ���	
* ����: ��
* ˵��: 
*******************************************************************************/
void Lock_02_Task(void)
{
	switch (SysTask.Lock_State[GLASS_02]) //_02��
	{
		case LOCK_STATE_DETECT:
			if (SysTask.LockMode[GLASS_02] != SysTask.LockModeSave[GLASS_02])
			{
				if (SysTask.LockMode[GLASS_02] == LOCK_ON)
				{
				    if (SysTask.Moto_State[GLASS_02] != MOTO_STATE_STOP) //����������ֹͣ�ɿ���״̬
					{
#if DEBUG_MOTO2 
                        u3_printf("Lock_02_Task ����ʧ��\n");
#endif
					    SysTask.LockMode[GLASS_02] = LOCK_OFF;  //����ʧ��
						break;
					}
                    
#if DEBUG_MOTO2 
                                            u3_printf(" Lock_02_Task ����\n");
#endif

					SysTask.Lock_StateTime[GLASS_02] = LOCK_OPEN_TIME; //
					SysTask.Lock_State[GLASS_02] = LOCK_STATE_DEBOUNSE;
					GPIO_ResetBits(LOCK_GPIO, LOCK_02_PIN);
					SysTask.LockModeSave[GLASS_02] = SysTask.LockMode[GLASS_02];
					SysTask.Lock_OffTime[GLASS_02] = 0;
				}
				else //��Ҫ������Ĭ������� ��׿�˲��ᷢ�����������
				{
#if DEBUG_MOTO2 
                        u3_printf(" Lock_02_Task ��Ҫ����\n");
#endif
					SysTask.LockModeSave[GLASS_02] = SysTask.LockMode[GLASS_02];
					SysTask.Lock_StateTime[GLASS_02] = 0;
					SysTask.Lock_State[GLASS_02] = LOCK_STATE_SENT;
				}
			}

			break;

		case LOCK_STATE_DEBOUNSE:
			if (SysTask.Lock_StateTime[GLASS_02] == 0)
			{
#if DEBUG_MOTO2 
                        u3_printf(" Lock_02_Task �ָ�����״̬\n");
#endif
                    SysTask.LockMode[GLASS_02] = LOCK_OFF;  //�ָ�����״̬
					SysTask.LockModeSave[GLASS_02] = SysTask.LockMode[GLASS_02];
			        GPIO_SetBits(LOCK_GPIO, LOCK_02_PIN);
					SysTask.Lock_State[GLASS_02] = LOCK_STATE_DETECT;

					if (SysTask.SendState == SEND_IDLE)
						SysTask.SendState = SEND_TABLET_ZERO;

					g_FingerAck 		= ACK_SUCCESS;
					g_AckType			= ZERO_ACK_TYPE;
					g_GlassSendAddr 	= GLASS_02;
			}

			break;

		case LOCK_STATE_SENT: //
			if (SysTask.Lock_StateTime[GLASS_02] != 0)
				break;
            
#if DEBUG_MOTO2 
                                    u3_printf(" Lock_02_Task ����\n");
#endif

			SysTask.Lock_State[GLASS_02] = LOCK_STATE_OFF;
			GPIO_ResetBits(LOCK_GPIO, LOCK_02_PIN );
			SysTask.Lock_StateTime[GLASS_02] = LOCK_OPEN_TIME; //

			break;

		case LOCK_STATE_OFF: //
			if (SysTask.Lock_StateTime[GLASS_02] != 0)
				break;
            
#if DEBUG_MOTO2 
                                    u3_printf(" Lock_02_Task �ָ�����״̬\n");
#endif
            
            SysTask.LockMode[GLASS_02] = LOCK_OFF;  //�ָ�����״̬
            SysTask.LockModeSave[GLASS_02] = SysTask.LockMode[GLASS_02];

			GPIO_SetBits(LOCK_GPIO, LOCK_02_PIN);
			SysTask.Lock_State[GLASS_02] = LOCK_STATE_DETECT;
			break;

		default:
			break;
	}
}


/*******************************************************************************
* ����: 
* ����: 
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void Lock_03_Task(void)
{
	static u8 u8ClockCnt = 0;
	static u8 u8ClockPer = 0;
	switch (SysTask.Lock_State[GLASS_03]) //_03��
	{
		case LOCK_STATE_DETECT:
			if (SysTask.LockMode[GLASS_03] != SysTask.LockModeSave[GLASS_03])
			{
				if (SysTask.LockMode[GLASS_03] == LOCK_ON)
				{
					if (SysTask.Moto_WaitTime[GLASS_03] != 0) //��������ֹͣ״̬
					{
						SysTask.Moto_WaitTime[GLASS_03] = 0;
					}

					if (GPIO_ReadInputDataBit(LOCK_ZERO_GPIO, LOCK_ZERO_03_PIN) == 0) // //��⵽��� ����
					{
						SysTask.Lock_StateTime[GLASS_03] = MOTO_TIME_OUT; //
						SysTask.Moto_WaitTime[GLASS_03] = MOTO_DEBOUNCE;
						SysTask.Lock_State[GLASS_03] = LOCK_STATE_DEBOUNSE;
						SysTask.Moto_State[GLASS_03] = MOTO_STATE_STOP; //ֹͣת��
						SysTask.LockModeSave[GLASS_03] = SysTask.LockMode[GLASS_03];
						SysTask.Lock_OffTime[GLASS_03] = LOCK_OFFTIME;
					}
				}
				else 
				{
					SysTask.LockModeSave[GLASS_03] = SysTask.LockMode[GLASS_03];
					GPIO_ResetBits(LOCK_GPIO, LOCK_03_PIN);
					u8ClockCnt			= 0;
					u8ClockPer			= 0;
					SysTask.Lock_StateTime[GLASS_03] = SysTask.nTick + LOCK_SET_L_TIME;
					SysTask.Lock_State[GLASS_03] = LOCK_STATE_SENT;
				}
			}

			break;

		case LOCK_STATE_DEBOUNSE:
			if (SysTask.Lock_StateTime[GLASS_03] != 0)
			{
				if (GPIO_ReadInputDataBit(LOCK_ZERO_GPIO, LOCK_ZERO_03_PIN) == 0) // //��⵽��� ����
				{
					GPIO_SetBits(LOCK_GPIO, LOCK_03_PIN);
					SysTask.Lock_State[GLASS_03] = LOCK_STATE_DETECT;

					if (SysTask.SendState == SEND_IDLE)
						SysTask.SendState = SEND_TABLET_ZERO;

					g_FingerAck 		= ACK_SUCCESS;
					g_AckType			= ZERO_ACK_TYPE;
					g_GlassSendAddr 	= GLASS_03;
				}
			}else{
                                         
#if DEBUG 
                u3_printf(" ��ʱǿ���˳�����������\n");
#endif
                SysTask.LockModeSave[GLASS_03] = LOCK_OFF;
                SysTask.LockMode[GLASS_03] = LOCK_OFF;
                SysTask.Lock_State[GLASS_03] = LOCK_STATE_DETECT;
            }

			break;

		case LOCK_STATE_SENT: //����3����������浥Ƭ����ִ�п���
			if (SysTask.Lock_StateTime[GLASS_03] != 0)
				break;

			if (u8ClockCnt < 4)
			{
				u8ClockCnt++;

				if (u8ClockPer == 0)
				{
					u8ClockPer			= 1;
					GPIO_SetBits(LOCK_GPIO, LOCK_03_PIN);
					SysTask.Lock_StateTime[GLASS_03] = SysTask.nTick + LOCK_SET_H_TIME;
				}
				else 
				{
					u8ClockPer			= 0;
					GPIO_ResetBits(LOCK_GPIO, LOCK_03_PIN);
					SysTask.Lock_StateTime[GLASS_03] = SysTask.nTick + LOCK_SET_L_TIME;
				}
			}
			else 
			{
				SysTask.Lock_State[GLASS_03] = LOCK_STATE_OFF;
				GPIO_SetBits(LOCK_GPIO, LOCK_03_PIN);
				SysTask.Lock_StateTime[GLASS_03] = SysTask.nTick + LOCK_SET_OFF_TIME; //ֹͣ��ƽ

			}

			break;

		case LOCK_STATE_OFF: //
			if (SysTask.Lock_StateTime[GLASS_03] != 0)
				break;

			GPIO_ResetBits(LOCK_GPIO, LOCK_03_PIN);
			SysTask.Lock_StateTime[GLASS_03] = SysTask.nTick + LOCK_SET_L_TIME;
			SysTask.Lock_State[GLASS_03] = LOCK_STATE_DETECT;

			//	�����ת������������ת������
			SysTask.Moto_State[GLASS_03] = MOTO_STATE_INIT; //���¿�ʼת��
			break;

		default:
			break;
	}
}


/*******************************************************************************
* ����: 
* ����: 
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void SysSaveDataTask(void)
{
	static u8 u8Status	= 0;
	static u8 u8count	= 0;
	u8 i;

	if (SysTask.bEnSaveData == FALSE)
		return;

	switch (u8Status)
	{
		case SAVE_INIT:
			memset(SysTask.FlashData.u8FlashData, 0, sizeof(SysTask.FlashData.u8FlashData)); //��0

			//			  memcpy(SysTask.FlashData.u8FlashData, SysTask.WatchState, sizeof(SysTask.WatchState));
			for (i = 0; i < GLASS_MAX_CNT; i++)
			{
				SysTask.FlashData.Data.WatchState[i].MotoTime = SysTask.WatchState[i].MotoTime;
				SysTask.FlashData.Data.WatchState[i].u8Dir = SysTask.WatchState[i].u8Dir;
				SysTask.FlashData.Data.WatchState[i].u8LockState = SysTask.WatchState[i].u8LockState;
			}

			SysTask.FlashData.Data.u16LedMode = LED_MODE_ON; //LEDģʽ
			u8Status = SAVE_WRITE;
			break;

		case SAVE_WRITE:
			if (FLASH_WriteMoreData(g_au32DataSaveAddr[SysTask.u16AddrOffset], SysTask.FlashData.u16FlashData, DATA_READ_CNT + LED_MODE_CNT + PASSWD_CNT))
			{
				u8count++;
				u8Status			= SAVE_WAIT;
				SysTask.u16SaveWaitTick = 100;		// 100ms ��ʱ������д��
			}
			else 
			{
				u8Status			= SAVE_EXIT;
			}

			break;

		case SAVE_WAIT:
			if (SysTask.u16SaveWaitTick == 0)
			{
				u8Status			= SAVE_WRITE;

				if (u8count++ >= 7) //ÿҳ����8��
				{
					if (++SysTask.u16AddrOffset < ADDR_MAX_OFFSET) //��5ҳ
					{
						for (i = 0; i < 3; i++) //��д8�����
						{
							if (!FLASH_WriteMoreData(g_au32DataSaveAddr[0], &SysTask.u16AddrOffset, 1))
								break;

							delay_ms(20);
						}
					}
					else 
					{
						u8Status			= SAVE_EXIT;
					}
				}
			}

			break;

		case SAVE_EXIT:
			SysTask.bEnSaveData = FALSE;
			u8Status = SAVE_INIT;
			u8count = 0;
			break;

		default:
			u8Status = SAVE_EXIT;
			break;
	}
}


/*******************************************************************************
* ����: 
* ����: 
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void MainTask(void)
{
	SysSaveDataTask();
	LedTask();
	SendTabletTask();								//���͸�ƽ������
	TabletToHostTask(); 							//����ƽ������������
//	HostToTabletTask(); 							//�������͸��ӻ�������������
//	FingerTouchTask();								//ָ��ģ��
	Moto_01_Task(); //ֻ������ת�ģ�ת��Ȧ
	Moto_02_Task(); //�м�� ������ �½�ʱ�и������ ������ʹ��ʱ�����ж�
	Moto_03_Task(); //��׼�ģ���������ת��Ҳ�����趨ʱ��ת����ʱ�䣬ת����ʱ��ͣ����ʱ��ġ�

    PWM_01_Task();
	Lock_01_Task();
	Lock_02_Task();
	Lock_03_Task();
}



/*******************************************************************************
* ����: 
* ����: 
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void SysInit(void)
{
	u8 i, j;

	SysTask.u16AddrOffset = FLASH_ReadHalfWord(g_au32DataSaveAddr[0]); //��ȡ
	memset(SysTask.FlashData.u8FlashData, 0, sizeof(SysTask.FlashData.u8FlashData));

	if ((SysTask.u16AddrOffset == 0xffff) || (SysTask.u16AddrOffset >= ADDR_MAX_OFFSET) ||
		 (SysTask.u16AddrOffset == 0)) //��ʼ״̬ flash��0xffff
	{
		SysTask.u16AddrOffset = 1;					//�ӵ�һҳ��ʼд��

		for (i = 0; i < 8; i++) //��д8�����
		{
			if (!FLASH_WriteMoreData(g_au32DataSaveAddr[0], &SysTask.u16AddrOffset, 1))
				break;

			delay_ms(20);
		}

		for (i = 0; i < GLASS_MAX_CNT; i++)
		{
			SysTask.WatchState[i].u8LockState = 0;
			SysTask.WatchState[i].u8Dir = MOTO_FR_FWD;
			SysTask.WatchState[i].MotoTime = MOTO_TIME_650;
            
            if(i == 0){
                SysTask.WatchState[i].u8Dir = MOTO_FR_FWD_REV;   //ֻ������ת��ֹͣģʽ������ѡ��
            }
		}

		//		  memcpy(SysTask.FlashData.u8FlashData, SysTask.WatchState, sizeof(SysTask.WatchState));
		for (i = 0; i < GLASS_MAX_CNT; i++)
		{
			SysTask.FlashData.Data.WatchState[i].MotoTime = SysTask.WatchState[i].MotoTime;
			SysTask.FlashData.Data.WatchState[i].u8Dir = SysTask.WatchState[i].u8Dir;
			SysTask.FlashData.Data.WatchState[i].u8LockState = SysTask.WatchState[i].u8LockState;
		}

		SysTask.FlashData.Data.u16LedMode = LED_MODE_ON; //LEDģʽ

		FLASH_WriteMoreData(g_au32DataSaveAddr[SysTask.u16AddrOffset], SysTask.FlashData.u16FlashData, sizeof(SysTask.FlashData.u16FlashData) / 2);
	}

	FLASH_ReadMoreData(g_au32DataSaveAddr[SysTask.u16AddrOffset], SysTask.FlashData.u16FlashData, sizeof(SysTask.FlashData.u16FlashData) / 2);


	for (i = 0; i < DATA_READ_CNT; i++)
	{
		if (SysTask.FlashData.u16FlashData[i] == 0xFFFF) //������FLASH�д�˵��ҳ�������⣬�ȴ�����ʱ�ٽ�������У��д����һҳ
		{
			for (j = 0; j < GLASS_MAX_CNT; j++)
			{
				SysTask.WatchState[j].u8LockState = 0;
				SysTask.WatchState[j].u8Dir = MOTO_FR_FWD;
				SysTask.WatchState[j].MotoTime = MOTO_TIME_650;
                if(i == 0){
                    SysTask.WatchState[i].u8Dir = MOTO_FR_FWD_REV;   //ֻ������ת��ֹͣģʽ������ѡ��
                }
			}

			//memcpy(SysTask.FlashData.u8FlashData, SysTask.WatchState, sizeof(SysTask.WatchState));
			for (i = 0; i < GLASS_MAX_CNT; i++)
			{
				SysTask.FlashData.Data.WatchState[i].MotoTime = SysTask.WatchState[i].MotoTime;
				SysTask.FlashData.Data.WatchState[i].u8Dir = SysTask.WatchState[i].u8Dir;
				SysTask.FlashData.Data.WatchState[i].u8LockState = SysTask.WatchState[i].u8LockState;
			}

			SysTask.FlashData.Data.u16LedMode = LED_MODE_ON; //LEDģʽ

			break;
		}

	}

	//	  ʹ��memcpy�ᵼ��TIM 2 TICk�޷������жϣ����������ж�û���ܵ�Ӱ�죬ԭ��δ֪������8����û������
	//	  ����9�����Ͼͻ���֡�
	//	  memcpy(SysTask.WatchState, SysTask.FlashData.u8FlashData, 10);
	for (i = 0; i < GLASS_MAX_CNT; i++)
	{
		SysTask.WatchState[i].MotoTime = SysTask.FlashData.Data.WatchState[i].MotoTime;
		SysTask.WatchState[i].u8Dir = SysTask.FlashData.Data.WatchState[i].u8Dir;

		//		  SysTask.WatchState[i].u8LockState   = SysTask.FlashData.Data.WatchState[i].u8LockState;  //��״̬���ڲ����ϴ������±ߵ�Ƭ����֤
	}

	SysTask.LED_Mode	= SysTask.FlashData.Data.u16LedMode;

	if ((SysTask.LED_Mode != LED_MODE_ON) && (SysTask.LED_Mode != LED_MODE_OFF))
	{
		SysTask.LED_Mode	= LED_MODE_ON;			//LEDģʽ
	}

	for (i = 0; i < GLASS_MAX_CNT; i++)
	{
		SysTask.Moto_Mode[i] = (MotoFR) (SysTask.WatchState[i].u8Dir);
		SysTask.Moto_Time[i] = (MotoTime_e) (SysTask.WatchState[i].MotoTime);
		SysTask.Moto_RunTime[i] = g_au16MoteTime[SysTask.Moto_Time[i]][1];
		SysTask.Moto_WaitTime[i] = g_au16MoteTime[SysTask.Moto_Time[i]][2];
		SysTask.LockMode[i] = LOCK_OFF; 			//��״̬���ڴ˸��£���Ϊ������Ҫ����֮��
	}

	SysTask.TouchState	= TOUCH_MATCH_INIT;
	SysTask.TouchSub	= TOUCH_SUB_INIT;
	SysTask.bEnFingerTouch = FALSE;
	SysTask.nWaitTime	= 0;
	SysTask.nTick		= 0;
	SysTask.u16SaveTick = 0;
	SysTask.u16PWMVal   = 0;
	SysTask.bEnSaveData = FALSE;
    SysTask.Moto3_SubStateTime = 0;

   
	SysTask.SendState	= SEND_IDLE;
	SysTask.SendSubState = SEND_SUB_INIT;
	SysTask.bTabReady	= FALSE;
	SysTask.LED_ModeSave = LED_MODE_DEF;			// ��֤��һ�ο�������ģʽ

	for (i = 0; i < GLASS_MAX_CNT; i++)
	{
		SysTask.LockModeSave[i] = LOCK_OFF;
		SysTask.LockMode[i] = LOCK_OFF;
	}


	for (i = 0; i < GLASS_MAX_CNT; i++)
	{
		SysTask.PWM_State[i] = PWM_STATE_INIT;
	}
	for (i = 0; i < GLASS_MAX_CNT; i++)
	{
		SysTask.PWM_RunTime[i] = 0;
	}
    

	for (i = 0; i < GLASS_MAX_CNT; i++)
	{
		SysTask.Moto_StateTime[i] = 0;
		SysTask.Moto_State[i] = MOTO_STATE_INIT;	//��֤����������ת
		SysTask.Moto_SubState[i] = MOTO_SUB_STATE_RUN;
		SysTask.Lock_StateTime[i] = 0;				//		  
	}
    SysTask.Moto_State[GLASS_02] = MOTO_STATE_STOP;	//����������Ĭ���ϵ��޶���
    SysTask.Moto_WaitTime[GLASS_02] = 0;
	//	  SysTask.u16BootTime = 3000;	//3��ȴ��ӻ�������ȫ
}


