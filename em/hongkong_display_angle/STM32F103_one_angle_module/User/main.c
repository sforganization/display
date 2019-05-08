
#include <string.h>

#include <stdio.h>

#include "delay.h"
#include "sys.h"
#include "usart1.h"
#include "usart2.h"
#include "usart3.h"

#include "timer.h"
#include "pwm.h"

//#include "pwm.h"
#include "ws2812b.h"
#include "wdg.h"


#define _MAININC_
#include "SysComment.h"
#undef _MAININC_
#define usart2_baund			57600//����2�����ʣ�����ָ��ģ�鲨���ʸ��ģ�ע�⣺ָ��ģ��Ĭ��57600��


void SysTickTask(void);


void DebugGPIOInit(void)
{
	/* ����IOӲ����ʼ���ṹ����� */
	GPIO_InitTypeDef GPIO_InitStructure;

	/* ʹ��(����)KEY1���Ŷ�ӦIO�˿�ʱ�� */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // ��Ҫ��� 5 ���ĸߵ�ƽ���Ϳ������ⲿ��һ���������裬������ԴΪ 5 �������Ұ� GPIO ����Ϊ��©ģʽ�����������̬ʱ������������͵�Դ������� 5���ĵ�ƽ

	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_SetBits(GPIOC, GPIO_Pin_13);

    delay_ms(3);
	GPIO_ResetBits(GPIOC, GPIO_Pin_13);
}


/*******************************************************************************
* ����: 
* ����: ������һ����Ƭ���ϵ����\�ϵ�
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void LockGPIOInit(void)
{
	/* ����IOӲ����ʼ���ṹ����� */
	GPIO_InitTypeDef GPIO_InitStructure;

	/* ʹ��(����)KEY1���Ŷ�ӦIO�˿�ʱ�� */
	LOCK_RCC_CLOCKCMD(LOCK_RCC_CLOCKGPIO, ENABLE);

	GPIO_InitStructure.GPIO_Pin = LOCK_01_PIN;

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // ��Ҫ��� 5 ���ĸߵ�ƽ���Ϳ������ⲿ��һ���������裬������ԴΪ 5 �������Ұ� GPIO ����Ϊ��©ģʽ�����������̬ʱ������������͵�Դ������� 5���ĵ�ƽ

	GPIO_Init(LOCK_GPIO, &GPIO_InitStructure);

	GPIO_SetBits(LOCK_GPIO, LOCK_01_PIN);
}


/*******************************************************************************
* ����: 
* ����: 
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void LEDGPIOInit(void)
{
	/* ����IOӲ����ʼ���ṹ����� */
	GPIO_InitTypeDef GPIO_InitStructure;

	/* ʹ��(����)KEY1���Ŷ�ӦIO�˿�ʱ�� */
	LED_RCC_CLOCKCMD(LED_RCC_CLOCKGPIO, ENABLE);

	GPIO_InitStructure.GPIO_Pin = LED_PIN_SINGLE | LED_PIN_STREAM;

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // ��Ҫ��� 5 ���ĸߵ�ƽ���Ϳ������ⲿ��һ���������裬������ԴΪ 5 �������Ұ� GPIO ����Ϊ��©ģʽ�����������̬ʱ������������͵�Դ������� 5���ĵ�ƽ

	GPIO_Init(LED_GPIO, &GPIO_InitStructure);

	GPIO_ResetBits(LED_GPIO, LED_PIN_SINGLE);
	GPIO_ResetBits(LED_GPIO, LED_PIN_STREAM);
}


/*******************************************************************************
* ����: 
* ����: �����io
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void Moto_ZeroGPIOInit(void)
{
	/* ����IOӲ����ʼ���ṹ����� */
	GPIO_InitTypeDef GPIO_InitStructure;

	/* ʹ��(����)KEY1���Ŷ�ӦIO�˿�ʱ�� */
	LOCK_ZERO_RCC_CLOCKCMD(LOCK_ZERO_RCC_CLOCKGPIO, ENABLE);

	GPIO_InitStructure.GPIO_Pin = LOCK_ZERO_01_PIN;

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	//��������

	GPIO_Init(KEY_GPIO, &GPIO_InitStructure);
}


/*******************************************************************************
* ����: 
* ����: ����
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void KeyGPIOInit(void)
{
	/* ����IOӲ����ʼ���ṹ����� */
	GPIO_InitTypeDef GPIO_InitStructure;

	/* ʹ��(����)KEY1���Ŷ�ӦIO�˿�ʱ�� */
	LOCK_ZERO_RCC_CLOCKCMD(KEY_RCC_CLOCKGPIO, ENABLE);

	GPIO_InitStructure.GPIO_Pin = KEY_01_PIN;

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	//��������

	GPIO_Init(LOCK_ZERO_GPIO, &GPIO_InitStructure);
}


