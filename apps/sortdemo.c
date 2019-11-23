/*	sortdemo.c for ulios application
	���ߣ�����
	���ܣ����߳�������ʾ����
	����޸����ڣ�2014-09-23
*/

#include "../lib/string.h"
#include "../lib/malloc.h"
#include "../lib/gclient.h"

#define SORTARRAY_LEN	400	/*�������鳤��*/
#define MAX_SORTDATA	128	/*�������ֵ*/
#define SIDE_WIDTH		8	/*ͼ�α߿���*/

typedef struct _SORTDATA
{
	long length;
	DWORD array[SORTARRAY_LEN];
}SORTDATA;

#define DEMO_STATE_IDLE		0	/*����״̬*/
#define DEMO_STATE_SORTING	1	/*��������*/
#define DEMO_STATE_PAUSE	2	/*��ͣ״̬*/

DWORD RandSeed;
SORTDATA Sdata[4];
DWORD DemoState;
THREAD_ID SthPtid[4];
DWORD PtidCount;
DWORD StartClock;

CTRL_WND *wnd = NULL;
CTRL_BTN *RandBtn = NULL;
CTRL_BTN *KillBtn = NULL;
CTRL_BTN *DemoBtn = NULL;

DWORD rand(DWORD x)
{
	RandSeed = RandSeed * 1103515245 + 12345;
	return RandSeed % x;
}

void DrawData(long i, long j, BOOL isWrite)
{
	THREAD_ID ptid;
	DWORD PTID, PPID, data[MSG_DATA_LEN];
	long x, y;

	GCWndGetClientLoca(wnd, &x, &y);
	GCFillRect(&wnd->client, SIDE_WIDTH + i * (MAX_SORTDATA + SIDE_WIDTH) + Sdata[i].array[j], SORTARRAY_LEN + SIDE_WIDTH - 1 - j, MAX_SORTDATA - Sdata[i].array[j], 1, 0xFFA0A0A0);
	GCFillRect(&wnd->client, SIDE_WIDTH + i * (MAX_SORTDATA + SIDE_WIDTH), SORTARRAY_LEN + SIDE_WIDTH - 1 - j, Sdata[i].array[j], 1, isWrite ? 0xFFFF0000 : 0xFF00FF00);
	GUIpaint(wnd->obj.gid, x + SIDE_WIDTH + i * (MAX_SORTDATA + SIDE_WIDTH), y + SORTARRAY_LEN + SIDE_WIDTH - 1 - j, MAX_SORTDATA, 1);
	KGetPtid(&ptid, &PTID, &PPID);
	ptid.ThedID = PTID;
	if (KRecvProcMsg(&ptid, data, 0) == NO_ERROR)	/*���յ���ͣ��Ϣ*/
		KRecvProcMsg(&ptid, data, INVALID);	/*�ȴ����ռ�����Ϣ*/
	GCFillRect(&wnd->client, SIDE_WIDTH + i * (MAX_SORTDATA + SIDE_WIDTH), SORTARRAY_LEN + SIDE_WIDTH - 1 - j, Sdata[i].array[j], 1, 0xFF000000);
	GUIpaint(wnd->obj.gid, x + SIDE_WIDTH + i * (MAX_SORTDATA + SIDE_WIDTH), y + SORTARRAY_LEN + SIDE_WIDTH - 1 - j, MAX_SORTDATA, 1);
}

//ð������
void BsortThread(SORTDATA *data)
{
	long i, j;
	DWORD tmp;
	for (i = data->length - 1; i > 0; i--)
	{
		for (j = 0; j < i; j++)
		{
			DrawData(0, j, FALSE);
			DrawData(0, j + 1, FALSE);
			if (data->array[j] > data->array[j + 1])
			{
				tmp = data->array[j];
				data->array[j] = data->array[j + 1];
				data->array[j + 1] = tmp;
				DrawData(0, j, TRUE);
				DrawData(0, j + 1, TRUE);
			}
		}
	}
	KExitThread(NO_ERROR);
}

