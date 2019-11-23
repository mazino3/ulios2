/*	bsrvapi.h for ulios driver
	���ߣ�����
	���ܣ�ulios��������API�ӿڶ��壬���û���������Ҫ�������ļ�
	����޸����ڣ�2010-03-09
*/

#ifndef	_BSRVAPI_H_
#define	_BSRVAPI_H_

#include "../MkApi/ulimkapi.h"

#define SRV_OUT_TIME	6000	/*������ó�ʱ������INVALID:���޵ȴ�*/

/**********�����쳣����**********/
#define SRV_REP_PORT	0	/*�����쳣�������˿�*/

/**********ATӲ�����**********/
#define SRV_ATHD_PORT	2	/*ATӲ����������˿�*/
#define ATHD_BPS		512	/*����ÿ�����ֽ���*/

#define ATHD_API_WRITESECTOR	0	/*дӲ���������ܺ�*/
#define ATHD_API_READSECTOR		1	/*��Ӳ���������ܺ�*/

#define ATHD_ERR_WRONG_DRV		-512	/*�������������*/
#define ATHD_ERR_HAVENO_REQ		-513	/*�޷����ܸ���ķ�������*/

/*дӲ������*/
static inline long HDWriteSector(DWORD drv, DWORD sec, BYTE cou, void *buf)
{
	THREAD_ID ptid;
	DWORD data[MSG_DATA_LEN];
	ptid.ProcID = SRV_ATHD_PORT;
	ptid.ThedID = INVALID;
	data[MSG_API_ID] = ATHD_API_WRITESECTOR;
	data[3] = drv;
	data[4] = sec;
	if ((data[0] = KWriteProcAddr(buf, ATHD_BPS * cou, &ptid, data, SRV_OUT_TIME)) != NO_ERROR)
		return data[0];
	return data[MSG_RES_ID];
}

/*��Ӳ������*/
static inline long HDReadSector(DWORD drv, DWORD sec, BYTE cou, void *buf)
{
	THREAD_ID ptid;
	DWORD data[MSG_DATA_LEN];
	ptid.ProcID = SRV_ATHD_PORT;
	ptid.ThedID = INVALID;
	data[MSG_API_ID] = ATHD_API_READSECTOR;
	data[3] = drv;
	data[4] = sec;
	if ((data[0] = KReadProcAddr(buf, ATHD_BPS * cou, &ptid, data, SRV_OUT_TIME)) != NO_ERROR)
		return data[0];
	return data[MSG_RES_ID];
}

/**********ʱ��������**********/
#define SRV_TIME_PORT	3	/*ʱ�����˿�*/

#define MSG_ATTR_TIME	0x01030000	/*ʱ�������Ϣ*/

#define TIME_API_CURSECOND		0	/*ȡ��1970�꾭�����빦�ܺ�*/
#define TIME_API_CURTIME		1	/*ȡ�õ�ǰʱ�书�ܺ�*/
#define TIME_API_MKTIME			2	/*TM�ṹת��Ϊ�빦�ܺ�*/
#define TIME_API_LOCALTIME		3	/*��ת��ΪTM�ṹ���ܺ�*/
#define TIME_API_GETRAND		4	/*ȡ����������ܺ�*/

#define TIME_ERR_WRONG_TM		-768	/*�Ƿ�TM�ṹ*/

typedef struct _TM
{
	BYTE sec;	/*��[0,59]*/
	BYTE min;	/*��[0,59]*/
	BYTE hor;	/*ʱ[0,23]*/
	BYTE day;	/*��[1,31]*/
	BYTE mon;	/*��[1,12]*/
	BYTE wday;	/*����[0,6]*/
	WORD yday;	/*һ���еĵڼ���[0,365]*/
	WORD yer;	/*��[1970,...]*/
	WORD mil;	/*����[0,999]*/
}TM;	/*ʱ��ṹ*/

/*ȡ��1970�꾭������*/
static inline long TMCurSecond(DWORD *sec)
{
	THREAD_ID ptid;
	DWORD data[MSG_DATA_LEN];
	ptid.ProcID = SRV_TIME_PORT;
	ptid.ThedID = INVALID;
	data[MSG_API_ID] = MSG_ATTR_TIME | TIME_API_CURSECOND;
	if ((data[0] = KSendMsg(&ptid, data, SRV_OUT_TIME)) != NO_ERROR)
		return data[0];
	*sec = data[1];
	return NO_ERROR;
}

