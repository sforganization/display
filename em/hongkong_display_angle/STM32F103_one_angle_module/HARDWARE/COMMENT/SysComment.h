

#ifndef __SYSCOMMENT_H__
#define __SYSCOMMENT_H__

#ifdef _MAININC_
#define EXTERN

#else

#define EXTERN					extern
#endif


/* ͷ�ļ�  ------------------------------------------------------------*/
#include "stm32f10x.h"
#include"finger.h"
#include "machine.h"


#define DEBUG_MOTO      1
#define DEBUG           1




#define LED_BRIGHNESS               0x000010

#define PWM_PERIOD                  (5000 - 1)              //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ 
#define PWM_PRESCALER               (72 - 1)                //Ԥ��Ƶֵ     ��TIMxʱ��Ƶ��Ϊ72M / (PWM_PRESCALER + 1) = 1000K, pwmƵ��Ϊ  72M / (PWM_PRESCALER + 1) / ��PWM_PERIOD + 1�� = hz
#define PWM_POSITIVE                (PWM_PERIOD * 4 / 10)   // ռ�ձ� 1 / 5
#define PWM_HZ                       (72000000 / (PWM_PRESCALER + 1) / (PWM_PERIOD + 1))

//	72M / (PWM_PRESCALER + 1) / ��PWM_PERIOD + 1��
#define MOTO_ANGLE_TIME 		4000//�ߵ�ƽʱ��Ҫ 7000ms���ң�TIME = 7000 / (PWM_POSITIVE * 1000 / pwm HZ )  ���ת��һ���Ƕȵ�ʱ��
#define MOTO_DEVIATION_TIME 	600 		//���ת��ʱ��ƫ�� 200ms ����ƫ��������дflashʱ����Ϊдflash��ֹһ�� flash����������Ķ�ȡ���������жϣ�

#define GLASS_MAX_CNT               1        //�ֱ�����

#define SAVE_TIME                   10000    //10���޶����Զ��������ڵ�״̬��Ϣ
#define DATA_READ_CNT               2        //��16 bit�洢��1���ֱ�ռ�� 3*1 = 3 �ֽ� ����ȡ2
#define LED_MODE_CNT                1        //LEDģʽ ��16�ֽڴ洢 ����ڽض�����
#define FINGER_CNT                  1        //ָ�Ƹ��� ��16�ֽڴ洢 ����ڽض�����
#define PASSWD_CNT                  3        //����������� ��16�ֽڴ洢   6λ����ռ��3

#define LOCK_OFFTIME                4000
#define SAVE_TICK_TIME              30      //�����ݸ���30S��д��


#define     FINGER_MAX_CNT          10    //�������ָ���û�����
#define     GLASS_SEND_ALL          0X80  //���λΪ1��ʾ���е��ֱ�
#define     MCU_ACK_ADDR            0xFF  //Ӧ���ַ
#define     FINGER_ACK_TYPE         0x01  //ָ��Ӧ���ַ
#define     ZERO_ACK_TYPE           0x02  //���Ӧ���ַ
#define     DETECT_ACK_TYPE         0x03  //������Ӧ��
#define     ERROR_ACK_TYPE          0xFF  //����Ӧ��


#define     LOCK_OFF                 1
#define     LOCK_ON                  0

#define     GLASS_01                 0

//#define FLASH_SAVE_ADDR         0x0807D000 	    // 103ZE 0x0807D000   512K  ���� ÿҳ2k ��ϸ��STM32���Ĳο��ֲ�_V10�� p31
                                                //����FLASH �����ַ(����Ϊż��������������,Ҫ���ڱ�������ռ�õ�������.
										        //����,д������ʱ��,���ܻᵼ�²�����������,�Ӷ����𲿷ֳ���ʧ.��������.
                                                //512k ��255ҳ����250ҳ����������ĸ�ҳ����������Ϣ��251~255����5ҳ��
                                                //���251ҳflashʧЧ�򱣴浽252ҳ���������ƣ���ſ���д10k * 5 = 50k��
                                                //��ÿ���޸�10�μ�����ؿ�����13��

#define FLASH_SAVE_ADDR         0X0800E800 	    // 103c8 0X0800E800   64K ���� ÿҳ1k ��ϸ��STM32���Ĳο��ֲ�_V10�� p31
                                                //����FLASH �����ַ(����Ϊż��������������,Ҫ���ڱ�������ռ�õ�������.
										        //����,д������ʱ��,���ܻᵼ�²�����������,�Ӷ����𲿷ֳ���ʧ.��������.
                                                //64k ��64ҳ 
                                                //5 ҳ   58 ~ 63
										
