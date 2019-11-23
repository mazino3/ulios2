/*	gmgr.c for ulios application
	���ߣ�����
	���ܣ�ͼ�λ���Դ����������
	����޸����ڣ�2012-02-13
*/

#include "../lib/string.h"
#include "../fs/fsapi.h"
#include "../lib/malloc.h"
#include "../lib/gclient.h"

CTRL_WND *MainWnd;	/*������*/
CTRL_SEDT *DirSedt;	/*��ַ��*/
CTRL_LST *PartList;	/*�����б�*/
CTRL_LST *FileList;	/*�ļ��б�*/
CTRL_BTN *ParBtn;	/*��Ŀ¼��ť*/
CTRL_BTN *CutBtn;	/*���а�ť*/
CTRL_BTN *CopyBtn;	/*���ư�ť*/
CTRL_BTN *PasteBtn;	/*ճ����ť*/
CTRL_BTN *DelBtn;	/*ɾ����ť*/
CTRL_BTN *DirBtn;	/*����Ŀ¼��ť*/
CTRL_BTN *FileBtn;	/*�����ļ���ť*/
CTRL_BTN *RenBtn;	/*��������ť*/
CTRL_SEDT *NameEdt;	/*���Ʊ༭��*/
DWORD op;	/*����*/
char PathBuf[MAX_PATH];	/*���ƻ����·������*/

#define OP_NONE	0	/*��*/
#define OP_CUT	1	/*����*/
#define OP_COPY	2	/*����*/
#define OP_DIR	3	/*����Ŀ¼*/
#define OP_FILE	4	/*�����ļ�*/
#define OP_REN	5	/*������*/

#define WND_WIDTH	400	/*������С���,�߶�*/
#define WND_HEIGHT	300
#define SIDE		2	/*�ؼ��߾�*/
#define EDT_HEIGHT	16	/*��ַ���߶�*/
#define PART_WIDTH	64	/*�����б���*/
#define BTN_WIDTH	56	/*��ť���,�߶�*/
#define BTN_HEIGHT	20

/*����ļ��б�*/
void FillFileList()
{
	FILE_INFO fi;
	long dh;
	LIST_ITEM *item;
	char buf[MAX_PATH];

	if ((dh = FSOpenDir("")) < 0)
		return;
	FSGetCwd(buf, MAX_PATH);
	GCSedtSetText(DirSedt, buf);
	GCLstDelAllItem(FileList);
	item = NULL;
	while (FSReadDir(dh, &fi) == NO_ERROR)
	{
		char buf[MAX_PATH];
		sprintf(buf, "%s %s", (fi.attr & FILE_ATTR_DIREC) ? "[]" : "==", fi.name);
		GCLstInsertItem(FileList, item, buf, &item);
	}
	FSclose(dh);
}

/*��ַ���س�����*/
void DirSedtEnterProc(CTRL_SEDT *edt)
{
	if (FSChDir(edt->text) == NO_ERROR)
		FillFileList();
}

/*�����б�ѡȡ����*/
void PartListSelProc(CTRL_LST *lst)
{
	if (lst->SelItem)	/*ѡ���̷�*/
	{
		if (FSChDir(lst->SelItem->text) == NO_ERROR)
			FillFileList();
	}
}

/*�����б���Ϣ����*/
long PartListMsgProc(THREAD_ID ptid, DWORD data[MSG_DATA_LEN])
{
	CTRL_LST *lst = (CTRL_LST*)data[GUIMSG_GOBJ_ID];
	switch (data[MSG_API_ID] & MSG_API_MASK)
	{
	case GM_CREATE:
		{
			DWORD pid;
			LIST_ITEM *item;

			pid = 0;
			item = NULL;
			while (FSEnumPart(&pid) == NO_ERROR)
			{
				char buf[4];
				sprintf(buf, "/%u", pid);
				GCLstInsertItem(lst, item, buf, &item);
				pid++;
			}
		}
		break;
	}
	return GCLstDefMsgProc(ptid, data);
}