/*ȡ�õ�ǰʱ��*/
static inline long TMCurTime(TM *tm)
{
	THREAD_ID ptid;
	DWORD data[MSG_DATA_LEN];
	ptid.ProcID = SRV_TIME_PORT;
	ptid.ThedID = INVALID;
	data[MSG_API_ID] = MSG_ATTR_TIME | TIME_API_CURTIME;
	if ((data[0] = KSendMsg(&ptid, data, SRV_OUT_TIME)) != NO_ERROR)
		return data[0];
	memcpy32(tm, &data[1], sizeof(TM) / sizeof(DWORD));
	return NO_ERROR;
}

/*TM�ṹת��Ϊ��*/
static inline long TMMkTime(DWORD *sec, const TM *tm)
{
	THREAD_ID ptid;
	DWORD data[MSG_DATA_LEN];
	ptid.ProcID = SRV_TIME_PORT;
	ptid.ThedID = INVALID;
	data[MSG_API_ID] = MSG_ATTR_TIME | TIME_API_MKTIME;
	memcpy32(&data[1], tm, sizeof(TM) / sizeof(DWORD));
	if ((data[0] = KSendMsg(&ptid, data, SRV_OUT_TIME)) != NO_ERROR)
		return data[0];
	if (data[MSG_RES_ID] != NO_ERROR)
		return data[MSG_RES_ID];
	*sec = data[1];
	return NO_ERROR;
}

/*��ת��ΪTM�ṹ*/
static inline long TMLocalTime(DWORD sec, TM *tm)
{
	THREAD_ID ptid;
	DWORD data[MSG_DATA_LEN];
	ptid.ProcID = SRV_TIME_PORT;
	ptid.ThedID = INVALID;
	data[MSG_API_ID] = MSG_ATTR_TIME | TIME_API_LOCALTIME;
	data[1] = sec;
	if ((data[0] = KSendMsg(&ptid, data, SRV_OUT_TIME)) != NO_ERROR)
		return data[0];
	memcpy32(tm, &data[1], sizeof(TM) / sizeof(DWORD));
	return NO_ERROR;
}

/*ȡ�������*/
static inline long TMGetRand()
{
	THREAD_ID ptid;
	DWORD data[MSG_DATA_LEN];
	ptid.ProcID = SRV_TIME_PORT;
	ptid.ThedID = INVALID;
	data[MSG_API_ID] = MSG_ATTR_TIME | TIME_API_GETRAND;
	if ((data[0] = KSendMsg(&ptid, data, SRV_OUT_TIME)) != NO_ERROR)
		return data[0];
	return data[1];
}

/**********�������������**********/
#define SRV_KBDMUS_PORT	4	/*����������˿�*/

#define MSG_ATTR_KBDMUS	0x01040000	/*���������Ϣ*/
#define MSG_ATTR_KBD	0x01040001	/*���̰�����Ϣ*/
#define MSG_ATTR_MUS	0x01040002	/*���״̬��Ϣ*/

#define KBD_STATE_LSHIFT	0x00010000
#define KBD_STATE_RSHIFT	0x00020000
#define KBD_STATE_LCTRL		0x00040000
#define KBD_STATE_RCTRL		0x00080000
#define KBD_STATE_LALT		0x00100000
#define KBD_STATE_RALT		0x00200000
#define KBD_STATE_PAUSE		0x00400000
#define KBD_STATE_PRTSC		0x00800000
#define KBD_STATE_SCRLOCK	0x01000000
#define KBD_STATE_NUMLOCK	0x02000000
#define KBD_STATE_CAPSLOCK	0x04000000
#define KBD_STATE_INSERT	0x08000000

#define MUS_STATE_LBUTTON	0x01
#define MUS_STATE_RBUTTON	0x02
#define MUS_STATE_MBUTTON	0x04
#define MUS_STATE_XSIGN		0x10
#define MUS_STATE_YSIGN		0x20
#define MUS_STATE_XOVRFLW	0x40
#define MUS_STATE_YOVRFLW	0x80

