/*	font.c for ulios driver
	���ߣ�����
	���ܣ���������������
	����޸����ڣ�2010-05-19
*/

#include "basesrv.h"
#include "../fs/fsapi.h"

#define FONT_FILE	"gb231212.fon"
#define FONT_SIZE	199344

int main()
{
	void *addr;
	long res;	/*���ؽ��*/
	BYTE font[FONT_SIZE];	/*����*/

	KDebug("booting... font driver\n");
	if ((res = KRegKnlPort(SRV_FONT_PORT)) != NO_ERROR)	/*ע�����˿ں�*/
		return res;
	if ((res = KMapPhyAddr(&addr, 0x90280, 0x7C)) != NO_ERROR)	/*ȡ��ϵͳĿ¼*/
		return res;
	if ((res = FSChDir((const char*)addr)) != NO_ERROR)	/*�л���ϵͳĿ¼*/
		return res;
	if ((res = FSopen(FONT_FILE, FS_OPEN_READ)) < 0)	/*�������ļ�*/
		return res;
	if (FSread(res, font, FONT_SIZE) <= 0)	/*��ȡ�����ļ�*/
		return -1;
	FSclose(res);
	KFreeAddr(addr);
	for (;;)
	{
		THREAD_ID ptid;
		DWORD data[MSG_DATA_LEN];

		if ((res = KRecvMsg(&ptid, data, INVALID)) != NO_ERROR)	/*�ȴ���Ϣ*/
			break;
		switch (data[MSG_ATTR_ID] & MSG_ATTR_MASK)
		{
		case MSG_ATTR_FONT:
			switch (data[MSG_API_ID] & MSG_API_MASK)
			{
			case FONT_API_GETFONT:
				data[MSG_RES_ID] = NO_ERROR;
				data[3] = 6;	/*�ַ���С*/
				data[4] = 12;
				KWriteProcAddr(font, FONT_SIZE, &ptid, data, 0);
				break;
			}
			break;
		case MSG_ATTR_ROMAP:
		case MSG_ATTR_RWMAP:
			data[MSG_RES_ID] = FONT_ERR_ARGS;
			KUnmapProcAddr((void*)data[MSG_ADDR_ID], data);
			break;
		}
	}
	KUnregKnlPort(SRV_FONT_PORT);
	return NO_ERROR;
}