/*�ļ��б�ѡȡ����*/
void FileListSelProc(CTRL_LST *lst)
{
	if (lst->SelItem)	/*ѡ���ļ�*/
	{
		GCBtnSetDisable(CutBtn, FALSE);
		GCBtnSetDisable(CopyBtn, FALSE);
		GCBtnSetDisable(DelBtn, FALSE);
		GCBtnSetDisable(RenBtn, FALSE);
	}
	else
	{
		GCBtnSetDisable(CutBtn, TRUE);
		GCBtnSetDisable(CopyBtn, TRUE);
		GCBtnSetDisable(DelBtn, TRUE);
		GCBtnSetDisable(RenBtn, TRUE);
	}
}

/*�ļ��б���Ϣ����*/
long FileListMsgProc(THREAD_ID ptid, DWORD data[MSG_DATA_LEN])
{
	CTRL_LST *lst = (CTRL_LST*)data[GUIMSG_GOBJ_ID];
	switch (data[MSG_API_ID] & MSG_API_MASK)
	{
	case GM_CREATE:
		FillFileList();
		break;
	case GM_LBUTTONDBCLK:
		if (lst->SelItem)	/*ѡ��Ŀ¼���ļ�*/
		{
			if (lst->SelItem->text[0] == '[')	/*��Ŀ¼*/
			{
				if (FSChDir(lst->SelItem->text + 3) == NO_ERROR)
					FillFileList();
			}
			else	/*ִ�г���*/
			{
				THREAD_ID ptid;
				KCreateProcess(0, lst->SelItem->text + 3, NULL, &ptid);
			}
		}
		break;
	}
	return GCLstDefMsgProc(ptid, data);
}

/*�ϼ�Ŀ¼��ť����*/
void ParBtnPressProc(CTRL_BTN *btn)
{
	if (FSChDir("..") == NO_ERROR)
		FillFileList();
}

/*���а�ť����*/
void CutBtnPressProc(CTRL_BTN *btn)
{
	if (FileList->SelItem)	/*ѡ���ļ�*/
	{
		char buf[MAX_PATH];
		FSGetCwd(buf, MAX_PATH);
		sprintf(PathBuf, "%s/%s", buf, FileList->SelItem->text + 3);
		op = OP_CUT;
		GCBtnSetDisable(PasteBtn, FALSE);
	}
}

/*���ư�ť����*/
void CopyBtnPressProc(CTRL_BTN *btn)
{
	if (FileList->SelItem)	/*ѡ���ļ�*/
	{
		char buf[MAX_PATH];
		FSGetCwd(buf, MAX_PATH);
		sprintf(PathBuf, "%s/%s", buf, FileList->SelItem->text + 3);
		op = OP_COPY;
		GCBtnSetDisable(PasteBtn, FALSE);
	}
}

/*�ݹ�ɾ��Ŀ¼*/
void DelTree(char *path)
{
	if (FSChDir(path) == NO_ERROR)
	{
		FILE_INFO fi;
		long dh;
		if ((dh = FSOpenDir("")) < 0)
		{
			FSChDir("..");
			return;
		}
		while (FSReadDir(dh, &fi) == NO_ERROR)
			if (!(fi.name[0] == '.' && (fi.name[1] == '\0' || (fi.name[1] == '.' && fi.name[2] == '\0'))))	/*�����˫��Ŀ¼������*/
				DelTree(fi.name);
		FSclose(dh);
		FSChDir("..");
	}
	FSremove(path);
}

