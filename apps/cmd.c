/*	cmd.c for ulios application
	���ߣ�����
	���ܣ�������ʾ������
	����޸����ڣ�2010-06-14
*/

#include "../lib/string.h"
#include "../driver/basesrv.h"
#include "../fs/fsapi.h"
#include "../gui/guiapi.h"

#define CMD_LEN		256
#define PROMPT		"����:"

char cmd[CMD_LEN], *cmdp;	/*���������*/

/*����*/
void cls(char *args)
{
	CUIClrScr();
}

/*�����ַ���ɫ�ͱ�����ɫ*/
void SetColor(char *args)
{
	char *p;

	for (p = args;; p++)
	{
		if (*p == ' ')
		{
			*(p++) = '\0';
			break;
		}
		if (*p == '\0')
		{
			CUIPutS("����: ǰ��ɫ ����ɫ\n");
			return;
		}
	}
	CUISetCol(atoi16(args), atoi16(p));
}

/*�˳�*/
void exitcmd(char *args)
{
	KExitProcess(NO_ERROR);
}

/*ɾ���ļ�*/
void delfile(char *args)
{
	if (FSremove(args) != NO_ERROR)
		CUIPutS("�޷�ɾ����\n");
}

/*�����ļ�*/
void copy(char *args)
{
	char buf[4096], *bufp;
	long in, out, siz;

	for (bufp = args;; bufp++)
	{
		if (*bufp == ' ')
		{
			*(bufp++) = '\0';
			break;
		}
		if (*bufp == '\0')
		{
			CUIPutS("����: Դ�ļ�·�� Ŀ��·��\n");
			return;
		}
	}
	if ((in = FSopen(args, FS_OPEN_READ)) < 0)
	{
		CUIPutS("Դ�ļ������ڣ�\n");
		return;
	}
	if ((out = FScreat(bufp)) < 0)
	{
		FSclose(in);
		CUIPutS("�޷�����Ŀ���ļ���\n");
		return;
	}
	while ((siz = FSread(in, buf, sizeof(buf))) > 0)
		FSwrite(out, buf, siz);
	FSclose(out);
	FSclose(in);
}

/*������*/
void ren(char *args)
{
	char *p;

	for (p = args;; p++)
	{
		if (*p == ' ')
		{
			*(p++) = '\0';
			break;
		}
		if (*p == '\0')
		{
			CUIPutS("����: Ŀ��·�� ������\n");
			return;
		}
	}
	if (FSrename(args, p) != NO_ERROR)
		CUIPutS("����������\n");
}

/*��ʾ�����б�*/
void partlist(char *args)
{
	struct _PI_INFO
	{
		PART_INFO info;
		char fstype[8];
	}pi;
	DWORD pid;

	pid = 0;
	while (FSEnumPart(&pid) == NO_ERROR)
	{
		THREAD_ID ptid;
		char buf[4096];

		FSGetPart(pid, &pi.info);
		sprintf(buf, "/%u\t����:%uMB\tʣ��:%uMB\t��ʽ:%s\t���:%s\n", pid, (DWORD)(pi.info.size / 0x100000), (DWORD)(pi.info.remain / 0x100000), pi.fstype, pi.info.label);
		CUIPutS(buf);
		ptid.ProcID = SRV_CUI_PORT;
		ptid.ThedID = INVALID;
		if (KRecvProcMsg(&ptid, (DWORD*)buf, 0) == NO_ERROR)
		{
			if (((DWORD*)buf)[MSG_ATTR_ID] == MSG_ATTR_CUIKEY && buf[4] == 27)	/*����ESC��*/
			{
				CUIPutS("�û�ȡ����\n");
				break;
			}
			else if (((DWORD*)buf)[MSG_ATTR_ID] == MSG_ATTR_EXTPROCREQ)	/*�˳�����*/
				KExitProcess(NO_ERROR);
		}
		pid++;
	}
}