#define KBDMUS_API_SETRECV		0	/*ע����ռ��������Ϣ���̹߳��ܺ�*/

/*ע����ռ��������Ϣ���߳�*/
static inline long KMSetRecv()
{
	THREAD_ID ptid;
	DWORD data[MSG_DATA_LEN];
	ptid.ProcID = SRV_KBDMUS_PORT;
	ptid.ThedID = INVALID;
	data[MSG_API_ID] = MSG_ATTR_KBDMUS | KBDMUS_API_SETRECV;
	return KSendMsg(&ptid, data, 0);
}

/**********VESA�Կ����������GDI�����**********/
#define SRV_VESA_PORT	5	/*VESA�Կ�����˿�*/
#define VESA_MAX_MODE	512	/*��ʾģʽ�б��������*/

#define MSG_ATTR_VESA	0x01050000	/*VESA�Կ���Ϣ*/

#define VESA_API_GETVMEM	0	/*ȡ���Դ�ӳ�书�ܺ�*/
#define VESA_API_GETMODE	1	/*ȡ��ģʽ�б��ܺ�*/

#define VESA_ERR_ARGS		-1280	/*��������*/

/*ȡ���Դ�ӳ��*/
static inline long VSGetVmem(void **vm, DWORD *width, DWORD *height, DWORD *PixBits, DWORD *CurMode)
{
	THREAD_ID ptid;
	DWORD data[MSG_DATA_LEN];
	ptid.ProcID = SRV_VESA_PORT;
	ptid.ThedID = INVALID;
	data[MSG_API_ID] = MSG_ATTR_VESA | VESA_API_GETVMEM;
	if ((data[0] = KSendMsg(&ptid, data, SRV_OUT_TIME)) != NO_ERROR)
		return data[0];
	if (data[MSG_RES_ID] != NO_ERROR)
		return data[MSG_RES_ID];
	*vm = (void*)data[MSG_ADDR_ID];
	*width = data[3];
	*height = data[4];
	*PixBits = data[5];
	*CurMode = data[6];
	return NO_ERROR;
}

/*ȡ��ģʽ�б�*/
static inline long VSGetMode(WORD mode[VESA_MAX_MODE], DWORD *ModeCou, DWORD *CurMode)
{
	THREAD_ID ptid;
	DWORD data[MSG_DATA_LEN];
	ptid.ProcID = SRV_VESA_PORT;
	ptid.ThedID = INVALID;
	data[MSG_API_ID] = VESA_API_GETMODE;
	if ((data[0] = KReadProcAddr(mode, VESA_MAX_MODE * sizeof(WORD), &ptid, data, SRV_OUT_TIME)) != NO_ERROR)
		return data[0];
	if (data[MSG_RES_ID] != NO_ERROR)
		return data[MSG_RES_ID];
	*ModeCou = data[MSG_SIZE_ID];
	*CurMode = data[3];
	return NO_ERROR;
}

/**********��������������**********/
#define SRV_FONT_PORT	6	/*�����������˿�*/

#define MSG_ATTR_FONT	0x01060000	/*�������������Ϣ*/

#define FONT_API_GETFONT	0	/*ȡ������ӳ�书�ܺ�*/

#define FONT_ERR_ARGS		-1536	/*��������*/

/*ȡ������ӳ��*/
static inline long FNTGetFont(const BYTE **font, DWORD *CharWidth, DWORD *CharHeight)
{
	THREAD_ID ptid;
	DWORD data[MSG_DATA_LEN];
	ptid.ProcID = SRV_FONT_PORT;
	ptid.ThedID = INVALID;
	data[MSG_API_ID] = MSG_ATTR_FONT | FONT_API_GETFONT;
	if ((data[0] = KSendMsg(&ptid, data, SRV_OUT_TIME)) != NO_ERROR)
		return data[0];
	if (data[MSG_RES_ID] != NO_ERROR)
		return data[MSG_RES_ID];
	*font = (const BYTE*)data[MSG_ADDR_ID];
	*CharWidth = data[3];
	*CharHeight = data[4];
	return NO_ERROR;
}

/**********CUI�ַ�����������**********/
#define SRV_CUI_PORT	7	/*CUI�ַ��������˿�*/

