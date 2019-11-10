/*	vesa.c for ulios driver
	���ߣ�����
	���ܣ�VESA 2.0�Կ������������
	����޸����ڣ�2010-05-19
*/

#include "basesrv.h"

#define FAR2LINE(addr)	((WORD)(addr) + (((addr) & 0xFFFF0000) >> 12))

int main()
{
	DWORD CurMode;	/*��ǰ��ʾģʽ*/
	DWORD width, height;	/*��Ļ��С*/
	DWORD PixBits;	/*ÿ����λ��*/
	DWORD VmPhy;	/*�Դ������ַ*/
	DWORD VmSiz;	/*ӳ���Դ��С*/
	WORD *Modep;
	DWORD ModeCou;	/*ģʽ����*/
	void *vm;		/*�Դ�ӳ���ַ*/
	long res;	/*���ؽ��*/
	WORD ModeList[VESA_MAX_MODE];	/*��ʾģʽ�б�*/

	KDebug("booting... vesa driver\n");
	if ((res = KRegKnlPort(SRV_VESA_PORT)) != NO_ERROR)	/*ע�����˿ں�*/
		return res;
	if ((res = KMapPhyAddr(&vm, 0x90000, 0x70000)) != NO_ERROR)	/*ȡ���Կ���Ϣ*/
		return res;
	CurMode = *((DWORD*)(vm + 0x2FC));
	if (CurMode)	/*ͼ��ģʽ*/
	{
		width = *((WORD*)(vm + 0x512));
		height = *((WORD*)(vm + 0x514));
		PixBits = *((BYTE*)(vm + 0x519));
		VmPhy = *((DWORD*)(vm + 0x528));
	}
	else	/*�ı�ģʽ*/
	{
		width = 80;
		height = 25;
		PixBits = 16;	/*ÿ���ַ�ռ2�ֽ�*/
		VmPhy = 0xB8000;
	}
	Modep = (WORD*)((DWORD)vm + FAR2LINE(*(DWORD*)(vm + 0x30E)) - 0x90000);	/*����ģʽ�б�*/
	for (ModeCou = 0; *Modep != 0xFFFF; ModeCou++, Modep++)
		ModeList[ModeCou] = *Modep;
	KFreeAddr(vm);
	VmSiz = ((PixBits + 7) >> 3) * width * height;
	if ((res = KMapPhyAddr(&vm, VmPhy, VmSiz)) != NO_ERROR)	/*ӳ���Դ�*/
		return res;
	for (;;)
	{
		THREAD_ID ptid;
		DWORD data[MSG_DATA_LEN];

		if ((res = KRecvMsg(&ptid, data, INVALID)) != NO_ERROR)	/*�ȴ���Ϣ*/
			break;
		switch (data[MSG_ATTR_ID] & MSG_ATTR_MASK)
		{
		case MSG_ATTR_VESA:
			switch (data[MSG_API_ID] & MSG_API_MASK)
			{
			case VESA_API_GETVMEM:
				data[MSG_RES_ID] = NO_ERROR;
				data[3] = width;	/*����/�ַ��ֱ���*/
				data[4] = height;
				data[5] = PixBits;	/*ɫ��λ��*/
				data[6] = CurMode;
				KReadProcAddr(vm, VmSiz, &ptid, data, 0);
				break;
			}
			break;
		case MSG_ATTR_ROMAP:
			data[MSG_RES_ID] = VESA_ERR_ARGS;
			KUnmapProcAddr((void*)data[MSG_ADDR_ID], data);
			break;
		case MSG_ATTR_RWMAP:
			if ((data[MSG_API_ID] & MSG_API_MASK) == VESA_API_GETMODE && data[MSG_SIZE_ID] >= (((ModeCou + 1) >> 1) << 2))
			{
				memcpy32((void*)data[MSG_ADDR_ID], ModeList, (ModeCou + 1) >> 1);
				data[MSG_RES_ID] = NO_ERROR;
				data[MSG_SIZE_ID] = ModeCou;
				data[3] = CurMode;
			}
			else
				data[MSG_RES_ID] = VESA_ERR_ARGS;
			KUnmapProcAddr((void*)data[MSG_ADDR_ID], data);
			break;
		}
	}
	KFreeAddr(vm);
	KUnregKnlPort(SRV_VESA_PORT);
	return NO_ERROR;
}