/*��ʾĿ¼�б�*/
void dir(char *args)
{
	FILE_INFO fi;
	long dh;

	if ((dh = FSOpenDir(args)) < 0)
	{
		CUIPutS("Ŀ¼�����ڣ�\n");
		return;
	}
	while (FSReadDir(dh, &fi) == NO_ERROR)
	{
		THREAD_ID ptid;
		TM tm;
		char buf[4096];

		TMLocalTime(fi.ModifyTime, &tm);
		sprintf(buf, "%d-%d-%d\t%d:%d:%d   \t%s\t%d\t%c%c%c%c%c%c\t%s\n", tm.yer, tm.mon, tm.day, tm.hor, tm.min, tm.sec, (fi.attr & FILE_ATTR_DIREC) ? "Ŀ¼" : "�ļ�", (DWORD)fi.size, (fi.attr & FILE_ATTR_RDONLY) ? 'R' : ' ', (fi.attr & FILE_ATTR_HIDDEN) ? 'H' : ' ', (fi.attr & FILE_ATTR_SYSTEM) ? 'S' : ' ', (fi.attr & FILE_ATTR_LABEL) ? 'L' : ' ', (fi.attr & FILE_ATTR_ARCH) ? 'A' : ' ', (fi.attr & FILE_ATTR_EXEC) ? 'X' : ' ', fi.name);
		CUIPutS(buf);
		ptid.ProcID = SRV_CUI_PORT;
		ptid.ThedID = INVALID;
		if (KRecvProcMsg(&ptid, (DWORD*)buf, 0) == NO_ERROR)
		{
			if (((DWORD*)buf)[MSG_ATTR_ID] == MSG_ATTR_CUIKEY && buf[4] == 27)	/*����ESC��*/
			{
				CUIPutS("�û�ȡ����\n");
				break;
			}
			else if (((DWORD*)buf)[MSG_ATTR_ID] == MSG_ATTR_EXTPROCREQ)	/*�˳�����*/
				KExitProcess(NO_ERROR);
		}
	}
	FSclose(dh);
}

/*�л�Ŀ¼*/
void cd(char *args)
{
	if (*args)
	{
		if (FSChDir(args) != NO_ERROR)
			CUIPutS("Ŀ¼�����ڣ�\n");
	}
	else
	{
		char path[MAX_PATH];
		long siz;
		if ((siz = FSGetCwd(path, MAX_PATH - 1)) < 0)
			CUIPutS("��ǰ·������\n");
		else
		{
			path[siz - 1] = '\n';
			path[siz] = '\0';
			CUIPutS(path);
		}
	}
}

/*����Ŀ¼*/
void md(char *args)
{
	if (FSMkDir(args) != NO_ERROR)
		CUIPutS("�޷�����Ŀ¼��\n");
}

void show(char *args)
{
	long fh, siz;
	char buf[4097];

	if ((fh = FSopen(args, FS_OPEN_READ)) < 0)
	{
		CUIPutS("�ļ������ڣ�\n");
		return;
	}
	while ((siz = FSread(fh, buf, 4096)) > 0)
	{
		THREAD_ID ptid;

		buf[siz] = '\0';
		CUIPutS(buf);
		ptid.ProcID = SRV_CUI_PORT;
		ptid.ThedID = INVALID;
		if (KRecvProcMsg(&ptid, (DWORD*)buf, 0) == NO_ERROR)
		{
			if (((DWORD*)buf)[MSG_ATTR_ID] == MSG_ATTR_CUIKEY && buf[4] == 27)	/*����ESC��*/
			{
				CUIPutS("�û�ȡ����\n");
				break;
			}
			else if (((DWORD*)buf)[MSG_ATTR_ID] == MSG_ATTR_EXTPROCREQ)	/*�˳�����*/
				KExitProcess(NO_ERROR);
		}
	}
	FSclose(fh);
}

