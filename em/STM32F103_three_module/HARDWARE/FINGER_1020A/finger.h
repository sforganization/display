/*******************
LED.h

*******************/

#ifndef _FINGER_H__
#define _FINGER_H__




#define ENABLE_DEBUG			0	//�Ƿ�ʹ��debug���ܣ�ʹ�ܵĻ����õ�һ���������



    
#define FINGER_POWER_RCC_CLOCKCMD RCC_APB2PeriphClockCmd
#define FINGER_POWER_RCC_CLOCKGPIO RCC_APB2Periph_GPIOA
#define FINGER_POWER_GPIO		GPIOA
#define FINGER_POWER_PIN		GPIO_Pin_1	//����ģ���Դ����
#define FINGER_POWER_ON()		GPIO_SetBits(FINGER_POWER_GPIO, FINGER_POWER_PIN)
#define FINGER_POWER_OFF()		GPIO_ResetBits(FINGER_POWER_GPIO, FINGER_POWER_PIN)

#define PS_Sta   PAin(4)//��ָ��ģ��״̬����


#define TRUE  0x01
#define FALSE  0x00

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

#define ACK_SUCCESS     0x00  //�����ɹ�
#define ACK_FAIL        0x01  //����ʧ��
#define ACK_FULL        0x04  //ָ�����ݿ�����
#define ACK_NOUSER      0x05  //�޴��û�
#define ACK_USER_EXIST  0x07  //�û��Ѵ���
#define ACK_TIMEOUT     0x08  //�ɼ���ʱ



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

unsigned char Finger_Search(void);

int Finger_ModeSet(void);


void FingerGPIO_Init(void);


#endif

