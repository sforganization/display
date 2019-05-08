#ifndef __MACHINE_H__
#define __MACHINE_H__

/* ����ͷ�ļ� ----------------------------------------------------------------*/

#include <string.h>
#include <stdio.h>
#include <stm32f10x.h>

/* ���Ͷ��� ------------------------------------------------------------------*/


//����ص���������
typedef  int    (*pfnCallBack)(char cmd, void *data);
//int Finger_Event(char cmd, void *data)

typedef struct 
{
    bool   bEnable;  //״̬���Ƿ�ʹ��
    bool   bFinish;       //�Ƿ����
    u16    u16RetVal;     //���н������ֵ
    vu32   u32StartTime;  //��ʼ����machine ״̬��ʱ��
    vu32   u32LastTime;   //������״̬��ʱ��
    u32    u32RunCnt;
}MachineRun_t;

typedef struct 
{
    u16   u16InterValTime;   //���ʱ�䣬״̬��Ƶ�� ��u32SysTimes - u32LastTime > , ���ھ�ȷ��ʱ����ʱ��������u32���49��Ż������
    pfnCallBack CallBack;   //�ص�����
    u8    cmd;
    vu32  u32Timeout;        //��ʱʱ��
    void *pData;            //����
}MachineCtrl_t;

typedef struct 
{
    MachineRun_t    MachineRun;
    MachineCtrl_t   MachineCtrl;
    bool            bInit; 
    bool            bMutex;
}PollEvent_t;

typedef struct 
{
    u8              u8FingerID;
}FingerData_t;

typedef enum 
{   
   EVENT_FAILED = 0, 
   EVENT_SUCCESS, 
   
   EVENT_TIME_OUT = 0XFE,
   EVENT_RET_DEF = 0XFF, 
}PollEvent_e;


/* �궨�� --------------------------------------------------------------------*/
/*
 */

#endif  //