#define MSG_ATTR_CUI	0x01070000	/*CUI�ַ�������Ϣ*/
#define MSG_ATTR_CUIKEY	0x01070001	/*CUI������Ϣ*/

#define CUI_API_SETRECV	0	/*ע����հ�����Ϣ���̹߳��ܺ�*/
#define CUI_API_GETCOL	1	/*ȡ���ַ�������ɫ���ܺ�*/
#define CUI_API_SETCOL	2	/*�����ַ�������ɫ���ܺ�*/
#define CUI_API_GETCUR	3	/*ȡ�ù��λ�ù��ܺ�*/
#define CUI_API_SETCUR	4	/*���ù��λ�ù��ܺ�*/
#define CUI_API_CLRSCR	5	/*�������ܺ�*/
#define CUI_API_PUTC	6	/*����ַ����ܺ�*/
#define CUI_API_PUTS	7	/*����ַ������ܺ�*/

#define CUI_ERR_ARGS	-1792	/*��������*/

/*ע����հ�����Ϣ���߳�*/
static inline long CUISetRecv()
{
	THREAD_ID ptid;
	DWORD data[MSG_DATA_LEN];
	ptid.ProcID = SRV_CUI_PORT;
	ptid.ThedID = INVALID;
	data[MSG_API_ID] = MSG_ATTR_CUI | CUI_API_SETRECV;
	return KSendMsg(&ptid, data, 0);
}

/*ȡ���ַ�������ɫ*/
static inline long CUIGetCol(DWORD *CharColor, DWORD *BgColor)
{
	THREAD_ID ptid;
	DWORD data[MSG_DATA_LEN];
	ptid.ProcID = SRV_CUI_PORT;
	ptid.ThedID = INVALID;
	data[MSG_API_ID] = MSG_ATTR_CUI | CUI_API_GETCOL;
	if ((data[0] = KSendMsg(&ptid, data, SRV_OUT_TIME)) != NO_ERROR)
		return data[0];
	*CharColor = data[1];
	*BgColor = data[2];
	return NO_ERROR;
}

/*�����ַ�������ɫ*/
static inline long CUISetCol(DWORD CharColor, DWORD BgColor)
{
	THREAD_ID ptid;
	DWORD data[MSG_DATA_LEN];
	ptid.ProcID = SRV_CUI_PORT;
	ptid.ThedID = INVALID;
	data[MSG_API_ID] = MSG_ATTR_CUI | CUI_API_SETCOL;
	data[1] = CharColor;
	data[2] = BgColor;
	return KSendMsg(&ptid, data, 0);
}

/*ȡ�ù��λ��*/
static inline long CUIGetCur(DWORD *CursX, DWORD *CursY)
{
	THREAD_ID ptid;
	DWORD data[MSG_DATA_LEN];
	ptid.ProcID = SRV_CUI_PORT;
	ptid.ThedID = INVALID;
	data[MSG_API_ID] = MSG_ATTR_CUI | CUI_API_GETCUR;
	if ((data[0] = KSendMsg(&ptid, data, SRV_OUT_TIME)) != NO_ERROR)
		return data[0];
	*CursX = data[1];
	*CursY = data[2];
	return NO_ERROR;
}

/*���ù��λ��*/
static inline long CUISetCur(DWORD CursX, DWORD CursY)
{
	THREAD_ID ptid;
	DWORD data[MSG_DATA_LEN];
	ptid.ProcID = SRV_CUI_PORT;
	ptid.ThedID = INVALID;
	data[MSG_API_ID] = MSG_ATTR_CUI | CUI_API_SETCUR;
	data[1] = CursX;
	data[2] = CursY;
	return KSendMsg(&ptid, data, 0);
}

/*����*/
static inline long CUIClrScr()
{
	THREAD_ID ptid;
	DWORD data[MSG_DATA_LEN];
	ptid.ProcID = SRV_CUI_PORT;
	ptid.ThedID = INVALID;
	data[MSG_API_ID] = MSG_ATTR_CUI | CUI_API_CLRSCR;
	return KSendMsg(&ptid, data, 0);
}

/*����ַ�*/
static inline long CUIPutC(char c)
{
	THREAD_ID ptid;
	DWORD data[MSG_DATA_LEN];
	ptid.ProcID = SRV_CUI_PORT;
	ptid.ThedID = INVALID;
	data[MSG_API_ID] = MSG_ATTR_CUI | CUI_API_PUTC;
	data[1] = c;
	return KSendMsg(&ptid, data, 0);
}