/*
*  ��Ϊ���ݴ洢��������������д�������������ʱ����flash�Ѿ���
*/
	

/*
		ҳ		��ʼ��ַ					    ������ַ
		255		0x0807F800		~			0X0807FFFF
		254		0x0807F000		~			0X0807F7FF
		253		0x0807E800		~			0X0807EFFF
		252		0x0807E000		~			0X0807F7FF
		251		0x0807D800		~			0X0807DFFF
		250		0x0807D000		~			0X0807F7FF
    
*/

#define     DATA_SAVE_ADDR      0x0807D000
#define		DATA_SAVE_SAVE_1	0x0807F800
#define		DATA_SAVE_SAVE_2	0x0807F000
#define		DATA_SAVE_SAVE_3	0x0807E800
#define		DATA_SAVE_SAVE_4	0x0807E000
#define		DATA_SAVE_SAVE_5	0x0807D800



#define MATCH_FINGER_CNT		2
#define FINGER_ID_MAX			10	//�������ָ���û�����Ϊָ��ģ���ϵ�û�и�Ӧ�źţ�����Ҫ�����ϵ�¼����ָ��


#define VOID					void	

//�������������ݴ�С������--���б�������ʹ���ֽ�����
//�豸��Ϣ
#define LED_SHOWFRE_DNS 		300
#define LED_SHOWFRE_OK			1000


#define FINGER_MAX_CNT			10   //ָ��ģ�����洢����

#define DEBOUNCE_TIME			40 //40ms ����

#define ADDR_MAX_OFFSET         6


#define MOTO_DEBOUNCE			4	//40ms ��ʱ�����е�λ��
#define LOCK_HAL_OPEN		    500	//500ms �͵�ƽ����
#define MOTO_LOCK_SUCCESS		800 //400ms �������




#define LED_RCC_CLOCKCMD             RCC_APB2PeriphClockCmd
#define LED_RCC_CLOCKGPIO            RCC_APB2Periph_GPIOA
#define LED_GPIO                     GPIOA

#define LED_PIN_SINGLE               GPIO_Pin_0  //LED
#define LED_PIN_STREAM               GPIO_Pin_8  //LED   ��ˮ���� 



#define LOCK_ZERO_RCC_CLOCKCMD       RCC_APB2PeriphClockCmd
#define LOCK_ZERO_RCC_CLOCKGPIO      RCC_APB2Periph_GPIOA
#define LOCK_ZERO_GPIO               GPIOA

#define LOCK_ZERO_01_PIN             GPIO_Pin_11


//#define MOTO_P_RCC_CLOCKCMD          RCC_APB2PeriphClockCmd   //ʹ��Ӳ��pwm  PB0 PB1
//#define MOTO_P_RCC_CLOCKGPIO         RCC_APB2Periph_GPIOA
//#define MOTO_P_GPIO                  GPIOA

//#define MOTO_01_PIN_P                GPIO_Pin_6  //PWM+

//#define MOTO_N_RCC_CLOCKCMD          RCC_APB2PeriphClockCmd
//#define MOTO_N_RCC_CLOCKGPIO         RCC_APB2Periph_GPIOA
//#define MOTO_N_GPIO                  GPIOA

//#define MOTO_01_PIN_N                GPIO_Pin_7 //PWM-

#define LOCK_RCC_CLOCKCMD            RCC_APB2PeriphClockCmd
#define LOCK_RCC_CLOCKGPIO           RCC_APB2Periph_GPIOA
#define LOCK_GPIO                    GPIOA

#define LOCK_01_PIN                  GPIO_Pin_12  //��

#define KEY_RCC_CLOCKCMD            RCC_APB2PeriphClockCmd
#define KEY_RCC_CLOCKGPIO           RCC_APB2Periph_GPIOA
#define KEY_GPIO                    GPIOA

#define KEY_01_PIN                  GPIO_Pin_15  //����

//    static u16 u16WriteCount = 0;  //��д����
//    static u16 u16EraseCount = 0;  //�ز�д����
//                FLASH_ReadMoreData(FLASH_SAVE_ADDR, readData, 8);
//                FLASH_WriteMoreData(FLASH_SAVE_ADDR, writeData, 8);
//                FLASH_ReadMoreData(FLASH_SAVE_ADDR, readData, 8); 



typedef enum 
{
    LED_MODE_ON = 0,    //���� 
    LED_MODE_OFF,
    LED_MODE_DEF = 0XFF, 
} LEDMode_T;