/*��ʾʱ��*/
void showtim(char *args)
{
	static const char *WeekName[7] = {"��", "һ", "��", "��", "��", "��", "��"};
	TM tm;
	char buf[40];
	TMCurTime(&tm);
	sprintf(buf, "����ʱ��:%d��%d��%d�� ����%s %dʱ%d��%d��\n", tm.yer, tm.mon, tm.day, WeekName[tm.wday], tm.hor, tm.min, tm.sec);
	CUIPutS(buf);
}

/*��ʾ�����б�*/
void proclist(char *args)
{
	FILE_INFO fi;
	DWORD pid;

	pid = 0;
	while (FSProcInfo(&pid, &fi) == NO_ERROR)
	{
		THREAD_ID ptid;
		char buf[4096];

		sprintf(buf, "PID:%d\t%s\n", pid, fi.name);
		CUIPutS(buf);
		ptid.ProcID = SRV_CUI_PORT;
		ptid.ThedID = INVALID;
		if (KRecvProcMsg(&ptid, (DWORD*)buf, 0) == NO_ERROR)
		{
			if (((DWORD*)buf)[MSG_ATTR_ID] == MSG_ATTR_CUIKEY && buf[4] == 27)	/*����ESC��*/
			{
				CUIPutS("�û�ȡ����\n");
				break;
			}
			else if (((DWORD*)buf)[MSG_ATTR_ID] == MSG_ATTR_EXTPROCREQ)	/*�˳�����*/
				KExitProcess(NO_ERROR);
		}
		pid++;
	}
}

/*ɱ������*/
void killproc(char *args)
{
	if (KKillProcess(atoi10(args)) != NO_ERROR)
		CUIPutS("ǿ�н�������ʧ�ܣ�\n");
}

/*����GUI������Ӧ�ó���*/
void startgui(char *args)
{
	THREAD_ID ptid;
	
	if (KGetKptThed(SRV_GUI_PORT, &ptid) == NO_ERROR)
	{
		CUIPutS("ͼ�ν�����������\n");
		return;
	}
	ptid.ProcID = SRV_CUI_PORT;
	ptid.ThedID = INVALID;
	SendExitProcReq(ptid);	/*�ر�CUI*/
	KCreateProcess(0, "gui.bin", NULL, &ptid);
	KSleep(5);	/*��ʱ,��ֹ���̼�������ϵ������*/
	KCreateProcess(0, "desktop.bin", NULL, &ptid);
	KSleep(5);
	KCreateProcess(0, "cui.bin", NULL, &ptid);	/*����ͼ��ģʽCUI*/
	KExitProcess(NO_ERROR);
}

/*����*/
void sound(char *args)
{
	if (SPKSound(atoi10(args)) != NO_ERROR)
	{
		CUIPutS("�޷����ӵ�������������\n");
		return;
	}
	KSleep(100);
	SPKNosound();
}

/*����*/
void help(char *args)
{
	CUIPutS(
		"cls:����\n"
		"color rrggbb rrggbb:����ǰ���ͱ���ɫ\n"
		"exit:�˳�\n"
		"part:�����б�\n"
		"del path:ɾ���ļ����Ŀ¼\n"
		"copy SrcPath DestPath:�����ļ�\n"
		"ren path name:������\n"
		"dir DirPath:Ŀ¼�б�\n"
		"cd DirPath:��ʾ���л���ǰĿ¼\n"
		"md DirPath:����Ŀ¼\n"
		"show FilePath:��ʾ�ļ�����\n"
		"time:��ʾ��ǰʱ��\n"
		"ps:�����б�\n"
		"kill ProcID:ǿ�н�������\n"
		"startgui:����ͼ�ν���\n"
		"sound freq:��һ��Ƶ�ʷ���һ��\n"
		"help:����\n"
		"�����ִ���ļ�·�������иó���\n");
}

/*����ƥ��*/
char *cmdcmp(char *str1, char *str2)
{
	while (*str1)
		if (*str1++ != *str2++)
			return NULL;
	if (*str2 == '\0')
		return str2;
	if (*str2 == ' ')
		return ++str2;
	return NULL;
}