/*�ݹ鸴��Ŀ¼����ǰĿ¼*/
void CopyTree(char *path)
{
	char *end, *name;
	FILE_INFO fi;
	long inh;

	end = path;	/*����·��������*/
	while (*end)
		end++;
	name = end;
	while (name > path && *name != '/')
		name--;
	if (*name == '/')
		name++;
	if ((inh = FSOpenDir(path)) >= 0)	/*���Դ�Ŀ¼*/
	{
		if (FSMkDir(name) == NO_ERROR)	/*������Ŀ¼*/
		{
			FSChDir(name);
			while (FSReadDir(inh, &fi) == NO_ERROR)
				if (!(fi.name[0] == '.' && (fi.name[1] == '\0' || (fi.name[1] == '.' && fi.name[2] == '\0'))))	/*�����˫��Ŀ¼������*/
				{
					*end = '/';
					strcpy(end + 1, fi.name);
					CopyTree(path);
					*end = '\0';
				}
			FSChDir("..");
		}
		FSclose(inh);
	}
	else if ((inh = FSopen(path, FS_OPEN_READ)) >= 0)	/*���Դ��ļ�*/
	{
		char buf[4096];
		long outh, siz;

		if ((outh = FScreat(name)) >= 0)	/*�������ļ�*/
		{
			while ((siz = FSread(inh, buf, sizeof(buf))) > 0)	/*��������*/
				FSwrite(outh, buf, siz);
			FSclose(outh);
		}
		FSclose(inh);
	}
}

/*ճ����ť����*/
void PasteBtnPressProc(CTRL_BTN *btn)
{
	if (op == OP_CUT || op == OP_COPY)	/*�Ѽ��л����ļ�*/
	{
		CopyTree(PathBuf);
		if (op == OP_CUT)
		{
			char buf[MAX_PATH];
			FSGetCwd(buf, MAX_PATH);
			DelTree(PathBuf);
			FSChDir(buf);
		}
		op = OP_NONE;
		GCBtnSetDisable(PasteBtn, TRUE);
		FillFileList();
	}
}

/*ɾ����ť����*/
void DelBtnPressProc(CTRL_BTN *btn)
{
	if (FileList->SelItem)	/*ѡ���ļ�*/
	{
		DelTree(FileList->SelItem->text + 3);
		FillFileList();
	}
}

/*���������س�����*/
void NameEdtEnterProc(CTRL_SEDT *edt)
{
	switch (op)
	{
	case OP_DIR:
		FSMkDir(edt->text);
		break;
	case OP_FILE:
		FSclose(FScreat(edt->text));
		break;
	case OP_REN:
		FSrename(FileList->SelItem->text + 3, edt->text);
		break;
	}
	GCFillRect(&edt->obj.uda, 0, 0, edt->obj.uda.width, edt->obj.uda.height, 0xCCCCCC);
	GUIdestroy(edt->obj.gid);
	NameEdt = NULL;
	GCBtnSetDisable(DirBtn, FALSE);
	GCBtnSetDisable(FileBtn, FALSE);
	GCBtnSetDisable(RenBtn, FALSE);
	GCBtnSetDisable(ParBtn, FALSE);
	FillFileList();
}

/*�����������Ϣ����*/
long NameEdtMsgProc(THREAD_ID ptid, DWORD data[MSG_DATA_LEN])
{
	CTRL_SEDT *edt = (CTRL_SEDT*)data[GUIMSG_GOBJ_ID];
	switch (data[MSG_API_ID] & MSG_API_MASK)
	{
	case GM_CREATE:
		GUISetFocus(edt->obj.gid);
		break;
	case GM_KEY:
		if ((data[1] & 0xFF) == 27)	/*����ESC*/
		{
			GCFillRect(&edt->obj.uda, 0, 0, edt->obj.uda.width, edt->obj.uda.height, 0xCCCCCC);
			GUIdestroy(edt->obj.gid);
			NameEdt = NULL;
			GCBtnSetDisable(DirBtn, FALSE);
			GCBtnSetDisable(FileBtn, FALSE);
			GCBtnSetDisable(RenBtn, FALSE);
			GCBtnSetDisable(ParBtn, FALSE);
			return NO_ERROR;
		}
		break;
	}
	return GCSedtDefMsgProc(ptid, data);
}