typedef enum 
{
    KEY_INIT = 0, 
    KEY_DEBOUCH, //����
    KEY_ADD_USER, //�����û�
    KEY_CLR_USER, //ɾ���û�
    KEY_WAIT,    
    KEY_IDLE,
    KEY_DEF = 0XFF, 
} KeyState_T;


typedef enum 
{
   TOUCH_MATCH_INIT = 0, 
   TOUCH_MATCH_AGAIN, //�ٴ�ƥ��
   TOUCH_ADD_USER, //�����û�
   TOUCH_DEL_USER, //ɾ���û�
   TOUCH_KEY_SHORT, //�����̰�
   TOUCH_WAIT,
   TOUCH_CHECK,										//
   TOUCH_IDLE,
   TOUCH_DEF = 0XFF, 
} TouchState_T;


typedef enum 
{
    TOUCH_SUB_INIT = 0, 
    TOUCH_SUB_ENTER,									//
    TOUCH_SUB_SETMODE_ONE,									//���ÿ��ظ�ģʽ
    TOUCH_SUB_AGAIN_PON,								//
    TOUCH_SUB_SETMODE_TWO,									//���ÿ��ظ�ģʽ
    TOUCH_SUB_CHECK_ROOT,									//
    TOUCH_SUB_AGAIN,									//
    TOUCH_SUB_WAIT, 
    TOUCH_SUB_EXIT, 
    TOUCH_SUB_DEF = 0XFF, 
} TouchSub_T;    
    
typedef struct WATCH_STATE
{
    u8 u8LockState; //��״̬,�����ϴ�����Ҫ���أ���Ϊ������һ��ʱ�����Զ��رգ���ƽ��˴���Ϳ���
    u8 u8Dir; //ת������
    u8 MotoTime; //ת��ʱ��      MotoTime_e  ����������
}WatchState_t;  //�ֱ�״̬

typedef struct 
{
    WatchState_t    WatchState[GLASS_MAX_CNT];
    u16             u16LedMode;
    u16             u16FingerfCnt;                      //
} PackData_t;


typedef union   PACK_U
{
    PackData_t      Data;
    u8              u8FlashData[(DATA_READ_CNT + LED_MODE_CNT  + FINGER_CNT) * 2];
    u16             u16FlashData[DATA_READ_CNT + LED_MODE_CNT + FINGER_CNT];    //��ΪFlash��ȡΪ16��bitΪ��λ��������ת��ע���ֽ����д�С�˵�����
}FlashData_u;

typedef enum 
{
    MOTO_TIME_TPD = 0, //ÿ��Ķ���ģʽ������12Сʱ��ֹͣ12Сʱ
    MOTO_TIME_650, //��ת2���ӣ�ֹͣ942S
    MOTO_TIME_750,  //��ת2���ӣ�ֹͣ800S
    MOTO_TIME_850,  //��ת2���ӣ�ֹͣ693S
    MOTO_TIME_1000, //��ת2���ӣ�ֹͣ570S
    MOTO_TIME_1950, //��ת2���ӣ�ֹͣ234S

    
    MOTO_TIME_ANGLE,  //ת��һ���Ƕ�
    MOTO_TIME_OFF = 0XFE,  //����״̬�£����ֹͣ

    MOTO_TIME_DEF = 0XFF,
}MotoTime_e;  
    

typedef enum 
{
    MOTO_FR_FWD = 0,  //��ת
    MOTO_FR_REV,      //��ת
    MOTO_FR_FWD_REV, //����ת
    MOTO_FR_STOP, //ֹͣ
    MOTO_FR_DEF = 0XFF,
}MotoFR;  

typedef enum 
{
    SEND_INIT = 0,     // ��ʼ״̬Ϊ��ѯ�����µĸ��ӻ�״̬   �ӻ���״ֻ̬�����ڸ����ӻ���
    SEND_TABLET_ALL,         //����ȫ���ӻ���ƽ��
    SEND_TABLET_FINGER,      //����ָ��ģ��Ӧ���ƽ��
    SEND_TABLET_DETECT,      //���ʹ�����״̬��ƽ��
    SEND_TABLET_ZERO,//�������Ӧ���ƽ��
    SEND_WAIT,         //�ȴ�״̬
    SEND_IDLE,         //����״̬
    SEND_DEF = 0XFF,
}SendState_T;  


typedef enum 
{
    SEND_SUB_INIT = 0,     // ��ʼ״̬Ϊ��ѯ�����µĸ��ӻ�״̬   �ӻ���״ֻ̬�����ڸ����ӻ���
    SEND_SUB_IDLE,         //����״̬
    SEND_SUB_WAIT,         //�ȴ�״̬ ���ȴ��ӻ�Ӧ��
    SEND_SUB_DEF = 0XFF,
}SendSubState_T; 

