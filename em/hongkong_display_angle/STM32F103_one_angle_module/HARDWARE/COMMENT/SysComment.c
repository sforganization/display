
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "usart1.h" 
#include "usart3.h" 
#include "delay.h"

#include "SysComment.h"
#include "stm32f10x_it.h"
#include "stm32f10x_iwdg.h"
#include "san_flash.h"
#include "ws2812b.h"




#define SLAVE_WAIT_TIME 		3000		//3s �ӻ�Ӧ��ʱ��

#define LOCK_SET_L_TIME 		50			//�͵�ƽʱ�� 25 ~ 60ms
#define LOCK_SET_H_TIME 		50			//�ߵ�ƽʱ��
#define LOCK_SET_OFF_TIME		1200		//����Ƭ����Ӧʱ�䣬�������֮����ת

#define MOTO_UP_TIME			800			//�������ʱ��
#define MOTO_DOWN_TIME			1500		//����½�ʱ��

#define POWER_ON_DELAY			250 		//�ϵ�һ��ʱ����ȥʹ�� ָ��ģ�飬ʵ��1020A ��Ҫ200ms					 1020C ��Ҫ100ms ,ע�����Ĳ�����Ҫ���϶�β�������ֹоƬ������ɵ�Ӱ��

#define LOCK_OPEN_TIME			500			//�������͵�ƽʱ��

#define STREAM_TIME 			150			//��ˮʱ��
#define BLINK_TIME				240			//��˸ʱ��
#define FAILED_TIME 			160			//��˸ʱ��

#define BLINK_CNT_1 			2			//��˸����
#define BLINK_CNT_5 			10			//��˸���� 00 /2 =5��
#define BLINK_CNT_6 			12			//��˸���� 00 /2 =5��
#define BLINK_CNT_8 			16			//��˸���� 00 /2 =5��
#define BLINK_CNT_10			20			//��˸����


#define KEY_PRESS_TIME			6000		//��������ʱ��	3S��������������û�
#define KEY_DEBOUCH_TIME		40			//��������ʱ��40ms
#define KEY_SHORT_TIME			3000		//�̰�ʱ��


#define ROOT_ID_NUM_1			1			//1020A ��ΪҪ̧��¼������ָ�ƣ������൱��һ��ָ��ռ������ID
#define ROOT_ID_NUM_2			2


#define RECV_PAKAGE_CHECK		10	 // 	10�����ݼ���

static bool 	g_bSensorState = TRUE;

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


char			priBuf[120] =
{
	0
};


PollEvent_t 	g_FingerEvent;

//�ڲ�����
void SysTickConfig(void);


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