//ѡ������
void SsortThread(SORTDATA *data)
{
	long i, j, p;
	DWORD tmp;
	for (i = data->length - 1; i > 0; i--)
	{
		p = i;
		for(j = 0; j < i; j++)
		{
			DrawData(1, j, FALSE);
			DrawData(1, p, FALSE);
			if (data->array[j] > data->array[p])
				p = j;
		}
		if (p != i)
		{
			DrawData(1, p, FALSE);
			DrawData(1, i, FALSE);
			tmp = data->array[p];
			data->array[p] = data->array[i];
			data->array[i] = tmp;
			DrawData(1, p, TRUE);
			DrawData(1, i, TRUE);
		}
	}
	KExitThread(NO_ERROR);
}

//˫��ѡ��
void DsortThread(SORTDATA *data)
{
	long i, j, l, h;
	DWORD tmp;
	for (i = 0; i < data->length / 2; i++)
	{
		l = i;
		h = data->length - i - 1;
		for (j = i; j < data->length - i - 1; j++)
		{
			DrawData(2, j, FALSE);
			DrawData(2, h, FALSE);
			DrawData(2, l, FALSE);
			if (data->array[j] > data->array[h])
				h = j;
			if (data->array[j] < data->array[l])
				l = j;
		}
		if (h != data->length - i - 1)
		{
			DrawData(2, h, FALSE);
			DrawData(2, data->length - i - 1, FALSE);
			tmp = data->array[h];
			data->array[h] = data->array[data->length - i - 1];
			data->array[data->length - i - 1] = tmp;
			DrawData(2, h, TRUE);
			DrawData(2, data->length - i - 1, TRUE);
		}
		if (l != i)
		{
			DrawData(2, l, FALSE);
			DrawData(2, i, FALSE);
			tmp = data->array[l];
			data->array[l] = data->array[i];
			data->array[i] = tmp;
			DrawData(2, l, TRUE);
			DrawData(2, i, TRUE);
		}
	}
	KExitThread(NO_ERROR);
}

//��������
void qsort(DWORD array[], long length)
{
	long i, j;
	DWORD val;

	if (length > 1)	/*ȷ�����鳤�ȴ���1��������������*/
	{
		i = 0;
		j = length - 1;
		DrawData(3, array - Sdata[3].array, FALSE);
		val = array[0];	/*ָ���ο�ֵval��С*/
		do 
		{
			/*�Ӻ���ǰ������valС��Ԫ�أ��ҵ����a[i]�в�����ѭ��*/
			while (i < j && array[j] >= val)
			{
				DrawData(3, array + j - Sdata[3].array, FALSE);
				j--;
			}
			array[i] = array[j];
			DrawData(3, array + i - Sdata[3].array, TRUE);
			/*��ǰ����������val���Ԫ�أ��ҵ����a[j]�в�����ѭ��*/
			while (i < j && array[i] <= val)
			{
				DrawData(3, array + i - Sdata[3].array, FALSE);
				i++;
			}
			array[j] = array[i];
			DrawData(3, array + j - Sdata[3].array, TRUE);
		}
		while (i < j);
		array[i] = val;	/*��������val�е����ŵ�a[i]��*/
		DrawData(3, array + i - Sdata[3].array, TRUE);
		qsort(array, i);	/*�ݹ飬��ǰi��������*/
		qsort(array + i + 1, length - i - 1);	/*��i+1��numsize-1��numsize-1-i��������*/
	}
}

void QsortThread(SORTDATA *data)
{
	qsort(data->array, data->length);
	KExitThread(NO_ERROR);
}