typedef enum 
{
    HtoS_CHECK = 0,     // host to slave init
    HtoS_SEND,         //����״̬
    HtoS_WAIT,         //�ȴ�״̬ ���ȴ�һ��ʱ�䣬�ٷ��ͣ���ʱ��ӻ���Ӧ
    HtoS_DEF = 0XFF,
}HtoS_State_T; 
    
    
typedef enum SAVE
{
    SAVE_INIT = 0, 
    SAVE_WRITE,
    SAVE_WAIT,
    SAVE_EXIT,
    SAVE_DEF = 0XFF, 
} Save_T;
   

typedef enum 
{
    CMD_GET_ATTR = 0,       // ��ȡ����    0
    CMD_ACK,                // Ӧ��1
    CMD_UPDATE,             // ���°�2
    CMD_READY,              // ������3
    CMD_ADD_USER,              // �����û�ָ��4
    CMD_DEL_USER,              // ɾ���û�ָ��5
    CMD_LOCK_MODE,             // ������������״̬6
    CMD_empt,             // ��������
    CMD_LED_MODE,              // LEDģʽ7
    CMD_PASSWD_ON,           // ����ģʽ������ָ��8
    CMD_PASSWD_OFF,           // ����ģʽ���ر�ָ��9
    
    CMD_DEF = 0XFF, 
}SendCmd_T; 



//typedef union SENDARRAY
//{
//    u8 u8SendArray[96];
//    SlaveState_t SlaveState[4];
//}SendArrayUnion_u; //��������������

typedef enum 
{
    MOTO_STATE_INIT = 0,  //
    MOTO_STATE_FIND_ZERO,
    MOTO_STATE_CHANGE_TIME,  //
    MOTO_STATE_CHANGE_DIR,  //
    MOTO_STATE_RUN_NOR,  // ��ת���߷�ת����״̬
    MOTO_STATE_RUN_CHA,  // ����ת����״̬
    MOTO_STATE_STOP,
    MOTO_STATE_WAIT,  //
    MOTO_STATE_IDLE,  //
    MOTO_STATE_DEF = 0XFF,
}MotoState;  


typedef enum 
{
    PWM_STATE_INIT = 0,  //
    PWM_STATE_FEW_P,     //��ת
    PWM_STATE_REV_P,     //��ת
    PWM_STATE_OPEN,     //����
    PWM_STATE_STOP,      //
    PWM_STATE_DEF = 0XFF,
}PWMState;  
    
typedef enum 
{
    PWM_SUB_INIT = 0,  //
    PWM_SUB_FEW,     //��ת
    PWM_SUB_REV,     //��ת
    PWM_SUB_DEF = 0XFF,
}PWMSubState;  
    

typedef enum 
{
    MOTO_SUB_STATE_RUN = 0,  //
    MOTO_SUB_STATE_WAIT,  //
    MOTO_SUB_STATE_UP,  //��������ת
    MOTO_SUB_STATE_ZERO_R,  //�����
    MOTO_SUB_STATE_UP_F,  //������ת forwad
    MOTO_SUB_STATE_DOWN,  //�½��� ��ת
    MOTO_SUB_STATE_ZERO_L,  //�����
    MOTO_SUB_STATE_DOWN_F,  //������ת forwad
    MOTO_SUB_STATE_DEF = 0XFF,
}MotoSubState;  
    
typedef enum 
{
    LOCK_STATE_DETECT = 0,  //
    LOCK_STATE_DEBOUNSE,  //����
    LOCK_STATE_OPEN,  //���ڿ���
    LOCK_STATE_SENT,  //����������3��50ms�͵�ƽ
    LOCK_STATE_OFF,  //��ȫ�������
    LOCK_STATE_DEF = 0XFF,
}LockState_T;  
    
typedef enum 
{
    STREAM_STATE_INIT = 0,  //
    STREAM_STATE_STREAM,
    STREAM_STATE_BLINK_ONE, //��һ������
    STREAM_STATE_BLINK_TWO,
    STREAM_STATE_BLINK_THREE,
    STREAM_STATE_BLINK_ROUR,
    STREAM_STATE_BLINK_ALL, //ȫ��
    STREAM_STATE_OPEN_LOCK,    //����ʱȫ��
    STREAM_STATE_SUCCESS,    //�ɹ�
    STREAM_STATE_FAILED,    //ʧ��
    STREAM_STATE_DEF = 0XFF,
}StreamState;     //��ˮ��Ч�� 
typedef union SENDARRAY
{
    u8 u8SendArray[48];
    WatchState_t    WatchState[16];                     //���16���ֱ�
}SendArrayUnion_u; //��������������

