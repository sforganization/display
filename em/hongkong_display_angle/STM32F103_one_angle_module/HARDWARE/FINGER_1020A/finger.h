/*******************
LED.h

*******************/

#ifndef _FINGER_H__
#define _FINGER_H__



#define FINGER_DEBUG			1	//�Ƿ�ʹ��debug���ܣ�ʹ�ܵĻ����õ�һ���������


#define FINGER_TOUCH_RCC_CLOCKCMD       RCC_APB2PeriphClockCmd
#define FINGER_TOUCH_RCC_CLOCKGPIO      RCC_APB2Periph_GPIOA
#define FINGER_TOUCH_GPIO		        GPIOA
#define FINGER_TOUCH_PIN		        GPIO_Pin_4	//��������Ӧ״̬(������Ӧʱ����ߵ�ƽ�ź�)
//��ʼ��Ϊ��������			


    
#define FINGER_POWER_RCC_CLOCKCMD       RCC_APB2PeriphClockCmd
#define FINGER_POWER_RCC_CLOCKGPIO      RCC_APB2Periph_GPIOA
#define FINGER_POWER_GPIO		        GPIOA
#define FINGER_POWER_PIN		        GPIO_Pin_5	//����ģ���Դ����

#define FINGER_POWER_ON()		        GPIO_SetBits(FINGER_POWER_GPIO, FINGER_POWER_PIN)
#define FINGER_POWER_OFF()		        GPIO_ResetBits(FINGER_POWER_GPIO, FINGER_POWER_PIN)

#define PS_Sta   PAin(4)//��ָ��ģ��״̬����


#define DATA_START      0xf5//���ݰ���ʼ
#define DATA_END      0xf5//���ݰ�����

#define CMD_ENROLL1     0x01//���ָ�Ʋ���һ
#define CMD_ENROLL2     0x02//���ָ�Ʋ����
#define CMD_ENROLL3     0x03//���ָ�Ʋ�����
#define CMD_DELETE      0x04//ɾ��ָ�����ָ��
#define CMD_CLEAR       0x05//�������ָ��
#define CMD_USERNUMB      0x09//ȡ�û�����
#define CMD_IDENTIFY      0x0b//1:1�ȶ�
#define CMD_SEARCH      0x0c//1:N�ȶ�
#define CMD_MODE_SET      0x2D//¼��ģʽ����

#define F1020_ACK_SUCCESS     0x00  //�����ɹ�
#define F1020_ACK_FAIL        0x01  //����ʧ��
#define F1020_ACK_FULL        0x04  //ָ�����ݿ�����
#define F1020_ACK_NOUSER      0x05  //�޴��û�
#define F1020_ACK_USER_EXIST  0x07  //�û��Ѵ���
#define F1020_ACK_TIMEOUT     0x08  //�ɼ���ʱ



#define F1020_MODE_REC      0x00//¼��ģʽ���� // 0�������ظ�1����ֹ�ظ�

#define FINGER_REV_TIME_OUT    1200      //1.2s ���ղ����˳�

typedef enum 
{
    FINGER_MODE_SET = 0, 
    FINGER_SEARCHING,    //1:N
    FINGER_READ,  
    FINGER_ENROLL,          //ע��ָ��
    FINGER_IDENTIFY,        //��ָ֤��  1:1
    FINGER_CLEAR,           //���ָ��
    FINGER_DELETE,          //ɾ��ָ��ָ��
} FingerOpt_T;   
    

typedef enum 
{
    FINGER_STATE_INIT = 0, 
    FINGER_STATE_WAIT,  
    FINGER_STATE_CHECK, 
    FINGER_STATE_EXIT, 
    FINGER_STATE_DEF = 0xFF, 

} FingerState_T;    

typedef enum 
{
    FINGER_SUB_INIT = 0, 
    FINGER_SUB_STEP1, 
    FINGER_SUB_STEP2, 
    FINGER_SUB_STEP3, 
    FINGER_SUB_STEP4, 
    FINGER_SUB_STEP5, 
    FINGER_SUB_WAIT, 
    FINGER_SUB_DEF = 0xFF, 
} FingerSub_T;    



//#define TRUE  0x01
//#define FALSE  0x00
typedef enum 
{
    FALSE = 0, TRUE = !FALSE,
} bool;

void Delay_ms(unsigned int ms)  ; 
/*******************************************************************************
**ע��ָ��
**��������ָ��ע��һ��ָ��ģ��
**������UserID ָ�ƺ�
*******************************************************************************/
unsigned char Finger_Enroll(unsigned int u_id);

/*******************************************************************************
**��ָ֤��
**
**������UserID ָ�ƺ�
*******************************************************************************/
unsigned char Finger_Identify(void);

/*******************************************************************************
**���ָ��
**
**������UserID ָ�ƺ�
*******************************************************************************/
unsigned char Finger_Clear(void);

/*******************************************************************************
**ɾ��ָ��ָ��
**
**������UserID ָ�ƺ�
*******************************************************************************/
unsigned char Finger_Delete(unsigned int u_id);

int Finger_Read(void);
unsigned char Finger_Search(int *userID);


int Finger_ModeSet(void);


void FingerGPIO_Init(void);

int Finger_Event(char cmd, void *data);

#endif

