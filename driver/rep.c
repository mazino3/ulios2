/*	rep.c for ulios driver
	���ߣ�����
	���ܣ������쳣�������
	����޸����ڣ�2010-06-16
*/

#include "../lib/string.h"
#include "basesrv.h"

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
			sprintf(buf, "���棺�߳�(pid=%d tid=%d)���ֲ��ɻָ����쳣��ISR=%u��%s��������=0x%X�������ַEIP=0x%X\n", ptid.ProcID, ptid.ThedID, data[1], IsrStr[data[1]], data[2], data[3]);
			CUIPutS(buf);
			break;
		case MSG_ATTR_EXCEP:
			sprintf(buf, "���棺�߳�(pid=%d tid=%d)�������˳�������=%d���쳣��ַ=0x%X�������ַEIP=0x%X\n", ptid.ProcID, ptid.ThedID, data[MSG_RES_ID], data[MSG_ADDR_ID], data[MSG_SIZE_ID]);
			CUIPutS(buf);
			break;
		}
		KSleep(20);
		SPKNosound();
	}
	return NO_ERROR;
}