/*******************************************************************************
* ����: 
* ����: 
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void FingerTouchTask(void)
{
	int retVal			= 0;
	static u8 u8MatchCnt = 0;						//ƥ��ʧ�ܴ�����Ĭ��ƥ��MATCH_FINGER_CNT�� 
	static u8 u8SetCnt	= 0;						//����ʧ�ܴ���
	int userID			= 0XFFFF;
	FingerData_t * pfingerData;

	switch (SysTask.TouchState)
	{
		case TOUCH_MATCH_INIT:
			if (SysTask.u32TouchWaitTime != 0)
				break;

			if (PS_Sta) //��ָ�ư���
			{
#if DEBUG
				u3_printf("TOUCH_MATCH_INIT �а���\n");
#endif

				if (SysTask.LockMode[GLASS_01] == LOCK_ON)
				{
#if DEBUG
					u3_printf("TOUCH_MATCH_INIT ִ�й���\n");
#endif

					SysTask.Stream_State = STREAM_STATE_INIT; //����
					SysTask.u32StreamTime = 0;

					SysTask.LockMode[GLASS_01] = LOCK_OFF; //�Ѿ�������������԰������� 
					SysTask.u32TouchWaitTime = SysTask.nTick + 3000; //��ʱһ��ʱ�� �ɹ�֮���ֹ���Ϲ���
					break;
				}

				SysTask.u32TouchWaitTime = SysTask.nTick + POWER_ON_DELAY; //һ��ʱ���ټ�⣬ ��ָ��ģ��MCU�ϵ���ȫ ,ʵ��1020A��Ҫ200ms	1020c 100ms
				FINGER_POWER_ON();					//����ָ��ģ���Դ
				SysTask.TouchState	= TOUCH_MATCH_AGAIN;
				u8MatchCnt			= 0;

			}

			break;

		case TOUCH_MATCH_AGAIN:
			if (SysTask.u32TouchWaitTime == 0) //��ָ�ư���
			{
#if 0
				GPIO_SetBits(GPIOC, GPIO_Pin_13);
#endif

				//u8Ensure			= Finger_Search(&userID); //1��N����ָ�ƿ�
				if (g_FingerEvent.bMutex == FALSE)
				{
					g_FingerEvent.bMutex = TRUE;	//������ֻ����һ��
					g_FingerEvent.MachineCtrl.u32Timeout = FINGER_REV_TIME_OUT; //1S ��ʱ  ,5�ξ���6S
					g_FingerEvent.MachineRun.u32StartTime = SysTask.u32SysTimes;
					g_FingerEvent.MachineRun.bFinish = FALSE;
					g_FingerEvent.MachineCtrl.CallBack(FINGER_SEARCHING, &g_FingerEvent);
				}
				else 
				{
					g_FingerEvent.MachineRun.u32LastTime = SysTask.u32SysTimes;
					g_FingerEvent.MachineCtrl.CallBack(FINGER_SEARCHING, &g_FingerEvent);
				}

				if (g_FingerEvent.MachineRun.bFinish == TRUE)
				{
					if (g_FingerEvent.MachineRun.u16RetVal == EVENT_TIME_OUT) //��ʱ�˳�
					{
#if DEBUG
						u3_printf("��ʱ�˳�����������\n");
#endif
					}

					pfingerData 		= ((FingerData_t *) g_FingerEvent.MachineCtrl.pData);
					userID				= pfingerData->u8FingerID;
					g_FingerEvent.bMutex = FALSE;
				}
				else 
				{
					break;
				}


#if 0
				GPIO_ResetBits(GPIOC, GPIO_Pin_13);
#endif

				if (g_FingerEvent.MachineRun.u16RetVal == TRUE) //ƥ��ɹ�
				{
					pfingerData 		= ((FingerData_t *) g_FingerEvent.MachineCtrl.pData);
					userID				= pfingerData->u8FingerID;

#if DEBUG
					u3_printf("TOUCH_MATCH_AGAIN ƥ��ɹ���lock���� ID %d\n", userID);
#endif

					SysTask.Stream_State = STREAM_STATE_OPEN_LOCK; //������ȫ��

					//SysTask.u16FingerfCnt = seach.pageID;
					SysTask.u32TouchWaitTime = SysTask.nTick + 3000; //��ʱһ��ʱ�� �ɹ�֮���ֹ���Ϲ���
					SysTask.TouchSub	= TOUCH_SUB_INIT;
					SysTask.TouchState	= TOUCH_MATCH_INIT; //����ƥ��
					SysTask.LockMode[GLASS_01] = LOCK_ON; //ƥ��ɹ������Կ���

					FINGER_POWER_OFF(); 			//�ر�ָ��ģ���Դ
				}
				else 
				{
#if DEBUG
					u3_printf("TOUCH_MATCH_AGAIN ƥ��ʧ��\n");
#endif

					SysTask.u32TouchWaitTime = SysTask.nTick + 100; //��ʱһ��ʱ�� ����ֳ�ʶ���ˣ�������һ���˼�⣬

					//ƥ��ʧ��
					if (u8MatchCnt++ >= MATCH_FINGER_CNT) // 50 * 3 ��Լ200MS��
					{
						SysTask.Stream_State = STREAM_STATE_FAILED; //����˸��ʾ
						SysTask.u8StreamShoWCnt = BLINK_CNT_8;
						FINGER_POWER_OFF(); 		//�ر�ָ��ģ���Դ
						SysTask.TouchState	= TOUCH_MATCH_INIT; //����ƥ��
						SysTask.u32TouchWaitTime = SysTask.nTick + 500; //��ʱһ��ʱ�� ����ֳ�ʶ���ˣ�������һ���˼�⣬
					}
				}
			}

			break;

		case TOUCH_ADD_USER:
			{
				switch (SysTask.TouchSub)
				{
					case TOUCH_SUB_INIT: // *
						if (SysTask.u16FingerfCnt > FINGER_ID_MAX - 2) //��Ϊһ��ָ��Ҫ¼�����Σ���̧���ٰ���ȥ�Ķ����� ������Ҫ��ȥ2
						{
#if DEBUG
							u3_printf("TOUCH_ADD_USER ");
							snprintf(priBuf, 50, "SysTask.u16FingerfCnt : %d\n", SysTask.u16FingerfCnt);
							u3_printf(priBuf);
							u3_printf("\n");
							u3_printf("�û�����\n");
#endif

							SysTask.Stream_State = STREAM_STATE_FAILED; //����˸��ʾ
							SysTask.u8StreamShoWCnt = BLINK_CNT_8;

							SysTask.TouchState	= TOUCH_MATCH_INIT; //����ƥ��
							SysTask.u32TouchWaitTime = SysTask.nTick + POWER_ON_DELAY; //��ʱһ��ʱ��
							return;
						}
						else if ((PS_Sta) && (SysTask.u32TouchWaitTime == 0))
						{
#if DEBUG
							u3_printf("TOUCH_ADD_USER ");
							snprintf(priBuf, 50, "SysTask.u16FingerfCnt : %d\n", SysTask.u16FingerfCnt);
							u3_printf(priBuf);
							u3_printf("����ָ��ģ���Դ-->���\n");
#endif

							FINGER_POWER_ON();		//����ָ��ģ���Դ
							SysTask.u32TouchWaitTime = POWER_ON_DELAY; //��ʱһ��ʱ����ȥ�����ظ�ģʽ
							u8MatchCnt			= 0;
							SysTask.u16FingerfCnt++;
							SysTask.TouchSub	= TOUCH_SUB_SETMODE_ONE;
							u8SetCnt			= 0;
							SysTask.Stream_State = STREAM_STATE_BLINK_TWO; //
						}

						break;

					case TOUCH_SUB_SETMODE_ONE: // *
						if (SysTask.u32TouchWaitTime == 0)
						{
							if (g_FingerEvent.bMutex == FALSE)
							{
								g_FingerEvent.bMutex = TRUE; //������ֻ����һ��
								g_FingerEvent.MachineCtrl.u32Timeout = FINGER_REV_TIME_OUT; //1S ��ʱ  ,5�ξ���6S
								g_FingerEvent.MachineRun.u32StartTime = SysTask.u32SysTimes;
								g_FingerEvent.MachineRun.bFinish = FALSE;
								g_FingerEvent.MachineCtrl.CallBack(FINGER_MODE_SET, &g_FingerEvent);
							}
							else 
							{
								g_FingerEvent.MachineRun.u32LastTime = SysTask.u32SysTimes;
								g_FingerEvent.MachineCtrl.CallBack(FINGER_MODE_SET, &g_FingerEvent);
							}

							if (g_FingerEvent.MachineRun.bFinish == TRUE)
							{
								if (g_FingerEvent.MachineRun.u16RetVal == EVENT_TIME_OUT) //��ʱ�˳�
								{
#if DEBUG
									u3_printf("��ʱ�˳�����������\n");
#endif
								}

								g_FingerEvent.bMutex = FALSE;
							}
							else 
							{
								break;				//�ȴ��²��ϴ��Ľ��״̬
							}

							u8SetCnt++;

							//if (!Finger_ModeSet())
							if (g_FingerEvent.MachineRun.u16RetVal == TRUE)
							{
#if DEBUG
								u3_printf("�����ظ�ģʽ�ɹ�\n");
								snprintf(priBuf, 10, "CNT : %d\n", u8SetCnt);
								u3_printf(priBuf);
								u3_printf("\n");
#endif

#if DEBUG
								u3_printf("��һ���ɼ�ָ��\n");
#endif

								SysTask.TouchSub	= TOUCH_SUB_ENTER;
							}
							else 
							{
#if DEBUG
								u3_printf("�����ظ�ģʽʧ��\n");
#endif

								if (u8SetCnt <= 5)
									SysTask.u32TouchWaitTime = 100;
								else 
									SysTask.TouchSub = TOUCH_SUB_ENTER;
							}
						}

						break;

					case TOUCH_SUB_ENTER: //
						if ((SysTask.u32TouchWaitTime == 0)) //ʱ�䵽ȥ�ɼ�
						{
							if (g_FingerEvent.bMutex == FALSE)
							{
								SysTask.Stream_State = STREAM_STATE_BLINK_TWO; //
								g_FingerEvent.bMutex = TRUE; //������ֻ����һ��
								g_FingerEvent.MachineCtrl.u32Timeout = FINGER_REV_TIME_OUT * 5; //1S ��ʱ  ,5�ξ���6S
								g_FingerEvent.MachineRun.u32StartTime = SysTask.u32SysTimes;
								g_FingerEvent.MachineRun.bFinish = FALSE;
								pfingerData 		= ((FingerData_t *) g_FingerEvent.MachineCtrl.pData);
								pfingerData->u8FingerID = SysTask.u16FingerfCnt;
								g_FingerEvent.MachineCtrl.CallBack(FINGER_ENROLL, &g_FingerEvent);
							}
							else 
							{
								pfingerData 		= ((FingerData_t *) g_FingerEvent.MachineCtrl.pData);
								pfingerData->u8FingerID = SysTask.u16FingerfCnt;
								g_FingerEvent.MachineRun.u32LastTime = SysTask.u32SysTimes;
								g_FingerEvent.MachineCtrl.CallBack(FINGER_ENROLL, &g_FingerEvent);
							}

							if (g_FingerEvent.MachineRun.bFinish == TRUE)
							{
								if (g_FingerEvent.MachineRun.u16RetVal == EVENT_TIME_OUT) //��ʱ�˳�
								{
#if DEBUG
									u3_printf("��ʱ�˳�����������\n");
#endif
								}

								pfingerData 		= ((FingerData_t *) g_FingerEvent.MachineCtrl.pData);
								userID				= pfingerData->u8FingerID;
								g_FingerEvent.bMutex = FALSE;
							}
							else 
							{
								break;				//�ȴ��²��ϴ��Ľ��״̬
							}

							//u8Ensure			= Finger_Enroll(SysTask.u16FingerfCnt);
							if (g_FingerEvent.MachineRun.u16RetVal == TRUE)
							{
#if DEBUG
								u3_printf("��һ���ɼ�ָ�Ƴɹ�\n");
#endif

								SysTask.Stream_State = STREAM_STATE_BLINK_THREE; //

								u8MatchCnt			= 0;
								SysTask.u32TouchWaitTime = 300; //��ʱһ��ʱ����ȥ�ɼ�
								SysTask.TouchSub	= TOUCH_SUB_AGAIN_PON; //�����ڶ���		
								FINGER_POWER_OFF(); //�ر�ָ��ģ���Դ,�ȴ�¼��ڶ���ָ��		
							}
							else 
							{

								if (u8MatchCnt++ >= MATCH_FINGER_CNT) // 50 * 3 ��Լ200MS��
								{
#if DEBUG
									u3_printf("��һ���ɼ�ָ��ʧ�ܣ��������������˳����\n");
#endif

									SysTask.Stream_State = STREAM_STATE_FAILED; //����˸��ʾ
									SysTask.u8StreamShoWCnt = BLINK_CNT_8;

									SysTask.u16FingerfCnt -= 1;
									FINGER_POWER_OFF(); //�ر�ָ��ģ���Դ

									SysTask.TouchState	= TOUCH_MATCH_INIT; //����ƥ��		
									SysTask.TouchSub	= TOUCH_SUB_INIT;
								}
								else 
								{
#if DEBUG
									u3_printf("��һ���ɼ�ָ��ʧ�� ");
									snprintf(priBuf, 10, "CNT : %d\n", u8MatchCnt);
									u3_printf(priBuf);
									u3_printf("\n");
#endif

									//ƥ��ʧ������Ҫ����һ������ʾ����
									SysTask.u32TouchWaitTime = 100; //��ʱһ��ʱ���ٴ����
								}
							}
						}

						break;

					case TOUCH_SUB_AGAIN_PON:
						if ((PS_Sta) && (SysTask.u32TouchWaitTime == 0))
						{
							FINGER_POWER_ON();		//����ָ��ģ���Դ
							SysTask.u32TouchWaitTime = POWER_ON_DELAY; //��ʱһ��ʱ����ȥ�ɼ�
							u8MatchCnt			= 0;
							SysTask.u16FingerfCnt++;
							SysTask.TouchSub	= TOUCH_SUB_SETMODE_TWO;
							u8SetCnt			= 0;
						}

						break;

					case TOUCH_SUB_SETMODE_TWO: // *
						if (SysTask.u32TouchWaitTime == 0)
						{

							if (g_FingerEvent.bMutex == FALSE)
							{
								g_FingerEvent.bMutex = TRUE; //������ֻ����һ��
								g_FingerEvent.MachineCtrl.u32Timeout = FINGER_REV_TIME_OUT; //1S ��ʱ  ,5�ξ���6S
								g_FingerEvent.MachineRun.u32StartTime = SysTask.u32SysTimes;
								g_FingerEvent.MachineRun.bFinish = FALSE;
								g_FingerEvent.MachineCtrl.CallBack(FINGER_MODE_SET, &g_FingerEvent);
							}
							else 
							{
								g_FingerEvent.MachineRun.u32LastTime = SysTask.u32SysTimes;
								g_FingerEvent.MachineCtrl.CallBack(FINGER_MODE_SET, &g_FingerEvent);
							}

							if (g_FingerEvent.MachineRun.bFinish == TRUE)
							{
								if (g_FingerEvent.MachineRun.u16RetVal == EVENT_TIME_OUT) //��ʱ�˳�
								{
#if DEBUG
									u3_printf("��ʱ�˳�����������\n");
#endif
								}

								g_FingerEvent.bMutex = FALSE;
							}
							else 
							{
								break;				//�ȴ��²��ϴ��Ľ��״̬
							}

							u8SetCnt++;

							if (!g_FingerEvent.MachineRun.u16RetVal)
							{
#if DEBUG
								u3_printf("2. �����ظ�ģʽ�ɹ�\n");
								snprintf(priBuf, 10, "CNT : %d\n", u8SetCnt);
								u3_printf(priBuf);
								u3_printf("\n");
#endif

								SysTask.TouchSub	= TOUCH_SUB_AGAIN;
							}
							else 
							{
								if (u8SetCnt <= 8)
									SysTask.u32TouchWaitTime = 50;
								else 
									SysTask.TouchSub = TOUCH_SUB_AGAIN;
							}
						}

						break;

					case TOUCH_SUB_AGAIN:
						if ((SysTask.u32TouchWaitTime == 0)) //
						{
							if (g_FingerEvent.bMutex == FALSE)
							{
								g_FingerEvent.bMutex = TRUE; //������ֻ����һ��
								g_FingerEvent.MachineCtrl.u32Timeout = FINGER_REV_TIME_OUT * 5; //1S ��ʱ  ,5�ξ���6S
								g_FingerEvent.MachineRun.u32StartTime = SysTask.u32SysTimes;
								g_FingerEvent.MachineRun.bFinish = FALSE;
								pfingerData 		= ((FingerData_t *) g_FingerEvent.MachineCtrl.pData);
								pfingerData->u8FingerID = SysTask.u16FingerfCnt;
								g_FingerEvent.MachineCtrl.CallBack(FINGER_ENROLL, &g_FingerEvent);
							}
							else 
							{
								pfingerData 		= ((FingerData_t *) g_FingerEvent.MachineCtrl.pData);
								pfingerData->u8FingerID = SysTask.u16FingerfCnt;
								g_FingerEvent.MachineRun.u32LastTime = SysTask.u32SysTimes;
								g_FingerEvent.MachineCtrl.CallBack(FINGER_ENROLL, &g_FingerEvent);
							}

							if (g_FingerEvent.MachineRun.bFinish == TRUE)
							{
								if (g_FingerEvent.MachineRun.u16RetVal == EVENT_TIME_OUT) //��ʱ�˳�
								{
#if DEBUG
									u3_printf("��ʱ�˳�����������\n");
#endif
								}

								pfingerData 		= ((FingerData_t *) g_FingerEvent.MachineCtrl.pData);
								userID				= pfingerData->u8FingerID;
								g_FingerEvent.bMutex = FALSE;
							}
							else 
							{
								break;				//�ȴ��²��ϴ��Ľ��״̬
							}

							//							u8Ensure			= Finger_Enroll(SysTask.u16FingerfCnt); //��������һ��ID�У�һ��ָ��һ��������������ע�⣬�����ָ�Ʋ�һ����ͬһ��
							if (g_FingerEvent.MachineRun.u16RetVal == TRUE)
							{
#if DEBUG
								u3_printf("�ڶ����ɼ�ָ�Ƴɹ�\n");
#endif

								SysTask.Stream_State = STREAM_STATE_SUCCESS; //

								SysTask.u16SaveTick = 1; //1��󱣴�
								SysTask.TouchSub	= TOUCH_SUB_WAIT;
								SysTask.u32TouchWaitTime = 300; //��ʱһ��ʱ���˳�
							}
							else 
							{
								//�ɼ�ָ��ʧ��
								if (u8MatchCnt++ >= MATCH_FINGER_CNT) // 50 * 3 ��Լ200MS��
								{
#if DEBUG
									u3_printf("�ɼ�ָ��ʧ�ܳ�������\n");
									u3_printf("ɾ����һ�α���ģ��˳����\n");
#endif

									SysTask.Stream_State = STREAM_STATE_FAILED; //����˸��ʾ
									SysTask.u8StreamShoWCnt = BLINK_CNT_8;

									Finger_Delete(SysTask.u16FingerfCnt - 1); //���ʧ�ܣ�ɾ����һ��¼���
									SysTask.u16FingerfCnt -= 2;
									FINGER_POWER_OFF(); //�ر�ָ��ģ���Դ

									SysTask.TouchState	= TOUCH_MATCH_INIT; //����ƥ��		
									SysTask.TouchSub	= TOUCH_SUB_INIT;
								}
								else 
								{
#if DEBUG
									u3_printf("�ɼ�ָ��ʧ�ܣ����²ɼ�\n");
#endif

									//ƥ��ʧ������Ҫ����һ������ʾ����
									//SysTask.TouchSub	= TOUCH_SUB_ENTER;	 //�ظ�ģʽ���ﲻ���ٴθ�
									SysTask.u32TouchWaitTime = 60; //��ʱһ��ʱ���ٴ����
								}
							}
						}

						break;

					case TOUCH_SUB_WAIT: // *
						if ((SysTask.u32TouchWaitTime == 0))
						{
							FINGER_POWER_OFF(); 	//�ر�ָ��ģ���Դ

							SysTask.TouchState	= TOUCH_MATCH_INIT; //����ƥ��		
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
						if (SysTask.u32TouchWaitTime == 0)
						{
							FINGER_POWER_ON();		//����ָ��ģ���Դ
							u8MatchCnt			= 0;
							SysTask.TouchSub	= TOUCH_SUB_ENTER;
							SysTask.u32TouchWaitTime = POWER_ON_DELAY; //
						}

						break;

					case TOUCH_SUB_ENTER: //
						if (SysTask.u32TouchWaitTime)
						{
							break;
						}

						if (g_FingerEvent.bMutex == FALSE)
						{
							g_FingerEvent.bMutex = TRUE; //������ֻ����һ��
							g_FingerEvent.MachineCtrl.u32Timeout = FINGER_REV_TIME_OUT * 5; //1S ��ʱ  ,5�ξ���6S
							g_FingerEvent.MachineRun.u32StartTime = SysTask.u32SysTimes;
							g_FingerEvent.MachineRun.bFinish = FALSE;
							pfingerData 		= ((FingerData_t *) g_FingerEvent.MachineCtrl.pData);
							pfingerData->u8FingerID = SysTask.u16FingerfCnt;
							g_FingerEvent.MachineCtrl.CallBack(FINGER_CLEAR, &g_FingerEvent);
						}
						else 
						{
							pfingerData 		= ((FingerData_t *) g_FingerEvent.MachineCtrl.pData);
							pfingerData->u8FingerID = SysTask.u16FingerfCnt;
							g_FingerEvent.MachineRun.u32LastTime = SysTask.u32SysTimes;
							g_FingerEvent.MachineCtrl.CallBack(FINGER_CLEAR, &g_FingerEvent);
						}

						if (g_FingerEvent.MachineRun.bFinish == TRUE)
						{
							if (g_FingerEvent.MachineRun.u16RetVal == EVENT_TIME_OUT) //��ʱ�˳�
							{
#if DEBUG
								u3_printf("��ʱ�˳�����������\n");
#endif
							}

							pfingerData 		= ((FingerData_t *) g_FingerEvent.MachineCtrl.pData);
							userID				= pfingerData->u8FingerID;
							g_FingerEvent.bMutex = FALSE;
						}
						else 
						{
							break;					//�ȴ��²��ϴ��Ľ��״̬
						}

						//u8Ensure = Finger_Clear(); //ɾ������ָ�ƿ�
						if (g_FingerEvent.MachineRun.u16RetVal == TRUE)
						{
#if DEBUG
							u3_printf("ɾ������ָ�ƿ⣬�ɹ�\n");
#endif

							SysTask.Stream_State = STREAM_STATE_SUCCESS; //

							SysTask.u16FingerfCnt = 0;
							SysTask.u16SaveTick = 1; //1��󱣴�
							SysTask.TouchSub	= TOUCH_SUB_WAIT;
							SysTask.u32TouchWaitTime = 100; //��ʱһ��ʱ���˳�
						}
						else //ɾ��ʧ�� ,
						{
							u8MatchCnt++;
							SysTask.u32TouchWaitTime = 100; //��ʱһ��ʱ���ٴ�ɾ��

							if (u8MatchCnt >= 3) //ʧ���˳�
							{
#if DEBUG
								u3_printf("ɾ������ָ�ƿ⣬ ʧ��\n");
#endif

								SysTask.Stream_State = STREAM_STATE_FAILED; //����˸��ʾ
								SysTask.u8StreamShoWCnt = BLINK_CNT_8;

								SysTask.TouchSub	= TOUCH_SUB_WAIT;
							}
						}

						break;

					case TOUCH_SUB_WAIT: // *
						if ((SysTask.u32TouchWaitTime == 0))
						{
							FINGER_POWER_OFF(); 	//�ر�ָ��ģ���Դ
							SysTask.TouchState	= TOUCH_MATCH_INIT; //����ƥ��		
							SysTask.TouchSub	= TOUCH_SUB_INIT;
						}

						break;

					default:
						break;
				}
			}
			break;

		case TOUCH_KEY_SHORT: //�̰�
			//�̰������ָ��
			//���ָ��ģ�����滹û�б����û����򱣴�ΪROOT�û�
			//����Ѿ�������ָ�ƣ�����ROOT�û���֤�ɹ�֮�󣬲ſ�������û���ROOT�û�Ϊ��һ����ӵ��û���
			switch (SysTask.TouchSub)
			{
				case TOUCH_SUB_INIT: // 
					if (SysTask.u16FingerfCnt == 0) //��ǰ��û�б������û���ֱ�����
					{
#if DEBUG
						u3_printf("TOUCH_KEY_SHORT û��ROOT�û���ֱ�����ΪROOT\n");
#endif
						SysTask.u32TouchWaitTime = 6000; //��ʱ�˳�

						SysTask.TouchState	= TOUCH_ADD_USER;
					}
					else 
					{
#if DEBUG
						snprintf(priBuf, 50, "SysTask.u16FingerfCnt : %d\n", SysTask.u16FingerfCnt);
						u3_printf(priBuf);
						u3_printf("�Ѿ���ROOT��ȷ��ROOT\n");
#endif

						SysTask.Stream_State = STREAM_STATE_BLINK_ONE; //

						SysTask.TouchSub	= TOUCH_SUB_CHECK_ROOT;
						SysTask.u32TouchWaitTime = 6000; //��ʱ�˳�
						u8MatchCnt			= 0;
					}

					break;

				case TOUCH_SUB_CHECK_ROOT: // ȷ��ROOT�û� 
					if (PS_Sta) //��ָ�ư���
					{
#if DEBUG
						u3_printf("TOUCH_SUB_AGAIN �а���\n");
#endif

						SysTask.u32TouchWaitTime = SysTask.nTick + POWER_ON_DELAY; //һ��ʱ���ټ�⣬ ��ָ��ģ��MCU�ϵ���ȫ ,ʵ��1020A��Ҫ200ms	1020c 100ms
						FINGER_POWER_ON();			//����ָ��ģ���Դ
						SysTask.TouchSub	= TOUCH_SUB_AGAIN;
					}
					else if (SysTask.u32TouchWaitTime == 0)
					{
						//��ʱ�˳�
#if DEBUG
						u3_printf("û��ʶ��ROOT ��ʱ�˳� \n");
#endif
						SysTask.TouchState	= TOUCH_MATCH_INIT; //����ƥ��
						SysTask.TouchSub	= TOUCH_SUB_INIT;
						SysTask.Stream_State = STREAM_STATE_FAILED; //����˸��ʾ
						SysTask.u8StreamShoWCnt = BLINK_CNT_6;
					}

					break;

				case TOUCH_SUB_AGAIN:
					if (SysTask.u32TouchWaitTime == 0) //��ָ�ư���
					{
						if (g_FingerEvent.bMutex == FALSE)
						{
							g_FingerEvent.bMutex = TRUE; //������ֻ����һ��
							g_FingerEvent.MachineCtrl.u32Timeout = FINGER_REV_TIME_OUT; //1S ��ʱ  ,5�ξ���6S
							g_FingerEvent.MachineRun.u32StartTime = SysTask.u32SysTimes;
							g_FingerEvent.MachineRun.bFinish = FALSE;
							g_FingerEvent.MachineCtrl.CallBack(FINGER_SEARCHING, &g_FingerEvent);
						}
						else 
						{
							g_FingerEvent.MachineRun.u32LastTime = SysTask.u32SysTimes;
							g_FingerEvent.MachineCtrl.CallBack(FINGER_SEARCHING, &g_FingerEvent);
						}

						if (g_FingerEvent.MachineRun.bFinish == TRUE)
						{
							if (g_FingerEvent.MachineRun.u16RetVal == EVENT_TIME_OUT) //��ʱ�˳�
							{
#if DEBUG
								u3_printf("��ʱ�˳�����������\n");
#endif
							}

							pfingerData 		= ((FingerData_t *) g_FingerEvent.MachineCtrl.pData);
							userID				= pfingerData->u8FingerID;
							g_FingerEvent.bMutex = FALSE;
						}
						else 
						{
							break;
						}

						//		u8Ensure			= Finger_Search(&userID); //1��N����ָ�ƿ�
						if (g_FingerEvent.MachineRun.u16RetVal == TRUE) //ƥ��ɹ�
						{

							pfingerData 		= ((FingerData_t *) g_FingerEvent.MachineCtrl.pData);
							userID				= pfingerData->u8FingerID;

							if ((userID != ROOT_ID_NUM_1) && (userID != ROOT_ID_NUM_2))
							{ //��ROOT�û�

#if DEBUG
								u3_printf("TOUCH_MATCH_AGAIN ƥ��ʧ��	��ROOT�û�");
								snprintf(priBuf, 50, "userID : %d\n", userID);
								u3_printf(priBuf);
								u3_printf("\n");
#endif

								SysTask.u32TouchWaitTime = SysTask.nTick + 100; //��ʱһ��ʱ�� �ٴ�����

								//ƥ��ʧ��
								if (u8MatchCnt++ >= MATCH_FINGER_CNT) // 50 * 3 ��Լ200MS��
								{

#if DEBUG
									u3_printf("TOUCH_MATCH_AGAIN ƥ��ʧ�ܳ��Σ��˳� \n");
#endif

									SysTask.Stream_State = STREAM_STATE_FAILED; //����˸��ʾ
									SysTask.u8StreamShoWCnt = BLINK_CNT_8;

									FINGER_POWER_OFF(); //�ر�ָ��ģ���Դ
									SysTask.TouchState	= TOUCH_MATCH_INIT; //����ƥ��
									SysTask.u32TouchWaitTime = SysTask.nTick + 500; //��ʱһ��ʱ�� 
								}
							}
							else 
							{
#if DEBUG
								u3_printf("ȷ��ΪROOT�����ָ��\n");
#endif

								FINGER_POWER_OFF(); //�ȹر�ָ��ģ���Դ����Ȼʶ�𲻵��� touch��ƽ�仯 
                                
                                SysTask.Stream_State = STREAM_STATE_BLINK_TWO; //����˸��ʾ

								SysTask.TouchState	= TOUCH_ADD_USER;
								SysTask.TouchSub	= TOUCH_SUB_INIT;
								SysTask.u32TouchWaitTime = 0;
							}
						}
						else 
						{
#if DEBUG
							u3_printf("TOUCH_MATCH_AGAIN ƥ��ʧ��\n");
#endif

							SysTask.u32TouchWaitTime = SysTask.nTick + 100; //��ʱһ��ʱ�� ����ֳ�ʶ���ˣ�������һ���˼�⣬

							//ƥ��ʧ��
							if (u8MatchCnt++ >= MATCH_FINGER_CNT) // 50 * 3 ��Լ200MS��
							{
#if DEBUG
								u3_printf("TOUCH_MATCH_AGAIN ƥ��ʧ�ܳ��Σ��˳� \n");
#endif
                                
                                SysTask.Stream_State = STREAM_STATE_FAILED; //����˸��ʾ
                                SysTask.u8StreamShoWCnt = BLINK_CNT_8;

								FINGER_POWER_OFF(); //�ر�ָ��ģ���Դ
								SysTask.TouchState	= TOUCH_MATCH_INIT; //����ƥ��
								SysTask.TouchSub	= TOUCH_SUB_INIT;
								SysTask.u32TouchWaitTime = SysTask.nTick + 500; //��ʱһ��ʱ�� ����ֳ�ʶ���ˣ�������һ���˼�⣬
							}
						}
					}

					break;

				default:
					break;
			}

			break;

		case TOUCH_CHECK: //��ȡID����EEPROM�Ƿ�һ��, ����ָ�Ƶ�¼��ģʽΪ���ظ�
			if (SysTask.u32TouchWaitTime)
				return;

			switch (SysTask.TouchSub)
			{
				case TOUCH_SUB_INIT: // *
					//					if (SysTask.u32TouchWaitTime == 0)
					//					{
					//						u8SetCnt++;
					//						if (!Finger_ModeSet())
					//						{
					//#if DEBUG
					//							u3_printf("TOUCH_CHECK �ɹ�");
					//#endif
					//							SysTask.TouchSub	= TOUCH_SUB_ENTER;
					//						}
					//						else 
					//						{
					//							SysTask.u32TouchWaitTime = 50;
					//							SysTask.TouchSub	= TOUCH_SUB_WAIT; //ʧ���������ý�ȥ
					//						}
					//					}
					//ÿһ�����ǰ�������ÿ��ظ����ģʽ
					SysTask.u32TouchWaitTime = 5;
					SysTask.TouchSub = TOUCH_SUB_ENTER;
					break;

				case TOUCH_SUB_ENTER: //
					if (SysTask.u32TouchWaitTime)
					{
						break;
					}

					if (g_FingerEvent.bMutex == FALSE)
					{
						g_FingerEvent.bMutex = TRUE; //������ֻ����һ��
						g_FingerEvent.MachineCtrl.u32Timeout = FINGER_REV_TIME_OUT; //1S ��ʱ  ,5�ξ���6S
						g_FingerEvent.MachineRun.u32StartTime = SysTask.u32SysTimes;
						g_FingerEvent.MachineRun.bFinish = FALSE;
						g_FingerEvent.MachineCtrl.CallBack(FINGER_READ, &g_FingerEvent);
					}
					else 
					{
						g_FingerEvent.MachineRun.u32LastTime = SysTask.u32SysTimes;
						g_FingerEvent.MachineCtrl.CallBack(FINGER_READ, &g_FingerEvent);
					}

					if (g_FingerEvent.MachineRun.bFinish == TRUE)
					{
						if (g_FingerEvent.MachineRun.u16RetVal == EVENT_TIME_OUT) //��ʱ�˳�
						{
#if DEBUG
							u3_printf("��ʱ�˳�����������\n");
#endif
						}

						g_FingerEvent.bMutex = FALSE;
					}
					else 
					{
						break;						//�ȴ��²��ϴ��Ľ��״̬
					}

					//retVal = Finger_Read();
					pfingerData = ((FingerData_t *) g_FingerEvent.MachineCtrl.pData);
					retVal = pfingerData->u8FingerID;

					if (retVal != -1)
					{
#if DEBUG
						snprintf(priBuf, 50, "SysTask.u16FingerfCnt : %d	 read cnt:	%d\n", SysTask.u16FingerfCnt, retVal);
						u3_printf(priBuf);
						u3_printf("\n");
#endif

						SysTask.TouchSub	= TOUCH_SUB_EXIT;

						if (((unsigned int) retVal) != SysTask.u16FingerfCnt)
						{
#if DEBUG
							u3_printf("��������ȡ�Ĳ�ƥ��\n");
#endif

							SysTask.Stream_State = STREAM_STATE_FAILED; //����˸��ʾ
							SysTask.u8StreamShoWCnt = BLINK_CNT_8;

							if ((retVal <= FINGER_ID_MAX) && (retVal & 0x01 == 0)) //��Ϊ1020A ����һ���û�ռ������ID
							{

#if DEBUG
								u3_printf("������ֵ ");
								snprintf(priBuf, 50, "SysTask.u16FingerfCnt : %d\n", SysTask.u16FingerfCnt);
								u3_printf(priBuf);
								u3_printf("\n");
#endif

								SysTask.u16FingerfCnt = retVal;
								SysTask.u16SaveTick = 1; //1��󱣴�
							}
							else 
							{
#if DEBUG
								u3_printf("ȫ�����\n");
#endif

								//TODO	Ҫ��Ҫɾ�������ָ����Ϣ
								SysTask.TouchState	= TOUCH_DEL_USER;
								SysTask.TouchSub	= TOUCH_SUB_INIT;

								SysTask.u16FingerfCnt = 0;
								SysTask.u16SaveTick = 1; //1��󱣴�
							}
						}
					}

					break;

				case TOUCH_SUB_WAIT: // *
					if ((SysTask.u32TouchWaitTime == 0))
					{
						if (u8SetCnt >= 20) //����20��ʧ�ܣ��򷵻�
							SysTask.TouchSub = TOUCH_SUB_ENTER;
						else 
							SysTask.TouchSub = TOUCH_SUB_INIT;
					}

					break;

				case TOUCH_SUB_EXIT: // *
					if ((SysTask.u32TouchWaitTime == 0))
					{
						FINGER_POWER_OFF(); 		//�ر�ָ��ģ���Դ
						SysTask.TouchState	= TOUCH_MATCH_INIT; //����ƥ��	 
						SysTask.TouchSub	= TOUCH_SUB_INIT;
						u8SetCnt			= 0;
					}

					break;

				default:
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
void LedTask(void)
{
	//	  if (SysTask.LockMode[GLASS_01] != SysTask.LockModeSave[GLASS_01])
	//	  {
	//		  if (SysTask.LockMode[GLASS_01] == LOCK_ON) //��Ҫ����
	//		  {
	//			  GPIO_SetBits(LED_GPIO, LED_PIN_SINGLE);
	//		  }
	//		  else //��Ҫ����
	//		  {
	//			  GPIO_ResetBits(LED_GPIO, LED_PIN_SINGLE);
	//		  }
	//		  
	//		SysTask.LockModeSave[GLASS_01] = SysTask.LockMode[GLASS_01];
	//	  }
}


/*******************************************************************************
* ����: 
* ����: ��ˮ��
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void LedStreamTask(void)
{
	static u8 u8LedNum	= 0;

	static u8 u8Blink	= 0;

	//��ˮ��Ч��
	switch (SysTask.Stream_State)
	{
		case STREAM_STATE_INIT:
			if (SysTask.u32StreamTime != 0)
				break;

			ws2812bClearAll();
			u8LedNum = 0;
			u8Blink = 0;

			if (SysTask.LockMode[GLASS_01] == LOCK_ON) //�����ڿ���״̬
			{
				SysTask.Stream_State = STREAM_STATE_OPEN_LOCK;
			}
			else //����
			{
				SysTask.Stream_State = STREAM_STATE_STREAM;
			}

			break;

		case STREAM_STATE_STREAM: //��ˮ
			if (SysTask.u32StreamTime != 0)
				break;

			SysTask.u32StreamTime = STREAM_TIME;

			if (u8LedNum > WS2812B_NUM)
			{
				ws2812bClearAll();
				u8LedNum			= 0;
				break;
			}
			else 
			{
				ws2812bSet(u8LedNum, LED_BRIGHNESS); //0X60 ���ȣ��ɵ�
			}

			u8LedNum++;
			break;

		case STREAM_STATE_OPEN_LOCK:
			if (SysTask.u32StreamTime != 0)
				break;

			ws2812bSetAll(LED_BRIGHNESS); //����ʱȫ����������ȥ�ر�
			SysTask.u32StreamTime = (vu32) - 1;
			break;

		case STREAM_STATE_BLINK_ONE: //��˸��һ����
			if (SysTask.u32StreamTime != 0)
				break;

			SysTask.u32StreamTime = BLINK_TIME;

			if (u8Blink)
			{
				ws2812bSet(0, LED_BRIGHNESS);		//0X60 ���ȣ��ɵ�
				u8Blink 			= 0;
			}
			else 
			{
				u8Blink 			= 1;
				ws2812bClearAll();
			}

			break;

		case STREAM_STATE_BLINK_TWO: //��˸�ڶ�����
			if (SysTask.u32StreamTime != 0)
				break;

			SysTask.u32StreamTime = BLINK_TIME;

			if (u8Blink)
			{
				ws2812bClearAll();
				ws2812bSet(0, LED_BRIGHNESS);		//0X60 ���ȣ��ɵ� 
				ws2812bSet(1, LED_BRIGHNESS);		//0X60 ���ȣ��ɵ� 
				u8Blink 			= 0;
			}
			else 
			{
				u8Blink 			= 1;
				ws2812bClear(1);
			}

			break;

		case STREAM_STATE_BLINK_THREE: //��˸��������
			if (SysTask.u32StreamTime != 0)
				break;

			SysTask.u32StreamTime = BLINK_TIME;

			if (u8Blink)
			{
				ws2812bClearAll();
				ws2812bSet(0, LED_BRIGHNESS);		//0X60 ���ȣ��ɵ� 
				ws2812bSet(1, LED_BRIGHNESS);		//0X60 ���ȣ��ɵ� 
				ws2812bSet(2, LED_BRIGHNESS);		//0X60 ���ȣ��ɵ� 
				u8Blink 			= 0;
			}
			else 
			{
				u8Blink 			= 1;
				ws2812bClear(2);
			}

			break;

		case STREAM_STATE_SUCCESS: //�ɹ�ƥ��
			if (SysTask.u32StreamTime != 0)
				break;

			SysTask.u32StreamTime = 2000; //��2S ��ʾ�ɹ����ٽ�����ˮ
			SysTask.Stream_State = STREAM_STATE_INIT;
			ws2812bSetAll(LED_BRIGHNESS);
			break;

		case STREAM_STATE_FAILED: //ʧ��
			if (SysTask.u32StreamTime != 0)
				break;

			if (SysTask.u8StreamShoWCnt == 0) //��˸��ɣ�������������ˮ״̬
			{
				u8LedNum			= 0;
				u8Blink 			= 0;
				SysTask.Stream_State = STREAM_STATE_INIT;
				SysTask.u32StreamTime = 500;
				break;
			}

			SysTask.u32StreamTime = FAILED_TIME;

			if (SysTask.u8StreamShoWCnt & 0x01)
			{
				ws2812bClearAll();
			}
		else 
			{
				ws2812bSetAll(LED_BRIGHNESS);
			}

			SysTask.u8StreamShoWCnt--;
			break;

		default:
			break;
	}
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
	static u8 u8StateSave = PWM_STATE_INIT;

	if (SysTask.PWM_State[GLASS_01] == u8StateSave)
		return;

	//A���
	switch (SysTask.PWM_State[GLASS_01])
	{
		case PWM_STATE_INIT:
			break;

		case PWM_STATE_FEW_P:
			TIM_SetCompare3(TIM3, PWM_POSITIVE); //�޸�PWMռ�ձ� ,��ת
			TIM_SetCompare4(TIM3, 0); //

			//TIM3->CCR4=0;
			TIM_CtrlPWMOutputs(TIM3, ENABLE); // ʹ�����PWM
			break;

		case PWM_STATE_REV_P:
			TIM_SetCompare4(TIM3, PWM_POSITIVE); //�޸�PWMռ�ձ�, ��ת
			TIM_SetCompare3(TIM3, 0); //

			//TIM3->CCR3=0;
			TIM_CtrlPWMOutputs(TIM3, ENABLE); // ʹ�����PWM
			break;

		case PWM_STATE_OPEN: //����
			switch (SysTask.PWM_SubState[GLASS_01])
			{
				case PWM_SUB_FEW: //��ת
#if DEBUG
					u3_printf(" PWM ȫ����ת����\n");
#endif

					TIM_SetCompare3(TIM3, PWM_PERIOD); //�޸�PWMռ�ձ� ,��ת
					TIM_SetCompare4(TIM3, 0);
					break;

				case PWM_SUB_REV: //��ת
#if DEBUG
					u3_printf(" PWM ȫ�ٷ�ת����\n");
#endif

					TIM_SetCompare4(TIM3, PWM_PERIOD); //�޸�PWMռ�ձ�, ��ת
					TIM_SetCompare3(TIM3, 0);
					break;

				default:
					break;
			}

			TIM_CtrlPWMOutputs(TIM3, ENABLE); // ʹ�����PWM
			break;

		case PWM_STATE_STOP:
			TIM_SetCompare4(TIM3, 0); //
			TIM_SetCompare3(TIM3, 0); //
			TIM_CtrlPWMOutputs(TIM3, DISABLE); // ֹͣ���PWM��ͣ��
		default:
			break;
	}

	u8StateSave 		= SysTask.PWM_State[GLASS_01];
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
			if (SysTask.Moto_Mode[GLASS_01] == MOTO_FR_FWD_REV)
			{
				SysTask.Moto_Time[GLASS_01] = MOTO_TIME_ANGLE; //תһ���Ƕ�
				SysTask.Moto_RunTime[GLASS_01] =
					 SysTask.nTick + g_au16MoteTime[SysTask.Moto_Time[GLASS_01]][1] +MOTO_DEVIATION_TIME; //��ϵͳƫ��
				SysTask.Moto_State[GLASS_01] = MOTO_STATE_FIND_ZERO;
				SysTask.Moto_ModeSave[GLASS_01] = SysTask.Moto_Mode[GLASS_01];


#if DEBUG_MOTO
				u3_printf("MOTO_STATE_INIT");
				u3_printf("��������ת ȥ��ԭ�㡣��\n");
#endif

				SysTask.Moto_SubState[GLASS_01] = MOTO_SUB_STATE_UP;

				//GPIO_SetBits(MOTO_P_GPIO, MOTO_01_PIN_P); //����ת
				//GPIO_ResetBits(MOTO_N_GPIO, MOTO_01_PIN_N); //
				SysTask.PWM_State[GLASS_01] = PWM_STATE_FEW_P; //����ת
			}
			else 
			{
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
					if (GPIO_ReadInputDataBit(LOCK_ZERO_GPIO, LOCK_ZERO_01_PIN) == 0) // �ҵ�ԭ�㣬
					{
#if DEBUG_MOTO
						u3_printf("MOTO_STATE_FIND_ZERO ");
						u3_printf("��ת �ҵ�ԭ�㣬");
						u3_printf("\n");
#endif

						SysTask.Moto_State[GLASS_01] = MOTO_STATE_RUN_CHA;
						SysTask.Moto_SubState[GLASS_01] = MOTO_SUB_STATE_UP_F; //������ת forward
						SysTask.Moto_RunTime[GLASS_01] =
							 SysTask.nTick + g_au16MoteTime[SysTask.Moto_Time[GLASS_01]][1];
					}
					else if (SysTask.Moto_RunTime[GLASS_01] == 0) //ת��һ���ĽǶ�û���ҵ�ԭ�㣬����ȥ��ԭ��
					{
#if DEBUG_MOTO
						u3_printf("MOTO_STATE_FIND_ZER");
						u3_printf("��ת û�е�ԭ��");
						u3_printf("\n");
#endif

						SysTask.Moto_SubState[GLASS_01] = MOTO_SUB_STATE_DOWN;

						//���ٽ���ת�յ�Ϊ��ʼ������ǰ���Ѿ���תһ���Ƕȣ�����Ҫ�Ⱦ����ص�֮ǰλ���ٷ�ת������ʱ��Ҫ���� 2
						//Ĭ����һ�ιػ���û�м�⵽���ʱ�����MOTO_DEVIATION_TIME
						SysTask.Moto_RunTime[GLASS_01] =
							 SysTask.nTick + (g_au16MoteTime[SysTask.Moto_Time[GLASS_01]][1] +MOTO_DEVIATION_TIME + MOTO_DEVIATION_TIME) * 2 + MOTO_DEVIATION_TIME;

						//GPIO_SetBits(MOTO_N_GPIO, MOTO_01_PIN_N); //��ת
						//GPIO_ResetBits(MOTO_P_GPIO, MOTO_01_PIN_P); //
						SysTask.PWM_State[GLASS_01] = PWM_STATE_REV_P;
					}

					break;

				case MOTO_SUB_STATE_DOWN: //�½��� ��ת
					if (GPIO_ReadInputDataBit(LOCK_ZERO_GPIO, LOCK_ZERO_01_PIN) == 0) // �ҵ�ԭ�㣬
					{
#if DEBUG_MOTO
						u3_printf("MOTO_STATE_FIND_ZERO");
						u3_printf("��ת �ҵ�ԭ�㣬\n");
#endif

						SysTask.Moto_State[GLASS_01] = MOTO_STATE_RUN_CHA;
						SysTask.Moto_SubState[GLASS_01] = MOTO_SUB_STATE_DOWN_F; //������ת forward
						SysTask.Moto_RunTime[GLASS_01] =
							 SysTask.nTick + g_au16MoteTime[SysTask.Moto_Time[GLASS_01]][1];
					}
					else if (SysTask.Moto_RunTime[GLASS_01] == 0) //ת��һ���ĽǶ�û���ҵ�ԭ�㣬���ʧЧ���Ե�ǰ�Ƕ�Ϊ��ת�յ�
					{
#if DEBUG_MOTO
						u3_printf("MOTO_STATE_FIND_ZERO");
						u3_printf("��תû�е�ԭ��\n");
#endif

						SysTask.Moto_State[GLASS_01] = MOTO_STATE_CHANGE_DIR; //��ͷ��ʼ��ת

						g_bSensorState		= FALSE;

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

#if DEBUG_MOTO
			u3_printf("\n---------------��������״̬-----------\n");
#endif

			break;

		case MOTO_STATE_RUN_CHA: //����ת����״̬�� ֻ������״̬ �������ģʽ����
			switch (SysTask.Moto_SubState[GLASS_01])
			{
				case MOTO_SUB_STATE_UP: //��������ת
					if (SysTask.Lock_State[GLASS_01] == LOCK_STATE_DEBOUNSE) //׼����������ת��ȥ��ԭ�� ����. �ⲿ��Ҫ��moto����ģ���������PWM�����
					{
						SysTask.PWM_State[GLASS_01] = PWM_STATE_OPEN;
						SysTask.PWM_SubState[GLASS_01] = PWM_SUB_FEW;
						break;
					}

					if (SysTask.Moto_RunTime[GLASS_01] == 0) // ��ʱ����� ��˵���Ѿ�ת�˹̶��Ƕ�
					{
#if DEBUG_MOTO
						u3_printf("MOTO_SUB_STATE_UP");
						u3_printf("��ת\n");
#endif

						SysTask.Moto_SubState[GLASS_01] = MOTO_SUB_STATE_ZERO_R;

						if (g_bSensorState == TRUE)
							SysTask.Moto_RunTime[GLASS_01] =
								 SysTask.nTick + g_au16MoteTime[SysTask.Moto_Time[GLASS_01]][1] +MOTO_DEVIATION_TIME; //��һ����ȥ����㣬����Ҫ����һ��ϵͳƫ���ʱ��
						else 
							SysTask.Moto_RunTime[GLASS_01] =
								 SysTask.nTick + g_au16MoteTime[SysTask.Moto_Time[GLASS_01]][1]; //û�и�Ӧ�������������ʱ��Ļ�׼�����ü���ƫ��

						SysTask.PWM_State[GLASS_01] = PWM_STATE_FEW_P; //����ת
					}

					break;

				case MOTO_SUB_STATE_ZERO_R: //�����
					if (SysTask.Lock_State[GLASS_01] == LOCK_STATE_DEBOUNSE) //׼����������ת��ȥ��ԭ�� ����. �ⲿ��Ҫ��moto����ģ���������PWM�����
					{
						SysTask.PWM_State[GLASS_01] = PWM_STATE_OPEN;
						SysTask.PWM_SubState[GLASS_01] = PWM_SUB_FEW;
					}

					if (GPIO_ReadInputDataBit(LOCK_ZERO_GPIO, LOCK_ZERO_01_PIN) == 0) // �ҵ�ԭ�㣬
					{
#if DEBUG_MOTO
						u3_printf(" MOTO_SUB_STATE_ZERO_R");
						u3_printf("��ת �ҵ�ԭ�㣬\n");
#endif

						if (SysTask.Lock_State[GLASS_01] == LOCK_STATE_DEBOUNSE) //׼�������������κδ���
							break;

						if (SysTask.Moto_Mode[GLASS_01] != SysTask.Moto_ModeSave[GLASS_01]) //״̬�ı䣬��ת������ֹͣ״̬ʱ�����������֧ 
						{
#if DEBUG_MOTO
							u3_printf(" MOTO_SUB_STATE_ZERO_R");
							u3_printf("״̬�ı�\n");
#endif

							SysTask.PWM_State[GLASS_01] = PWM_STATE_STOP;

							SysTask.Moto_State[GLASS_01] = MOTO_STATE_INIT; //����ʱһ��ʱ���ٷ�ת
							break;
						}

						SysTask.Moto_SubState[GLASS_01] = MOTO_SUB_STATE_UP_F;
						SysTask.Moto_RunTime[GLASS_01] =
							 SysTask.nTick + g_au16MoteTime[SysTask.Moto_Time[GLASS_01]][1];
					}
					else if (SysTask.Moto_RunTime[GLASS_01] == 0) //ת��һ���ĽǶ�û���ҵ�ԭ��
					{
#if DEBUG_MOTO
						u3_printf(" MOTO_SUB_STATE_ZERO_R");
						u3_printf("��ת	û���ҵ�ԭ�㣬\n");
#endif

						SysTask.Moto_SubState[GLASS_01] = MOTO_SUB_STATE_UP_F;
						SysTask.Moto_RunTime[GLASS_01] =
							 SysTask.nTick + g_au16MoteTime[SysTask.Moto_Time[GLASS_01]][1];
					}

					break;

				case MOTO_SUB_STATE_UP_F: //������ת forward
					if (SysTask.Lock_State[GLASS_01] == LOCK_STATE_DEBOUNSE) //׼����������ת��ȥ��ԭ�� ����. �ⲿ��Ҫ��moto����ģ���������PWM�����
					{
						SysTask.PWM_State[GLASS_01] = PWM_STATE_OPEN;
						SysTask.PWM_SubState[GLASS_01] = PWM_SUB_REV;
						break;
					}

					if (SysTask.Moto_RunTime[GLASS_01] == 0) // ��ʱ����� ��˵���Ѿ�ת�˹̶��Ƕ�
					{
#if DEBUG_MOTO
						u3_printf("MOTO_SUB_STATE_UP_F");
						u3_printf("������ת��\n");
#endif

						SysTask.Moto_SubState[GLASS_01] = MOTO_SUB_STATE_DOWN;
					}

					break;

				case MOTO_SUB_STATE_DOWN: //�½��� ��ת
#if DEBUG_MOTO
					u3_printf("MOTO_SUB_STATE_DOWN");
					u3_printf("��ת��\n");
#endif

					if (SysTask.Lock_State[GLASS_01] == LOCK_STATE_DEBOUNSE) //׼����������ת��ȥ��ԭ�� ����. �ⲿ��Ҫ��moto����ģ���������PWM�����
					{
						SysTask.PWM_State[GLASS_01] = PWM_STATE_OPEN;
						SysTask.PWM_SubState[GLASS_01] = PWM_SUB_REV;
						break;
					}

					if (g_bSensorState == TRUE)
						SysTask.Moto_RunTime[GLASS_01] =
							 SysTask.nTick + g_au16MoteTime[SysTask.Moto_Time[GLASS_01]][1] + MOTO_DEVIATION_TIME;
					else 
						SysTask.Moto_RunTime[GLASS_01] =
							 SysTask.nTick + g_au16MoteTime[SysTask.Moto_Time[GLASS_01]][1];

					SysTask.PWM_State[GLASS_01] = PWM_STATE_REV_P;
					SysTask.Moto_SubState[GLASS_01] = MOTO_SUB_STATE_ZERO_L;
					break;

				case MOTO_SUB_STATE_ZERO_L: //�����
					if (SysTask.Lock_State[GLASS_01] == LOCK_STATE_DEBOUNSE) //׼����������ת��ȥ��ԭ�� ����. �ⲿ��Ҫ��moto����ģ���������PWM�����
					{
						SysTask.PWM_State[GLASS_01] = PWM_STATE_OPEN;
						SysTask.PWM_SubState[GLASS_01] = PWM_SUB_REV;
					}

					if (GPIO_ReadInputDataBit(LOCK_ZERO_GPIO, LOCK_ZERO_01_PIN) == 0) // �ҵ�ԭ�㣬
					{
#if DEBUG_MOTO
						u3_printf(" MOTO_SUB_STATE_ZERO_L");
						u3_printf("�ҵ�ԭ�㣬\n");
#endif

						if (SysTask.Lock_State[GLASS_01] == LOCK_STATE_DEBOUNSE) //׼�������������κδ���
							break;

						if (SysTask.Moto_Mode[GLASS_01] != SysTask.Moto_ModeSave[GLASS_01]) //״̬�ı䣬��ת������ֹͣ״̬ʱ�����������֧ 
						{
#if DEBUG_MOTO
							u3_printf(" MOTO_SUB_STATE_ZERO_L");
							u3_printf("״̬�ı�\n");
#endif

							//GPIO_ResetBits(MOTO_P_GPIO, MOTO_01_PIN_P); //����ͣ���ת��
							//GPIO_ResetBits(MOTO_N_GPIO, MOTO_01_PIN_N); //����ͣ���ת��
							SysTask.PWM_State[GLASS_01] = PWM_STATE_STOP;
							SysTask.Moto_State[GLASS_01] = MOTO_STATE_INIT; //����ʱһ��ʱ���ٷ�ת
							break;
						}

						SysTask.Moto_SubState[GLASS_01] = MOTO_SUB_STATE_DOWN_F;
						SysTask.Moto_RunTime[GLASS_01] =
							 SysTask.nTick + g_au16MoteTime[SysTask.Moto_Time[GLASS_01]][1] + MOTO_DEVIATION_TIME;
					}
					else if (SysTask.Moto_RunTime[GLASS_01] == 0) //ת��һ���ĽǶ�û���ҵ�ԭ�㣬
					{
#if DEBUG_MOTO
						u3_printf("MOTO_SUB_STATE_ZERO_L");
						u3_printf("û���ҵ� ԭ�㣬\n");
#endif

						if (SysTask.Lock_State[GLASS_01] == LOCK_STATE_DEBOUNSE) //׼�������������κδ���
							break;

						if (SysTask.Moto_Mode[GLASS_01] != SysTask.Moto_ModeSave[GLASS_01]) //״̬�ı䣬��ת������ֹͣ״̬ʱ�����������֧ 
						{
							SysTask.PWM_State[GLASS_01] = PWM_STATE_STOP;
							SysTask.PWM_SubState[GLASS_01] = PWM_SUB_INIT;

							SysTask.Moto_State[GLASS_01] = MOTO_STATE_INIT; //����ʱһ��ʱ���ٷ�ת
							break;
						}

						SysTask.Moto_SubState[GLASS_01] = MOTO_SUB_STATE_DOWN_F;
						SysTask.Moto_RunTime[GLASS_01] =
							 SysTask.nTick + g_au16MoteTime[SysTask.Moto_Time[GLASS_01]][1];
					}

					break;

				case MOTO_SUB_STATE_DOWN_F: //������ת forward
					if (SysTask.Lock_State[GLASS_01] == LOCK_STATE_DEBOUNSE) //׼����������ת��ȥ��ԭ�� ����. �ⲿ��Ҫ��moto����ģ���������PWM�����
					{
						SysTask.PWM_State[GLASS_01] = PWM_STATE_OPEN;
						SysTask.PWM_SubState[GLASS_01] = PWM_SUB_FEW;
						break;
					}

					if (SysTask.Moto_RunTime[GLASS_01] == 0) // ��ʱ����� ��˵�� �Ѿ�ת�˹̶��Ƕ�
					{
#if DEBUG_MOTO
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
			if ((GPIO_ReadInputDataBit(LOCK_ZERO_GPIO, LOCK_ZERO_01_PIN) == 0) ||
				 (SysTask.PWM_State[GLASS_01] == PWM_STATE_STOP)) //Ҫ��ص�ԭ��ſ���
			{
#if DEBUG_MOTO
				u3_printf(" MOTO_STATE_STOP ֹͣ\n");
#endif

				//GPIO_ResetBits(MOTO_P_GPIO, MOTO_01_PIN_P); //����ͣ���ת��
				//GPIO_ResetBits(MOTO_N_GPIO, MOTO_01_PIN_N); //����ͣ���ת��
				SysTask.PWM_State[GLASS_01] = PWM_STATE_STOP;
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
* ����: �����������ָ��ʶ����ȷ����Կ�������� �ڿ���״̬������һ��ָ�ư���?
	?������û��¼���Ҳ���Թ� 
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
					SysTask.Lock_StateTime[GLASS_01] = MOTO_DEBOUNCE; //��ʱ
					SysTask.Lock_State[GLASS_01] = LOCK_STATE_DEBOUNSE;
					SysTask.LockModeSave[GLASS_01] = SysTask.LockMode[GLASS_01];
					SysTask.Lock_OffTime[GLASS_01] = LOCK_OFFTIME;
					GPIO_SetBits(LED_GPIO, LED_PIN_SINGLE);
				}
				else //��Ҫ����
				{
					SysTask.LockModeSave[GLASS_01] = SysTask.LockMode[GLASS_01];
					SysTask.Lock_StateTime[GLASS_01] = SysTask.nTick + LOCK_SET_L_TIME;
					SysTask.Lock_State[GLASS_01] = LOCK_STATE_SENT;
					GPIO_ResetBits(LED_GPIO, LED_PIN_SINGLE);
				}
			}

			break;

		case LOCK_STATE_DEBOUNSE: //
			if (SysTask.Lock_StateTime[GLASS_01] == 0)
			{
				if (GPIO_ReadInputDataBit(LOCK_ZERO_GPIO, LOCK_ZERO_01_PIN) != 0) //Ҫ��ص�ԭ��ſ���
					break;

#if DEBUG
				u3_printf(" lock ��������������\n");
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

			g_FingerAck = ACK_SUCCESS;
			g_AckType = ZERO_ACK_TYPE;
			g_GlassSendAddr = GLASS_01;
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

			//				memcpy(SysTask.FlashData.u8FlashData, SysTask.WatchState, sizeof(SysTask.WatchState));
			for (i = 0; i < GLASS_MAX_CNT; i++)
			{
				SysTask.FlashData.Data.WatchState[i].MotoTime = SysTask.WatchState[i].MotoTime;
				SysTask.FlashData.Data.WatchState[i].u8Dir = SysTask.WatchState[i].u8Dir;
				SysTask.FlashData.Data.WatchState[i].u8LockState = SysTask.WatchState[i].u8LockState;
			}

			SysTask.FlashData.Data.u16FingerfCnt = SysTask.u16FingerfCnt;
			SysTask.FlashData.Data.u16LedMode = LED_MODE_DEF; //LEDģʽ
			u8Status = SAVE_WRITE;
			break;

		case SAVE_WRITE:
			if (FLASH_WriteMoreData(g_au32DataSaveAddr[SysTask.u16AddrOffset], SysTask.FlashData.u16FlashData, DATA_READ_CNT + LED_MODE_CNT + FINGER_CNT))
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
* ����:��������
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void KeyTask(void)
{
	int pressTime		= 0;

	switch (SysTask.Key_State)
	{
		case KEY_INIT:
			if (!GPIO_ReadInputDataBit(KEY_GPIO, KEY_01_PIN))
			{
				SysTask.KeyStateTime = KEY_PRESS_TIME;
				SysTask.Key_State	= KEY_DEBOUCH;
			}

			break;

		case KEY_DEBOUCH:
			if (!GPIO_ReadInputDataBit(KEY_GPIO, KEY_01_PIN))
				break;

			pressTime = KEY_PRESS_TIME - SysTask.KeyStateTime;

			if (SysTask.TouchState != TOUCH_MATCH_INIT) //ָ��ģ�黹������ ������״̬ ����û�г��� ���ȴ�ָ�� ģ������״̬ �˳�֮�� �ٽ���
				return;

			if (pressTime < KEY_DEBOUCH_TIME)
			{ //����
				SysTask.Key_State	= KEY_INIT;
			}
			else if (pressTime < KEY_SHORT_TIME)
			{
				SysTask.Key_State	= KEY_ADD_USER;
			}
			else 
			{
				SysTask.Key_State	= KEY_CLR_USER;
			}

			break;

		case KEY_ADD_USER:
#if DEBUG
			u3_printf("�̰�������û�\n");
#endif

			SysTask.TouchState = TOUCH_KEY_SHORT;
			SysTask.TouchSub = TOUCH_SUB_INIT;
			SysTask.Key_State = KEY_WAIT;
			SysTask.KeyStateTime = 400; //��ʱһ��ʱ�����������key
			break;

		case KEY_CLR_USER:
#if DEBUG
			u3_printf("������ɾ���û�\n");
#endif

			SysTask.TouchState = TOUCH_DEL_USER;
			SysTask.Key_State = KEY_WAIT;
			SysTask.KeyStateTime = 400; //��ʱһ��ʱ�����������key
			break;

		case KEY_WAIT:
			if ((SysTask.KeyStateTime != 0) || (SysTask.TouchState != TOUCH_MATCH_INIT)) //����ɾ���û���Ҫһ����ʱ��
				break;

#if DEBUG
			u3_printf("-----������������-----\n");
#endif

			SysTask.Key_State = KEY_INIT;
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
void MainTask(void)
{
	SysSaveDataTask();
	KeyTask();
	FingerTouchTask();								//ָ��ģ��
	Moto_01_Task(); 								//ֻ������ת��
	PWM_01_Task();
	Lock_01_Task();
	LedTask();
	LedStreamTask();
}


/*******************************************************************************
* ����: 
* ����: 
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void SysEventInit(void)
{
	g_FingerEvent.bInit = TRUE;
	g_FingerEvent.bMutex = FALSE;
	g_FingerEvent.MachineCtrl.pData = (void *)
	malloc(sizeof(FingerData_t));
	g_FingerEvent.MachineCtrl.CallBack = Finger_Event;
	g_FingerEvent.MachineCtrl.u16InterValTime = 5;	//5ms ����һ��

	g_FingerEvent.MachineRun.bEnable = TRUE;
	g_FingerEvent.MachineRun.u32StartTime = 0;
	g_FingerEvent.MachineRun.u32LastTime = 0;
	g_FingerEvent.MachineRun.u32RunCnt = 0;
	g_FingerEvent.MachineRun.bFinish = FALSE;
	g_FingerEvent.MachineRun.u16RetVal = 0;
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

			if (i == 0)
			{
				SysTask.WatchState[i].u8Dir = MOTO_FR_FWD_REV; //ֻ������ת��ֹͣģʽ������ѡ��
			}
		}

		//			memcpy(SysTask.FlashData.u8FlashData, SysTask.WatchState, sizeof(SysTask.WatchState));
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

				if (i == 0)
				{
					SysTask.WatchState[i].u8Dir = MOTO_FR_FWD_REV; //ֻ������ת��ֹͣģʽ������ѡ��
				}
			}

			//memcpy(SysTask.FlashData.u8FlashData, SysTask.WatchState, sizeof(SysTask.WatchState));
			for (i = 0; i < GLASS_MAX_CNT; i++)
			{
				SysTask.FlashData.Data.WatchState[i].MotoTime = SysTask.WatchState[i].MotoTime;
				SysTask.FlashData.Data.WatchState[i].u8Dir = SysTask.WatchState[i].u8Dir;
				SysTask.FlashData.Data.WatchState[i].u8LockState = SysTask.WatchState[i].u8LockState;
			}

			//SysTask.FlashData.Data.u16FingerfCnt = SysTask.u16FingerfCnt;
			//SysTask.FlashData.Data.u16LedMode = LED_MODE_ON; //LEDģʽ
			break;
		}

	}

	//		ʹ��memcpy�ᵼ��TIM 2 TICk�޷������жϣ����������ж�û���ܵ�Ӱ�죬ԭ��δ֪������8����û������
	//		����9�����Ͼͻ���֡�
	//		memcpy(SysTask.WatchState, SysTask.FlashData.u8FlashData, 10);
	for (i = 0; i < GLASS_MAX_CNT; i++)
	{
		SysTask.WatchState[i].MotoTime = SysTask.FlashData.Data.WatchState[i].MotoTime;
		SysTask.WatchState[i].u8Dir = SysTask.FlashData.Data.WatchState[i].u8Dir;

		//			SysTask.WatchState[i].u8LockState	= SysTask.FlashData.Data.WatchState[i].u8LockState;  //��״̬���ڲ����ϴ������±ߵ�Ƭ����֤
	}

	SysTask.LED_Mode	= SysTask.FlashData.Data.u16LedMode;
	SysTask.u16FingerfCnt = SysTask.FlashData.Data.u16FingerfCnt;

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

	SysTask.TouchSub	= TOUCH_SUB_INIT;
	SysTask.bEnFingerTouch = FALSE;
	SysTask.nTick		= 0;
	SysTask.u16SaveTick = 0;
	SysTask.bEnSaveData = FALSE;

	SysTask.TouchState	= TOUCH_CHECK;				//��������ָ��ģʽ
	FINGER_POWER_ON();								//��ָ��ģ��DSP�ϵ磬���������һ��ָ��ID �Ƿ���  EEPRO		M  һ��
	SysTask.u32TouchWaitTime = POWER_ON_DELAY;		//


	SysTask.SendState	= SEND_IDLE;
	SysTask.SendSubState = SEND_SUB_INIT;
	SysTask.bTabReady	= FALSE;
	SysTask.LED_ModeSave = LED_MODE_DEF;			// ��֤��һ�ο�������ģʽ
	SysTask.Stream_State = STREAM_STATE_INIT;
	SysTask.u32StreamTime = 0;
	SysTask.Key_State	= KEY_INIT;
	SysTask.KeyStateTime = 0;
	SysTask.u8StreamShoWCnt = 0;



	for (i = 0; i < GLASS_MAX_CNT; i++)
	{
		SysTask.LockModeSave[i] = LOCK_OFF;
		SysTask.LockMode[i] = LOCK_OFF;
	}


	for (i = 0; i < GLASS_MAX_CNT; i++)
	{
		SysTask.PWM_State[i] = PWM_STATE_INIT;
		SysTask.PWM_SubState[i] = PWM_SUB_INIT;

	}

	for (i = 0; i < GLASS_MAX_CNT; i++)
	{
		SysTask.Moto_StateTime[i] = 0;
		SysTask.Moto_State[i] = MOTO_STATE_INIT;	//��֤����������ת
		SysTask.Moto_SubState[i] = MOTO_SUB_STATE_RUN;
		SysTask.Lock_StateTime[i] = 0;				//			
	}

	SysEventInit();

	//		SysTask.u16BootTime = 3000;	//3��ȴ��ӻ�������ȫ
}