void DrawScene()	/*������ͼ*/
{
	long i, j;

	for (i = 0; i < 4; i++)
	{
		GCFillRect(&wnd->client, SIDE_WIDTH + i * (MAX_SORTDATA + SIDE_WIDTH), SIDE_WIDTH, MAX_SORTDATA, SORTARRAY_LEN, 0xFFA0A0A0);
		for (j = 0; j < SORTARRAY_LEN; j++)
			GCFillRect(&wnd->client, SIDE_WIDTH + i * (MAX_SORTDATA + SIDE_WIDTH), SORTARRAY_LEN + SIDE_WIDTH - 1 - j, Sdata[i].array[j], 1, 0xFF000000);
	}
	GCDrawStr(&wnd->client, MAX_SORTDATA * 0 + SIDE_WIDTH * 1, SORTARRAY_LEN + SIDE_WIDTH * 2, "ð������", 0xFFFF8000);
	GCDrawStr(&wnd->client, MAX_SORTDATA * 1 + SIDE_WIDTH * 2, SORTARRAY_LEN + SIDE_WIDTH * 2, "ѡ������", 0xFFFF8000);
	GCDrawStr(&wnd->client, MAX_SORTDATA * 2 + SIDE_WIDTH * 3, SORTARRAY_LEN + SIDE_WIDTH * 2, "˫��ѡ��", 0xFFFF8000);
	GCDrawStr(&wnd->client, MAX_SORTDATA * 3 + SIDE_WIDTH * 4, SORTARRAY_LEN + SIDE_WIDTH * 2, "��������", 0xFFFF8000);
	GCWndGetClientLoca(wnd, &i, &j);
	GUIpaint(wnd->obj.gid, i + SIDE_WIDTH, j + SIDE_WIDTH, MAX_SORTDATA * 4 + SIDE_WIDTH * 3, SORTARRAY_LEN + SIDE_WIDTH * 2 + GCCharHeight);
}

void RandBtnPressProc(CTRL_BTN *btn)
{
	long i, j;

	RandSeed = ~(DWORD)TMGetRand();
	for (i = 0; i < 4; i++)
	{
		Sdata[i].length = SORTARRAY_LEN;
		for (j = 0; j < SORTARRAY_LEN; j++)
			Sdata[i].array[j] = rand(MAX_SORTDATA) + 1u;
	}
	DrawScene();
	GCBtnSetDisable(DemoBtn, FALSE);
}

void KillBtnPressProc(CTRL_BTN *btn)
{
	KKillThread(SthPtid[0].ThedID);
	KKillThread(SthPtid[1].ThedID);
	KKillThread(SthPtid[2].ThedID);
	KKillThread(SthPtid[3].ThedID);
}

void DemoBtnPressProc(CTRL_BTN *btn)
{
	switch (DemoState)
	{
	case DEMO_STATE_IDLE:	/*תΪ��������*/
		PtidCount = 4;
		KCreateThread((void(*)(void*))BsortThread, 0x4000, Sdata + 0, &SthPtid[0]);
		KCreateThread((void(*)(void*))SsortThread, 0x4000, Sdata + 1, &SthPtid[1]);
		KCreateThread((void(*)(void*))DsortThread, 0x4000, Sdata + 2, &SthPtid[2]);
		KCreateThread((void(*)(void*))QsortThread, 0x4000, Sdata + 3, &SthPtid[3]);
		DemoState = DEMO_STATE_SORTING;
		KGetClock(&StartClock);
		GCBtnSetDisable(RandBtn, TRUE);
		GCBtnSetDisable(KillBtn, FALSE);
		GCBtnSetText(btn, "��ͣ");
		break;
	case DEMO_STATE_SORTING:	/*תΪ��ͣ*/
		{
			DWORD data[MSG_DATA_LEN];
			data[MSG_ATTR_ID] = MSG_ATTR_USER;
			KSendMsg(&SthPtid[0], data, 0);
			KSendMsg(&SthPtid[1], data, 0);
			KSendMsg(&SthPtid[2], data, 0);
			KSendMsg(&SthPtid[3], data, 0);
		}
		DemoState = DEMO_STATE_PAUSE;
		GCBtnSetText(btn, "����");
		break;
	case DEMO_STATE_PAUSE:	/*תΪ����*/
		{
			DWORD data[MSG_DATA_LEN];
			data[MSG_ATTR_ID] = MSG_ATTR_USER;
			KSendMsg(&SthPtid[0], data, 0);
			KSendMsg(&SthPtid[1], data, 0);
			KSendMsg(&SthPtid[2], data, 0);
			KSendMsg(&SthPtid[3], data, 0);
		}
		DemoState = DEMO_STATE_SORTING;
		GCBtnSetText(btn, "��ͣ");
		break;
	}
}