/*�������������*/
void CreateNameEdt(const char *text)
{
	CTRL_ARGS args;

	if (NameEdt == NULL)
	{
		args.width = BTN_WIDTH;
		args.height = EDT_HEIGHT;
		args.x = ParBtn->obj.x;
		args.y = ParBtn->obj.y + BTN_HEIGHT * 8 + SIDE * 8;
		args.style = 0;
		args.MsgProc = NameEdtMsgProc;
		GCSedtCreate(&NameEdt, &args, MainWnd->obj.gid, &MainWnd->obj, text, NameEdtEnterProc);
	}
	GCBtnSetDisable(PasteBtn, TRUE);
	GCBtnSetDisable(DirBtn, TRUE);
	GCBtnSetDisable(FileBtn, TRUE);
	GCBtnSetDisable(RenBtn, TRUE);
}

/*����Ŀ¼��ť����*/
void DirBtnPressProc(CTRL_BTN *btn)
{
	CreateNameEdt(NULL);
	op = OP_DIR;
}

/*�����ļ���ť����*/
void FileBtnPressProc(CTRL_BTN *btn)
{
	CreateNameEdt(NULL);
	op = OP_FILE;
}

/*��������ť����*/
void RenamBtnPressProc(CTRL_BTN *btn)
{
	if (FileList->SelItem)	/*ѡ���ļ�*/
	{
		CreateNameEdt(FileList->SelItem->text + 3);
		GCBtnSetDisable(ParBtn, TRUE);
		op = OP_REN;
	}
}

/*��������Ϣ����*/
long MainMsgProc(THREAD_ID ptid, DWORD data[MSG_DATA_LEN])
{
	CTRL_WND *wnd = (CTRL_WND*)data[GUIMSG_GOBJ_ID];
	switch (data[MSG_API_ID] & MSG_API_MASK)
	{
	case GM_CREATE:
		{
			CTRL_ARGS args;

			GCWndGetClientLoca(wnd, &args.x, &args.y);
			args.width = wnd->client.width - SIDE * 2;
			args.height = EDT_HEIGHT;
			args.x += SIDE;
			args.y += SIDE;
			args.style = 0;
			args.MsgProc = NULL;
			GCSedtCreate(&DirSedt, &args, wnd->obj.gid, &wnd->obj, NULL, DirSedtEnterProc);
			args.width = PART_WIDTH;
			args.height = wnd->client.height - EDT_HEIGHT - SIDE * 3;
			args.y += EDT_HEIGHT + SIDE;
			args.style = 0;
			args.MsgProc = PartListMsgProc;
			GCLstCreate(&PartList, &args, wnd->obj.gid, &wnd->obj, PartListSelProc);
			args.width = wnd->client.width - PART_WIDTH - BTN_WIDTH - SIDE * 4;
			args.x += PART_WIDTH + SIDE;
			args.MsgProc = FileListMsgProc;
			GCLstCreate(&FileList, &args, wnd->obj.gid, &wnd->obj, FileListSelProc);
			args.x += args.width + SIDE;
			args.width = BTN_WIDTH;
			args.height = BTN_HEIGHT;
			args.MsgProc = NULL;
			GCBtnCreate(&ParBtn, &args, wnd->obj.gid, &wnd->obj, "�ϼ�Ŀ¼", NULL, ParBtnPressProc);
			args.y += BTN_HEIGHT + SIDE;
			args.style = BTN_STYLE_DISABLED;
			GCBtnCreate(&CutBtn, &args, wnd->obj.gid, &wnd->obj, "����", NULL, CutBtnPressProc);
			args.y += BTN_HEIGHT + SIDE;
			GCBtnCreate(&CopyBtn, &args, wnd->obj.gid, &wnd->obj, "����", NULL, CopyBtnPressProc);
			args.y += BTN_HEIGHT + SIDE;
			GCBtnCreate(&PasteBtn, &args, wnd->obj.gid, &wnd->obj, "ճ��", NULL, PasteBtnPressProc);
			args.y += BTN_HEIGHT + SIDE;
			GCBtnCreate(&DelBtn, &args, wnd->obj.gid, &wnd->obj, "ɾ��", NULL, DelBtnPressProc);
			args.y += BTN_HEIGHT + SIDE;
			args.style = 0;
			GCBtnCreate(&DirBtn, &args, wnd->obj.gid, &wnd->obj, "����Ŀ¼", NULL, DirBtnPressProc);
			args.y += BTN_HEIGHT + SIDE;
			GCBtnCreate(&FileBtn, &args, wnd->obj.gid, &wnd->obj, "�����ļ�", NULL, FileBtnPressProc);
			args.y += BTN_HEIGHT + SIDE;
			args.style = BTN_STYLE_DISABLED;
			GCBtnCreate(&RenBtn, &args, wnd->obj.gid, &wnd->obj, "������", NULL, RenamBtnPressProc);
		}
		break;
	case GM_SIZE:
		{
			long x, y;
			DWORD width, height;

			GCWndGetClientLoca(wnd, &x, &y);
			width = wnd->client.width - SIDE * 2;
			height = EDT_HEIGHT;
			x += SIDE;
			y += SIDE;
			GCGobjSetSize(&DirSedt->obj, x, y, width, height);
			width = PART_WIDTH;
			height = wnd->client.height - EDT_HEIGHT - SIDE * 3;
			y += EDT_HEIGHT + SIDE;
			GCLstSetSize(PartList, x, y, width, height);
			width = wnd->client.width - PART_WIDTH - BTN_WIDTH - SIDE * 4;
			x += PART_WIDTH + SIDE;
			GCLstSetSize(FileList, x, y, width, height);
			x += width + SIDE;
			GCGobjMove(&ParBtn->obj, x, y);
			y += BTN_HEIGHT + SIDE;
			GCGobjMove(&CutBtn->obj, x, y);
			y += BTN_HEIGHT + SIDE;
			GCGobjMove(&CopyBtn->obj, x, y);
			y += BTN_HEIGHT + SIDE;
			GCGobjMove(&PasteBtn->obj, x, y);
			y += BTN_HEIGHT + SIDE;
			GCGobjMove(&DelBtn->obj, x, y);
			y += BTN_HEIGHT + SIDE;
			GCGobjMove(&DirBtn->obj, x, y);
			y += BTN_HEIGHT + SIDE;
			GCGobjMove(&FileBtn->obj, x, y);
			y += BTN_HEIGHT + SIDE;
			GCGobjMove(&RenBtn->obj, x, y);
		}
		break;
	}
	return GCWndDefMsgProc(ptid, data);
}

