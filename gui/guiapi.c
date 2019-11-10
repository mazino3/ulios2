/*	guiapi.c for ulios graphical user interface
	���ߣ�����
	���ܣ�ͼ���û�����ӿڣ���ӦӦ�ó��������ִ�з���
	����޸����ڣ�2010-10-05
*/

#include "gui.h"

extern GOBJ_DESC *gobjt;	/*���������������*/
extern GOBJ_DESC *FstGobj;	/*���������������ָ��*/
extern CLIPRECT *ClipRectt;	/*���о��ι����*/
extern CLIPRECT *FstClipRect;	/*���о��ι����ָ��*/
extern DWORD *MusBak;	/*��걳������*/
extern DWORD *MusPic;	/*���ͼ��*/

#define PTID_ID	MSG_DATA_LEN

void ApiGetGinfo(DWORD *argv)
{
	if ((argv[MSG_ATTR_ID] & MSG_MAP_MASK) == MSG_ATTR_ROMAP)
	{
		argv[MSG_RES_ID] = GUI_ERR_WRONG_ARGS;
		KUnmapProcAddr((void*)argv[MSG_ADDR_ID], argv);
		return;
	}
	argv[1] = GDIwidth;
	argv[2] = GDIheight;
	argv[MSG_RES_ID] = NO_ERROR;
	KSendMsg((THREAD_ID*)&argv[PTID_ID], argv, 0);
}

void ApiCreate(DWORD *argv)
{
	GOBJ_DESC *gobj = NULL;

	if (argv[5] < GOBJT_LEN && *(DWORD*)(&(gobj = &gobjt[argv[5]])->ptid) != INVALID)	/*��鸸����ID��Ч,������Ч*/
	{
		if ((argv[MSG_RES_ID] = CreateGobj(gobj, *(THREAD_ID*)&argv[PTID_ID], argv[GUIMSG_GOBJ_ID], (short)argv[3], (short)(argv[3] >> 16), (short)argv[4], (short)(argv[4] >> 16), (argv[MSG_ATTR_ID] & MSG_ATTR_MASK) == MSG_ATTR_GUI ? NULL : (DWORD*)argv[MSG_ADDR_ID], argv[MSG_SIZE_ID] / sizeof(DWORD), &gobj)) == NO_ERROR)
		{
			argv[5] = gobj - gobjt;
			argv[GUIMSG_GOBJ_ID] = gobj->ClientSign;
		}
	}
	else if (*(DWORD*)(&gobjt[0].ptid) == INVALID)	/*gobjt��0��Ϊ��,����������*/
	{
		if ((argv[MSG_RES_ID] = CreateDesktop(*(THREAD_ID*)&argv[PTID_ID], argv[GUIMSG_GOBJ_ID], (argv[MSG_ATTR_ID] & MSG_ATTR_MASK) == MSG_ATTR_GUI ? NULL : (DWORD*)argv[MSG_ADDR_ID], argv[MSG_SIZE_ID] / sizeof(DWORD))) == NO_ERROR)
		{
			argv[5] = 0;
			argv[GUIMSG_GOBJ_ID] = gobjt[0].ClientSign;
		}
	}
	else
		argv[MSG_RES_ID] = GUI_ERR_INVALID_GOBJID;
	if (argv[MSG_RES_ID] != NO_ERROR && (argv[MSG_ATTR_ID] & MSG_MAP_MASK) == MSG_ATTR_ROMAP)
		KUnmapProcAddr((void*)argv[MSG_ADDR_ID], argv);
	else
	{
		argv[MSG_API_ID] = MSG_ATTR_GUI | GM_CREATE;
		KSendMsg((THREAD_ID*)&argv[PTID_ID], argv, 0);
	}
}