/*����ַ���*/
static inline long CUIPutS(const char *str)
{
	THREAD_ID ptid;
	DWORD data[MSG_DATA_LEN];
	ptid.ProcID = SRV_CUI_PORT;
	ptid.ThedID = INVALID;
	data[MSG_API_ID] = CUI_API_PUTS;
	if ((data[0] = KWriteProcAddr((void*)str, strlen(str) + 1, &ptid, data, SRV_OUT_TIME)) != NO_ERROR)
		return data[0];
	return data[MSG_RES_ID];
}

/**********ϵͳ���ȷ������**********/
#define SRV_SPK_PORT	8	/*ϵͳ���ȷ���˿�*/

#define MSG_ATTR_SPK	0x01080000	/*ϵͳ������Ϣ*/

#define SPK_API_SOUND	0	/*�������ܺ�*/
#define SPK_API_NOSOUND	1	/*ֹͣ�������ܺ�*/

/*����*/
static inline long SPKSound(DWORD freq)
{
	THREAD_ID ptid;
	DWORD data[MSG_DATA_LEN];
	ptid.ProcID = SRV_SPK_PORT;
	ptid.ThedID = INVALID;
	data[MSG_API_ID] = MSG_ATTR_SPK | SPK_API_SOUND;
	data[1] = freq;
	return KSendMsg(&ptid, data, 0);
}

/*ֹͣ����*/
static inline long SPKNosound()
{
	THREAD_ID ptid;
	DWORD data[MSG_DATA_LEN];
	ptid.ProcID = SRV_SPK_PORT;
	ptid.ThedID = INVALID;
	data[MSG_API_ID] = MSG_ATTR_SPK | SPK_API_NOSOUND;
	return KSendMsg(&ptid, data, 0);
}

/**********COM���ڷ������**********/
#define SRV_UART_PORT	9	/*COM���ڷ���˿�*/

#define MSG_ATTR_UART	0x01090000	/*COM������Ϣ*/

#define UART_API_OPENCOM	0	/*�򿪴��ڹ��ܺ�*/
#define UART_API_CLOSECOM	1	/*�رմ��ڹ��ܺ�*/
#define UART_API_WRITECOM	2	/*д���ڹ��ܺ�*/
#define UART_API_READCOM	3	/*�����ڹ��ܺ�*/

#define UART_ARGS_BITS_5		0x00	/*5������λ*/
#define UART_ARGS_BITS_6		0x01	/*6������λ*/
#define UART_ARGS_BITS_7		0x02	/*7������λ*/
#define UART_ARGS_BITS_8		0x03	/*8������λ*/
#define UART_ARGS_STOP_1		0x00	/*1��ֹͣλ*/
#define UART_ARGS_STOP_1_5		0x04	/*5������λʱ1.5��ֹͣλ*/
#define UART_ARGS_STOP_2		0x04	/*2��ֹͣλ*/
#define UART_ARGS_PARITY_NONE	0x00	/*����żУ��*/
#define UART_ARGS_PARITY_ODD	0x08	/*��λУ��*/
#define UART_ARGS_PARITY_EVEN	0x18	/*żλУ��*/

#define UART_ERR_NOPORT	-2304	/*COM�˿ڲ�����*/
#define UART_ERR_BAUD	-2305	/*�����ʴ���*/
#define UART_ERR_NOTIME	-2306	/*��ʱ����*/
#define UART_ERR_BUSY	-2307	/*�˿�������æ*/

/*�򿪴���*/
static inline long UARTOpenCom(DWORD com, DWORD baud, DWORD args)
{
	THREAD_ID ptid;
	DWORD data[MSG_DATA_LEN];
	ptid.ProcID = SRV_UART_PORT;
	ptid.ThedID = INVALID;
	data[MSG_API_ID] = MSG_ATTR_UART | UART_API_OPENCOM;
	data[1] = com;
	data[2] = baud;
	data[3] = args;
	if ((data[0] = KSendMsg(&ptid, data, SRV_OUT_TIME)) != NO_ERROR)
		return data[0];
	return data[MSG_RES_ID];
}