int main()
{
	CTRL_ARGS args;
	long res;

	if ((res = InitMallocTab(0x1000000)) != NO_ERROR)	/*����16MB���ڴ�*/
		return res;
	if ((res = GCinit()) != NO_ERROR)
		return res;
	args.width = WND_WIDTH;
	args.height = WND_HEIGHT;
	args.x = (GCwidth - args.width) / 2;	/*����*/
	args.y = (GCheight - args.height) / 2;
	args.style = WND_STYLE_CAPTION | WND_STYLE_BORDER | WND_STYLE_CLOSEBTN | WND_STYLE_MAXBTN | WND_STYLE_MINBTN | WND_STYLE_SIZEBTN;
	args.MsgProc = MainMsgProc;
	GCWndCreate(&MainWnd, &args, 0, NULL, "��Դ������");
	MainWnd->MinWidth = WND_WIDTH;
	MainWnd->MinHeight = WND_HEIGHT;

	for (;;)
	{
		THREAD_ID ptid;
		DWORD data[MSG_DATA_LEN];

		if ((res = KRecvMsg(&ptid, data, INVALID)) != NO_ERROR)	/*�ȴ���Ϣ*/
			break;
		if (GCDispatchMsg(ptid, data) == NO_ERROR)	/*����GUI��Ϣ*/
		{
			if ((data[MSG_API_ID] & MSG_API_MASK) == GM_DESTROY && data[GUIMSG_GOBJ_ID] == (DWORD)MainWnd)	/*����������,�˳�����*/
				break;
		}
	}
	return NO_ERROR;
}
