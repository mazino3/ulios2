/*	rep.c for ulios driver
	���ߣ�����
	���ܣ������쳣�������
	����޸����ڣ�2010-06-16
*/

#include "basesrv.h"

/*˫��ת��Ϊ����*/
char *Itoa(char *buf, DWORD n, DWORD r)
{
	static const char num[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	char *p, *q;

	q = p = buf;
	for (;;)
	{
		*p++ = num[n % r];
		n /= r;
		if (n == 0)
			break;
	}
	buf = p;	/*ȷ���ַ���β��*/
	*p-- = '\0';
	while (p > q)	/*��ת�ַ���*/
	{
		char c = *q;
		*q++ = *p;
		*p-- = c;
	}
	return buf;
}

/*��ʽ�����*/
void Sprintf(char *buf, const char *fmtstr, ...)
{
	long num;
	const DWORD *args = (DWORD*)(&fmtstr);

	while (*fmtstr)
	{
		if (*fmtstr == '%')
		{
			fmtstr++;
			switch (*fmtstr)
			{
			case 'd':
				num = *((long*)++args);
				if (num < 0)
				{
					*buf++ = '-';
					buf = Itoa(buf, -num, 10);
				}
				else
					buf = Itoa(buf, num, 10);
				break;
			case 'u':
				buf = Itoa(buf, *((DWORD*)++args), 10);
				break;
			case 'x':
			case 'X':
				buf = Itoa(buf, *((DWORD*)++args), 16);
				break;
			case 'o':
				buf = Itoa(buf, *((DWORD*)++args), 8);
				break;
			case 's':
				buf = strcpy(buf, *((const char**)++args)) - 1;
				break;
			case 'c':
				*buf++ = *((char*)++args);
				break;
			default:
				*buf++ = *fmtstr;
			}
		}
		else
			*buf++ = *fmtstr;
		fmtstr++;
	}
	*buf = '\0';
}

int main()
{
	static const char *IsrStr[20] =
	{
		"���������",
		"�����쳣",
		"���������ж�",
		"���Զϵ�",
		"INTO����",
		"�߽���Խ��",
		"�Ƿ�������",
		"Э������������",
		"˫�ع���",
		"Э��������Խ��",
		"��ЧTSS�쳣",
		"�β�����",
		"��ջ���쳣",
		"ͨ�ñ����쳣",
		"ҳ�쳣",
		"δ֪�쳣",
		"Э�����������������",
		"��������",
		"Machine Check",
		"SIMD�����쳣"
	};
	long res;

	KDebug("booting... rep driver\n");
	if ((res = KRegKnlPort(SRV_REP_PORT)) != NO_ERROR)	/*ע�����˿ں�*/
		return res;
	for (;;)
	{
		THREAD_ID ptid;
		DWORD data[MSG_DATA_LEN];
		char buf[256];

		if ((res = KRecvMsg(&ptid, data, INVALID)) != NO_ERROR)	/*�ȴ���Ϣ*/
			break;
		SPKSound(1000);	/*������������*/
		switch (data[MSG_ATTR_ID] & MSG_ATTR_MASK)
		{
		case MSG_ATTR_ISR:
			Sprintf(buf, "���棺�߳�(pid=%d tid=%d)���ֲ��ɻָ����쳣��ISR=%u��%s��������=0x%X�������ַEIP=0x%X\n", ptid.ProcID, ptid.ThedID, data[1], IsrStr[data[1]], data[2], data[3]);
			CUIPutS(buf);
			break;
		case MSG_ATTR_EXCEP:
			Sprintf(buf, "���棺�߳�(pid=%d tid=%d)�������˳�������=%d���쳣��ַ=0x%X�������ַEIP=0x%X\n", ptid.ProcID, ptid.ThedID, data[MSG_RES_ID], data[MSG_ADDR_ID], data[MSG_SIZE_ID]);
			CUIPutS(buf);
			break;
		}
		KSleep(20);
		SPKNosound();
	}
	return NO_ERROR;
}