void ProcStr(char *str, char **exec, char **args)
{
	(*args) = (*exec) = NULL;
	while (*str == ' ' || *str == '\t')	/*�������ǰ������Ŀո�*/
		str++;
	if (*str == '\0')
		return;
	if (*str == '\"')	/*˫�����ڵĲ�����������*/
	{
		*exec = ++str;
		while (*str != '\0' && *str != '\"')	/*����ƥ���˫����*/
			str++;
	}
	else	/*��ͨ�����ÿո�ָ�*/
	{
		*exec = str;
		while (*str != '\0' && *str != ' ' && *str != '\t')	/*�����ո�*/
			str++;
	}
	if (*str == '\0')
		return;
	*str++ = '\0';
	while (*str == ' ' || *str == '\t')	/*�������ǰ������Ŀո�*/
		str++;
	*args = str;
}

void CmdProc(char *str)
{
	struct
	{
		char *str;
		void (*cmd)(char *args);
	}CMD[] = {
		{"cls", cls},
		{"color", SetColor},
		{"exit", exitcmd},
		{"part", partlist},
		{"del", delfile},
		{"copy", copy},
		{"ren", ren},
		{"dir", dir},
		{"cd", cd},
		{"md", md},
		{"show", show},
		{"time", showtim},
		{"ps", proclist},
		{"kill", killproc},
		{"startgui", startgui},
		{"sound", sound},
		{"help", help}
	};
	long i;
	char *exec, *args;
	THREAD_ID ptid;
	char buf[40];
	for (i = 0; i < sizeof(CMD) / 8; i++)
		if ((args = cmdcmp(CMD[i].str, str)) != NULL)
		{
			CMD[i].cmd(args);	/*�ڲ�����*/
			CUIPutS(PROMPT);
			return;
		}
	ProcStr(str, &exec, &args);
	if (KCreateProcess(0, exec, args, &ptid) != NO_ERROR)
		sprintf(buf, "��Ч��������ִ���ļ�!\n%s", PROMPT);
	else
		sprintf(buf, "����ID: %d\n%s", ptid.ProcID, PROMPT);
	CUIPutS(buf);
}

/*����������Ӧ*/
void KeyProc(char c)
{
	switch (c)
	{
	case '\0':
		return;
	case '\b':
		if (cmdp != cmd)
		{
			cmdp--;	/*ɾ���ַ�*/
			CUIPutC('\b');
		}
		break;
	case '\r':
		CUIPutC('\n');
		*cmdp = '\0';
		CmdProc(cmd);	/*ִ������*/
		cmdp = cmd;
		*cmdp = '\0';
		break;
	default:
		if (cmdp - cmd < CMD_LEN - 1)
		{
			if (c == '\t')
				c = ' ';
			*cmdp++ = c;
			CUIPutC(c);
		}
		break;
	}
}

int main()
{
	THREAD_ID ptid;
	long res;

	if ((res = CUISetRecv()) != NO_ERROR)
		return res;
	CUIPutS(
		"��ӭ����\n"
		"����������������������������������������\n"
		"����������������������������������������\n"
		"����������������������������������������\n"
		"����������������������������������������\n"
		"����������������������������������������\n"
		"����������������������������������������\n"
		"����help����������!\n");
	CUIPutS(PROMPT);
	cmdp = cmd;
	for (;;)
	{
		DWORD data[MSG_DATA_LEN];

		if ((res = KRecvMsg(&ptid, data, INVALID)) != NO_ERROR)
			break;
		if (data[MSG_ATTR_ID] == MSG_ATTR_CUIKEY)	/*������Ϣ*/
			KeyProc(data[1]);
		else if (data[MSG_ATTR_ID] == MSG_ATTR_EXTPROCREQ)
			break;
	}
	return NO_ERROR;
}