/*******************************************************************************
* ����: 
* ����: �������io  һ����һ������������ת����, ʹ��PWM���� 
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void Moto_DirGPIOInit(void)
{
	//	/* ����IOӲ����ʼ���ṹ����� */
	//	GPIO_InitTypeDef GPIO_InitStructure;
	//	/* ʹ��(����)���Ŷ�ӦIO�˿�ʱ�� 						+ */
	//	MOTO_P_RCC_CLOCKCMD(MOTO_P_RCC_CLOCKGPIO, ENABLE);
	//	GPIO_InitStructure.GPIO_Pin =	MOTO_01_PIN_P;
	//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//
	//	GPIO_Init(MOTO_P_GPIO, &GPIO_InitStructure);
	//	/* ʹ��(����)���Ŷ�ӦIO�˿�ʱ�� 					-*/
	//	MOTO_N_RCC_CLOCKCMD(MOTO_N_RCC_CLOCKGPIO, ENABLE);
	//	  
	//	GPIO_InitStructure.GPIO_Pin =	MOTO_01_PIN_N;
	//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//
	//	GPIO_Init(MOTO_N_GPIO, &GPIO_InitStructure);
}


/*******************************************************************************
* ����: 
* ����: debug
* �β�:		
* ����: ��
* ˵��: 
*******************************************************************************/
void Debug_GPIOInit(void)
{
	/* ����IOӲ����ʼ���ṹ����� */
	GPIO_InitTypeDef GPIO_InitStructure;

	/* ʹ��(����)KEY1���Ŷ�ӦIO�˿�ʱ�� */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	//��������

	GPIO_Init(GPIOC, &GPIO_InitStructure);
}


int main(void)
{
	delay_init();									//��ʱ������ʼ��	  
	NVIC_Configuration();							//����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ� ��������ֻ����һ��;				 
	usart2_init(19200); 							//��ʼ������2,������ָ��ģ��ͨѶ ��������ʼ��һ����ʱ�� ��Ϊ�������� ���ж� 
	KeyGPIOInit();
	LEDGPIOInit();
	LockGPIOInit();
	Moto_ZeroGPIOInit();
	Moto_DirGPIOInit();

	FingerGPIO_Init();								//��ʼ��FR��״̬����  ʶ��ָ�ư�ѹ����ߵ�ƽ����Ϊ���ù�ϵ��ʼ�������ŵ������

	usart3_init(115200); //debug ��

	//��������Ӧ״̬(������Ӧʱ����ߵ�ƽ�ź�)
#if DEBUG
	DebugGPIOInit();
	u3_printf("----------------start--------------\n");
#endif

	San_PWM_Init(PWM_PERIOD, PWM_PRESCALER);		//200hz
	ws2812bInit();									//����Ὺһ����ʱ��1��Ϊpwm��� 
	IWDG_Init(4, 3750);								//��Ƶϵ��Ϊ64������ֵΪ625�����ʱ��Ϊ1S                      3750    6S
	SysInit();
	GENERAL_TIMx_Configuration();

	while (1)
	{
		MainTask();

		if (SysTask.nTick)
		{
			SysTask.nTick--;
			SysTickTask();
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
void SysTickTask(void)
{
	vu16 static 	u16SecTick = 0; 				//�����
	u8				i;

	if (u32SysSoftDog++ > 200)  //ι��
	{
		u32SysSoftDog		= 0;
		IWDG_Feed();
	}

	SysTask.u32SysTimes++;							//ϵͳ����ʱ��

	if (u16SecTick++ >= 1000) //������
	{
		u16SecTick			= 0;

		if (SysTask.u16SaveTick)
		{
			SysTask.u16SaveTick--;

			if (SysTask.u16SaveTick == 0)
				SysTask.bEnSaveData = TRUE; //��������
		}
	}

	if (SysTask.u16SaveWaitTick)
		SysTask.u16SaveWaitTick--;

	if (SysTask.u32TouchWaitTime)
		SysTask.u32TouchWaitTime--;

	if (SysTask.u32StreamTime)
		SysTask.u32StreamTime--;

	if (SysTask.KeyStateTime)
		SysTask.KeyStateTime--;



	for (i = 0; i < GLASS_MAX_CNT; i++)
	{
		if (SysTask.Moto_StateTime[i])
			SysTask.Moto_StateTime[i] --;

		if (SysTask.Moto_RunTime[i])
			SysTask.Moto_RunTime[i] --;

		if (SysTask.Moto_WaitTime[i])
			SysTask.Moto_WaitTime[i] --;

		if (SysTask.Lock_StateTime[i])
			SysTask.Lock_StateTime[i] --;

		if (SysTask.Lock_OffTime[i])
			SysTask.Lock_OffTime[i] --;
	}



	//			if (state)
	//			{
	//				state				= 0;
	//				GPIO_ResetBits(GPIOC, GPIO_Pin_13);
	//			}
	//			else 
	//			{
	//				state				= 1;
	//				GPIO_SetBits(GPIOC, GPIO_Pin_13);
	//			}
}