void ApiDestroy(DWORD *argv)
{
	GOBJ_DESC *gobj, *ParGobj = NULL;

	if ((argv[MSG_ATTR_ID] & MSG_MAP_MASK) == MSG_ATTR_ROMAP)
	{
		argv[MSG_RES_ID] = GUI_ERR_WRONG_ARGS;
		KUnmapProcAddr((void*)argv[MSG_ADDR_ID], argv);
		return;
	}
	if (argv[GUIMSG_GOBJ_ID] < GOBJT_LEN && ((THREAD_ID*)&argv[PTID_ID])->ProcID == (gobj = &gobjt[argv[GUIMSG_GOBJ_ID]])->ptid.ProcID)	/*��鴰��ID��Ч,������Ч,���̷�������Ĵ���*/
	{
		ParGobj = gobj->par;
		argv[GUIMSG_GOBJ_ID] = gobj->ClientSign;
		if (ParGobj)
			argv[MSG_RES_ID] = DeleteGobj(gobj);
		else	/*gobj��IDΪ0,ɾ��������*/
			argv[MSG_RES_ID] = DeleteDesktop(gobj);
	}
	else
		argv[MSG_RES_ID] = GUI_ERR_INVALID_GOBJID;
	KSendMsg((THREAD_ID*)&argv[PTID_ID], argv, 0);
}

void ApiMove(DWORD *argv)
{
	GOBJ_DESC *gobj;

	if ((argv[MSG_ATTR_ID] & MSG_MAP_MASK) == MSG_ATTR_ROMAP)
	{
		argv[MSG_RES_ID] = GUI_ERR_WRONG_ARGS;
		KUnmapProcAddr((void*)argv[MSG_ADDR_ID], argv);
		return;
	}
	if (argv[GUIMSG_GOBJ_ID] && argv[GUIMSG_GOBJ_ID] < GOBJT_LEN && ((THREAD_ID*)&argv[PTID_ID])->ProcID == (gobj = &gobjt[argv[GUIMSG_GOBJ_ID]])->ptid.ProcID)	/*��鴰��ID��Ϊ0����Ч,������Ч,���̷�������Ĵ���*/
	{
		if ((argv[MSG_RES_ID] = MoveGobj(gobj, argv[1], argv[2])) == NO_ERROR)
			argv[GUIMSG_GOBJ_ID] = gobj->ClientSign;
	}
	else
		argv[MSG_RES_ID] = GUI_ERR_INVALID_GOBJID;
	KSendMsg((THREAD_ID*)&argv[PTID_ID], argv, 0);
}

void ApiSize(DWORD *argv)
{
	GOBJ_DESC *gobj;

	if (argv[GUIMSG_GOBJ_ID] && argv[GUIMSG_GOBJ_ID] < GOBJT_LEN && ((THREAD_ID*)&argv[PTID_ID])->ProcID == (gobj = &gobjt[argv[GUIMSG_GOBJ_ID]])->ptid.ProcID)	/*��鴰��ID��Ϊ0����Ч,������Ч,���̷�������Ĵ���*/
	{
		if ((argv[MSG_RES_ID] = SizeGobj(gobj, (short)argv[3], (short)(argv[3] >> 16), (short)argv[4], (short)(argv[4] >> 16), (argv[MSG_ATTR_ID] & MSG_ATTR_MASK) == MSG_ATTR_GUI ? NULL : (DWORD*)argv[MSG_ADDR_ID], argv[MSG_SIZE_ID] / sizeof(DWORD))) == NO_ERROR)
			argv[GUIMSG_GOBJ_ID] = gobj->ClientSign;
	}
	else
		argv[MSG_RES_ID] = GUI_ERR_INVALID_GOBJID;
	if (argv[MSG_RES_ID] != NO_ERROR && (argv[MSG_ATTR_ID] & MSG_MAP_MASK) == MSG_ATTR_ROMAP)
		KUnmapProcAddr((void*)argv[MSG_ADDR_ID], argv);
	else
	{
		argv[MSG_API_ID] = MSG_ATTR_GUI | GM_SIZE;
		KSendMsg((THREAD_ID*)&argv[PTID_ID], argv, 0);
	}
}

void ApiPaint(DWORD *argv)
{
	GOBJ_DESC *gobj;

	if ((argv[MSG_ATTR_ID] & MSG_MAP_MASK) == MSG_ATTR_ROMAP)
	{
		argv[MSG_RES_ID] = GUI_ERR_WRONG_ARGS;
		KUnmapProcAddr((void*)argv[MSG_ADDR_ID], argv);
		return;
	}
	if (argv[GUIMSG_GOBJ_ID] < GOBJT_LEN && ((THREAD_ID*)&argv[PTID_ID])->ProcID == (gobj = &gobjt[argv[GUIMSG_GOBJ_ID]])->ptid.ProcID)	/*��鴰��ID��Ч,������Ч,���̷�������Ĵ���*/
		PaintGobj(gobj, (short)argv[1], (short)(argv[1] >> 16), (short)argv[2], (short)(argv[2] >> 16));	/*��������Ϣ*/
}