typedef struct 
{
    vu32 			u32SysTimes;					   //ϵͳ����ʱ��
    bool			mUpdate;

    vu16			nTick;								//������
    vu16			nLoadTime;							//��ʾ����ʱ��

    vu32 			u32TouchWaitTime;					//״̬����ʱ
    TouchState_T	TouchState;
    TouchSub_T	    TouchSub;
    bool 			bEnFingerTouch;						//ȫ��ָ��ʶ����
	u16             u16FingerID; 					    //ָ��ģ��ID
    u16         	u16FingerfCnt;						//

    SendState_T     SendState;
    SendSubState_T  SendSubState;
    u8              u8WatchCount;
    u16             u16AddrOffset;
    u16             u16SaveTick;                        //��ʱ����״̬��Ϣʱ��
    u16             u16SaveWaitTick;                    //����״̬����ʱ
    bool 			bEnSaveData;						//sȫ�ܱ���
    
    WatchState_t    WatchState[GLASS_MAX_CNT];          //���16���ֱ�
    WatchState_t    WatchStateSave[GLASS_MAX_CNT];      //���16���ֱ�
    FlashData_u     FlashData;
    
    bool 			bTabReady;							//ƽ���ϵ�׼������
    u8              u8Passwd[PASSWD_CNT * 2];           //���뱣��

    u8              u8LedMode;                          //LEDģʽ
    u8              u8LedModeSave;                      //����LEDģʽ

    PWMState        PWM_State[GLASS_MAX_CNT];          //PWM���Ƶ��
    PWMSubState     PWM_SubState[GLASS_MAX_CNT]; 
    //vu32 			PWM_RunTime[GLASS_MAX_CNT];		    //PWM����ʱ��
    //vu32 			PWM_WaitTime[GLASS_MAX_CNT];		//PWMֹͣʱ��

    vu32            Moto_StateTime[GLASS_MAX_CNT];      //״̬����ʱ
    MotoState       Moto_State[GLASS_MAX_CNT];          //�������״̬
    MotoSubState    Moto_SubState[GLASS_MAX_CNT];       //���������״̬
    MotoFR          Moto_Mode[GLASS_MAX_CNT];           //���ģʽ
    MotoFR          Moto_ModeSave[GLASS_MAX_CNT];       //���ģʽ����
    MotoTime_e      Moto_Time[GLASS_MAX_CNT];           //�������ʱ��ö��
    vu32 			Moto_RunTime[GLASS_MAX_CNT];		//�������ʱ��
    vu32 			Moto_WaitTime[GLASS_MAX_CNT];		//���ֹͣʱ��
    LockState_T     Lock_State[GLASS_MAX_CNT];          //��״̬��
    u8              LockMode[GLASS_MAX_CNT];            // ��״̬ 1 �� or 0 ��
    u8              LockModeSave[GLASS_MAX_CNT];        // ��״̬ 1 �� or 0 ��
    vu32 			Lock_StateTime[GLASS_MAX_CNT];		//״̬����ʱ
    vu32            Lock_OffTime[GLASS_MAX_CNT];        //���ر�ʱ��

    KeyState_T      Key_State;
    vu32            KeyStateTime;      //״̬����ʱ
    
    vu32 			u32StreamTime;					    //״̬����ʱ
    u8              u8StreamShoWCnt;                    //��ˮ����˸����
    StreamState     Stream_State;                       //��ˮ������״̬
    u8              LED_Mode;                           // LEDģʽ
    u8              LED_ModeSave;                       // LEDģʽ
} SYS_TASK;


/* ȫ�ֱ��� -----------------------------------------------------------*/

/* ȫ�ֱ��� -----------------------------------------------------------*/
EXTERN vu32 	u32SysSoftDog; //�������� 

EXTERN SYS_TASK SysTask;


EXTERN void Sys_DelayMS(uint16_t nms);
EXTERN void Sys_GetMac(u8 * mac);
EXTERN void Sys_LayerInit(void);
EXTERN void Sys_IWDGConfig(u16 time);
EXTERN void Sys_IWDGReloadCounter(void);
EXTERN void Sys_1s_Tick(void);

EXTERN void DelayUs(uint16_t nCount);
EXTERN void DelayMs(uint16_t nCount);
EXTERN void Strcpy(u8 * str1, u8 * str2, u8 len);
EXTERN bool Strcmp(u8 * str1, u8 * str2, u8 len);

EXTERN void MainTask(void);
EXTERN void SysInit(void);
EXTERN void SysSaveData(void);


#endif