long MainMsgProc(THREAD_ID ptid, DWORD data[MSG_DATA_LEN])
{
	CTRL_WND *wnd = (CTRL_WND*)data[GUIMSG_GOBJ_ID];
	switch (data[MSG_API_ID] & MSG_API_MASK)
	{
	case GM_CREATE:
		{
			long CliX, CliY;
			CTRL_ARGS args;
			GCWndGetClientLoca(wnd, &CliX, &CliY);
			args.width = MAX_SORTDATA;
			args.height = 24;
			args.x = CliX + SIDE_WIDTH;
			args.y = CliY + SORTARRAY_LEN + SIDE_WIDTH * 3 + GCCharHeight;
			args.style = 0;
			args.MsgProc = NULL;
			GCBtnCreate(&RandBtn, &args, wnd->obj.gid, &wnd->obj, "������������", NULL, RandBtnPressProc);
			args.x = CliX + MAX_SORTDATA * 3 + SIDE_WIDTH * 4;
			args.style = BTN_STYLE_DISABLED;
			GCBtnCreate(&KillBtn, &args, wnd->obj.gid, &wnd->obj, "ֹͣ��ʾ�߳�", NULL, KillBtnPressProc);
			args.width = MAX_SORTDATA * 2 + SIDE_WIDTH;
			args.x = CliX + MAX_SORTDATA + SIDE_WIDTH * 2;
			GCBtnCreate(&DemoBtn, &args, wnd->obj.gid, &wnd->obj, "��ʼ������ʾ", NULL, DemoBtnPressProc);
		}
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
	args.width = MAX_SORTDATA * 4 + SIDE_WIDTH * 5 + 2;
	args.height = SORTARRAY_LEN + SIDE_WIDTH * 4 + GCCharHeight + 24 + 21;
	args.x = 80;
	args.y = 40;
	args.style = WND_STYLE_CAPTION | WND_STYLE_BORDER | WND_STYLE_CLOSEBTN;
	args.MsgProc = MainMsgProc;
	GCWndCreate(&wnd, &args, 0, NULL, "ulios2���߳�������ʾ����");

	for (;;)
	{
		THREAD_ID ptid;
		DWORD data[MSG_DATA_LEN];

		if ((res = KRecvMsg(&ptid, data, INVALID)) != NO_ERROR)	/*�ȴ���Ϣ*/
			break;
		if (GCDispatchMsg(ptid, data) == NO_ERROR)	/*����GUI��Ϣ*/
		{
			if ((data[MSG_API_ID] & MSG_API_MASK) == GM_DESTROY && data[GUIMSG_GOBJ_ID] == (DWORD)wnd)	/*����������,�˳�����*/
				break;
		}
		else if ((data[MSG_ATTR_ID] & MSG_ATTR_MASK) == MSG_ATTR_THEDEXIT)
		{
			long i;

			for (i = 0; i < 4; i++)
				if (ptid.ThedID == SthPtid[i].ThedID)
				{
					char buf[32];
					DWORD clk;
					long x, y;
					
					KGetClock(&clk);
					sprintf(buf, ":%uMS", (clk - StartClock) * 10);
					GCFillRect(&wnd->client, SIDE_WIDTH + i * (MAX_SORTDATA + SIDE_WIDTH) + GCCharWidth * 8, SORTARRAY_LEN + SIDE_WIDTH * 2, 14 * GCCharWidth, GCCharHeight, 0xFFC0C0C0);
					GCDrawStr(&wnd->client, SIDE_WIDTH + i * (MAX_SORTDATA + SIDE_WIDTH) + GCCharWidth * 8, SORTARRAY_LEN + SIDE_WIDTH * 2, buf, 0xFF602060);
					GCWndGetClientLoca(wnd, &x, &y);
					GUIpaint(wnd->obj.gid, x + SIDE_WIDTH + i * (MAX_SORTDATA + SIDE_WIDTH) + GCCharWidth * 8, y + SORTARRAY_LEN + SIDE_WIDTH * 2, 14 * GCCharWidth, GCCharHeight);
					break;
				}
			if (--PtidCount == 0)
			{
				DemoState = DEMO_STATE_IDLE;
				GCBtnSetDisable(RandBtn, FALSE);
				GCBtnSetDisable(KillBtn, TRUE);
				GCBtnSetText(DemoBtn, "��ʼ������ʾ");
			}
		}
	}
	return NO_ERROR;
}