void ApiSetTop(DWORD *argv)
{
	GOBJ_DESC *gobj = NULL;

	if ((argv[MSG_ATTR_ID] & MSG_MAP_MASK) == MSG_ATTR_ROMAP)
	{
		argv[MSG_RES_ID] = GUI_ERR_WRONG_ARGS;
		KUnmapProcAddr((void*)argv[MSG_ADDR_ID], argv);
		return;
	}
	if (argv[GUIMSG_GOBJ_ID] < GOBJT_LEN && ((THREAD_ID*)&argv[PTID_ID])->ProcID == (gobj = &gobjt[argv[GUIMSG_GOBJ_ID]])->ptid.ProcID)	/*��鴰��ID��Ч,������Ч,���̷�������Ĵ���*/
	{
		if ((argv[MSG_RES_ID] = SetTopGobj(gobj)) == NO_ERROR)
		{
			argv[1] = TRUE;
			argv[GUIMSG_GOBJ_ID] = gobj->ClientSign;
		}
	}
	else
		argv[MSG_RES_ID] = GUI_ERR_INVALID_GOBJID;
	KSendMsg((THREAD_ID*)&argv[PTID_ID], argv, 0);
}

void ApiSetFocus(DWORD *argv)
{
	GOBJ_DESC *gobj = NULL;

	if ((argv[MSG_ATTR_ID] & MSG_MAP_MASK) == MSG_ATTR_ROMAP)
	{
		argv[MSG_RES_ID] = GUI_ERR_WRONG_ARGS;
		KUnmapProcAddr((void*)argv[MSG_ADDR_ID], argv);
		return;
	}
	if (argv[GUIMSG_GOBJ_ID] < GOBJT_LEN && ((THREAD_ID*)&argv[PTID_ID])->ProcID == (gobj = &gobjt[argv[GUIMSG_GOBJ_ID]])->ptid.ProcID)	/*��鴰��ID��Ч,������Ч,���̷�������Ĵ���*/
	{
		if ((argv[MSG_RES_ID] = SetFocusGobj(gobj)) == NO_ERROR)
		{
			argv[1] = TRUE;
			argv[GUIMSG_GOBJ_ID] = gobj->ClientSign;
		}
	}
	else
		argv[MSG_RES_ID] = GUI_ERR_INVALID_GOBJID;
	KSendMsg((THREAD_ID*)&argv[PTID_ID], argv, 0);
}

void ApiDrag(DWORD *argv)
{
	GOBJ_DESC *gobj;

	if ((argv[MSG_ATTR_ID] & MSG_MAP_MASK) == MSG_ATTR_ROMAP)
	{
		argv[MSG_RES_ID] = GUI_ERR_WRONG_ARGS;
		KUnmapProcAddr((void*)argv[MSG_ADDR_ID], argv);
		return;
	}
	if (argv[GUIMSG_GOBJ_ID] && argv[GUIMSG_GOBJ_ID] < GOBJT_LEN && ((THREAD_ID*)&argv[PTID_ID])->ProcID == (gobj = &gobjt[argv[GUIMSG_GOBJ_ID]])->ptid.ProcID)	/*��鴰��ID��Ϊ0����Ч,������Ч,���̷�������Ĵ���*/
		DragGobj(gobj, argv[1]);	/*��������Ϣ*/
}

void ApiSetMouse(DWORD *argv)
{
	if ((argv[MSG_ATTR_ID] & MSG_MAP_MASK) != MSG_ATTR_ROMAP)
		return;
	argv[MSG_RES_ID] = SetMousePic((short)argv[3], (short)(argv[3] >> 16), (short)argv[4], (short)(argv[4] >> 16), (DWORD*)argv[MSG_ADDR_ID], argv[MSG_SIZE_ID] / sizeof(DWORD));
	KUnmapProcAddr((void*)argv[MSG_ADDR_ID], argv);
}

/*ϵͳ���ñ�*/
void (*ApiTable[])(DWORD *argv) = {
	ApiGetGinfo, ApiCreate, ApiDestroy, ApiMove, ApiSize, ApiPaint, ApiSetTop, ApiSetFocus, ApiDrag, ApiSetMouse
};