/*�رմ���*/
static inline long UARTCloseCom(DWORD com)
{
	THREAD_ID ptid;
	DWORD data[MSG_DATA_LEN];
	ptid.ProcID = SRV_UART_PORT;
	ptid.ThedID = INVALID;
	data[MSG_API_ID] = MSG_ATTR_UART | UART_API_CLOSECOM;
	data[1] = com;
	if ((data[0] = KSendMsg(&ptid, data, SRV_OUT_TIME)) != NO_ERROR)
		return data[0];
	return data[MSG_RES_ID];
}

/*д����*/
static inline long UARTWriteCom(DWORD com, void *buf, DWORD siz, DWORD time)
{
	THREAD_ID ptid;
	DWORD data[MSG_DATA_LEN];
	ptid.ProcID = SRV_UART_PORT;
	ptid.ThedID = INVALID;
	data[MSG_API_ID] = UART_API_WRITECOM;
	data[3] = com;
	data[4] = time;
	if ((data[0] = KWriteProcAddr(buf, siz, &ptid, data, INVALID)) != NO_ERROR)
		return data[0];
	return data[MSG_RES_ID];
}

/*������*/
static inline long UARTReadCom(DWORD com, void *buf, DWORD siz, DWORD time)
{
	THREAD_ID ptid;
	DWORD data[MSG_DATA_LEN];
	ptid.ProcID = SRV_UART_PORT;
	ptid.ThedID = INVALID;
	data[MSG_API_ID] = UART_API_READCOM;
	data[3] = com;
	data[4] = time;
	if ((data[0] = KReadProcAddr(buf, siz, &ptid, data, INVALID)) != NO_ERROR)
		return data[0];
	return data[MSG_RES_ID];
}

/**********���뷨�������**********/
#define SRV_IME_PORT	11	/*���뷨����˿�*/

#define MSG_ATTR_IME	0x010B0000	/*���뷨��Ϣ*/

#define IME_API_OPENBAR		0	/*�����뷨���������ܺ�*/
#define IME_API_CLOSEBAR	1	/*�ر����뷨���������ܺ�*/
#define IME_API_PUTKEY		2	/*�����뷨���Ͱ������ܺ�*/

/*�����뷨������*/
static inline long IMEOpenBar(long x, long y)
{
	THREAD_ID ptid;
	DWORD data[MSG_DATA_LEN];
	ptid.ProcID = SRV_IME_PORT;
	ptid.ThedID = INVALID;
	data[MSG_API_ID] = MSG_ATTR_IME | IME_API_OPENBAR;
	data[1] = x;
	data[2] = y;
	return KSendMsg(&ptid, data, 0);
}

/*�ر����뷨������*/
static inline long IMECloseBar()
{
	THREAD_ID ptid;
	DWORD data[MSG_DATA_LEN];
	ptid.ProcID = SRV_IME_PORT;
	ptid.ThedID = INVALID;
	data[MSG_API_ID] = MSG_ATTR_IME | IME_API_CLOSEBAR;
	return KSendMsg(&ptid, data, 0);
}

/*�����뷨���Ͱ���*/
static inline long IMEPutKey(DWORD key)
{
	THREAD_ID ptid;
	DWORD data[MSG_DATA_LEN];
	ptid.ProcID = SRV_IME_PORT;
	ptid.ThedID = INVALID;
	data[MSG_API_ID] = MSG_ATTR_IME | IME_API_PUTKEY;
	data[1] = key;
	return KSendMsg(&ptid, data, 0);
}

/**********PCI���������������**********/
#define SRV_PCI_PORT	12	/*PCI������������˿�*/

#define MSG_ATTR_PCI	0x010C0000	/*PCI������Ϣ*/

#define PCI_API_0	0	/*0���ܺ�*/
#define PCI_API_1	1	/*1���ܺ�*/

/*0�Ź���*/
static inline long PCIfunc(DWORD freq)
{
	THREAD_ID ptid;
	DWORD data[MSG_DATA_LEN];
	ptid.ProcID = SRV_PCI_PORT;
	ptid.ThedID = INVALID;
	data[MSG_API_ID] = MSG_ATTR_PCI | PCI_API_0;
	data[1] = freq;
	return KSendMsg(&ptid, data, 0);
}

#endif