/*��ʼ��GUI,������ɹ������˳�*/
long InitGUI()
{
	long res;
	GOBJ_DESC *gobj;
	CLIPRECT *clip;

	if ((res = KRegKnlPort(SRV_GUI_PORT)) != NO_ERROR)	/*ע�����˿�*/
		return res;
	if ((res = KMapUserAddr((void**)&gobjt, sizeof(GOBJ_DESC) * GOBJT_LEN + sizeof(CLIPRECT) * CLIPRECTT_LEN + sizeof(DWORD) * MUSPIC_MAXLEN * 2)) != NO_ERROR)	/*���봰�������������,���о��ι����,���ͼ�񻺴�*/
		return res;
	if ((res = VSGetVmem(&GDIvm, &GDIwidth, &GDIheight, &GDIPixBits, &GDImode)) != NO_ERROR)	/*��ʼ��GDI*/
		return res;
	if (!GDImode)
		return GUI_ERR_WRONG_VESAMODE;

	for (gobj = FstGobj = gobjt; gobj < &gobjt[GOBJT_LEN - 1]; gobj++)
	{
		*(DWORD*)(&gobj->ptid) = INVALID;
		gobj->nxt = gobj + 1;
	}
	*(DWORD*)(&gobj->ptid) = INVALID;
	gobj->nxt = NULL;
	ClipRectt = (CLIPRECT*)(gobjt + GOBJT_LEN);
	for (clip = FstClipRect = ClipRectt; clip < &FstClipRect[CLIPRECTT_LEN - 1]; clip++)
		clip->nxt = clip + 1;
	clip->nxt = NULL;
	MusBak = (DWORD*)(ClipRectt + CLIPRECTT_LEN);
	MusPic = MusBak + MUSPIC_MAXLEN;

	return NO_ERROR;
}

int main()
{
	long res;

	KDebug("booting... gui server\n");
	if ((res = InitGUI()) != NO_ERROR)
		return res;
	InitKbdMus();
	for (;;)
	{
		DWORD data[MSG_DATA_LEN + 1];

		if ((res = KRecvMsg((THREAD_ID*)&data[PTID_ID], data, INVALID)) != NO_ERROR)	/*�ȴ���Ϣ*/
			break;
		if (data[MSG_ATTR_ID] == MSG_ATTR_KBD)	/*����������Ϣ*/
			KeyboardProc(data[1]);
		else if (data[MSG_ATTR_ID] == MSG_ATTR_MUS)	/*���������Ϣ*/
			MouseProc(data[1], (long)data[2], (long)data[3], (long)data[4]);
		else if ((data[MSG_ATTR_ID] & MSG_ATTR_MASK) == MSG_ATTR_CNLMAP)	/*���ڽ����쳣�˳��Ĵ���*/
		{
			GOBJ_DESC *gobj;

			for (gobj = gobjt; gobj < &gobjt[GOBJT_LEN]; gobj++)
				if (*(DWORD*)(&gobj->ptid) != INVALID && (DWORD)gobj->vbuf == data[MSG_ADDR_ID])	/*������������*/
				{
					data[MSG_API_ID] = MSG_ATTR_GUI | GM_DESTROY;
					data[GUIMSG_GOBJ_ID] = gobj - gobjt;
					break;
				}
		}
		else if ((data[MSG_ATTR_ID] & MSG_MAP_MASK) == MSG_ATTR_ROMAP || (data[MSG_ATTR_ID] & MSG_ATTR_MASK) == MSG_ATTR_GUI)	/*API������Ϣ*/
		{
			if ((data[MSG_API_ID] & MSG_API_MASK) >= sizeof(ApiTable) / sizeof(void*))
			{
				data[MSG_RES_ID] = GUI_ERR_WRONG_ARGS;
				if (data[MSG_ATTR_ID] < MSG_ATTR_USER)
					KUnmapProcAddr((void*)data[MSG_ADDR_ID], data);
				else
					KSendMsg((THREAD_ID*)&data[PTID_ID], data, 0);
			}
			else
				ApiTable[data[MSG_API_ID] & MSG_API_MASK](data);
		}
		else if (data[MSG_ATTR_ID] == MSG_ATTR_EXTPROCREQ)	/*�˳�����*/
			break;
	}
	KUnregKnlPort(SRV_GUI_PORT);
	return NO_ERROR;
}
