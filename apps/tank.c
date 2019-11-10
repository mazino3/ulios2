/*	tank.c for ulios application
	���ߣ�����
	���ܣ�̹����Ϸ
	����޸����ڣ�2017-06-29
*/

#include "../lib/malloc.h"
#include "../lib/gclient.h"

// ����
#define DIR_UP 0	// ��
#define DIR_DOWN 1  // ��
#define DIR_LEFT 2  // ��
#define DIR_RIGHT 3 // ��

// �ж�
#define ACT_IDLE 15 // ��ֹ
#define ACT_FIRE 16 // ����

// ̹��״̬
#define STATE_INIT 0	// ��ʼ
#define STATE_IDLE 1	// ����
#define STATE_FROZ 2	// ����
#define STATE_DEAD 3	// ����
#define STATE_SUPER 4	// �޵�

// ��ͼ����
#define ATTR_SPACE 0	// �յ�
#define ATTR_ROAD 1		// ��·
#define ATTR_WALL 2		// שǽ
#define ATTR_IRON 3		// ��ǽ
#define ATTR_RIVER 4	// ����
#define ATTR_GRASS 5	// �ݵ�
#define ATTR_HOME 6		// ��Ѩ

#define PRICE_SUPER 0	// �޵�
#define PRICE_ENERGY 1	// ��������
#define PRICE_ARMOR 2	// ���ӻ���
#define PRICE_NUM 3		// ��������
#define PRICE_BOMB 4	// ȫ��������
#define PRICE_HOME 5	// �ӹ̳�

typedef struct // �ڵ�
{
	short x, y;		// λ��
	BYTE clock;		// ��ʱ��
	BYTE dir;		// ����
	BYTE speed;		// �ٶ�
	BYTE energy;	// ����
} SHELL;

typedef struct // ̹��
{
	short x, y;		// λ��
	BYTE clock;		// ��ʱ��
	BYTE dir;		// ����
	BYTE speed;		// �ٶ�
	BYTE armor;		// ����
	BYTE state;		// ״̬
	BYTE action;	// �ж�
	BYTE s_speed;	// �ڵ��ٶ�
	BYTE s_energy;	// �ڵ�����
	SHELL shell;	// �ڵ�
} TANK;

typedef struct // ��ͼ��
{
	BYTE attr; // ����
} MAP;

// ��Ϸ����
#define ALIGN_SIZE 8		// �н�����ߴ�
#define ALIGN_MASK 0xFFF8	// �н�����ģ��
#define BLOCK_WIDTH 16		// ͼ�ο��С
#define MAP_WIDTH 40		// ��ͼ���
#define MAP_HEIGHT 30		// ��ͼ�߶�
#define TANK_NUM 6			// ̹������
#define HOME_X 21			// ��λ��
#define HOME_Y 29
#define INIT_X 19			// ����λ��
#define INIT_Y 29
#define INIT_DIR DIR_UP		// ��������
#define RBT_INIT_Y 0		// �����˳�����λ��

// ʱ�����
#define CLK_INIT 16			// ��ʼ����ʱ��
#define CLK_INIT_SUPER 64	// ��ʼ�޵�״̬ʱ��
#define CLK_SUPER 250		// ��ʼ�޵�״̬ʱ��
#define CLK_DEAD 16			// ����״̬ʱ��

// ȫ������
DWORD *ImageBuf;
TANK *tanks;
MAP *map;
BYTE MyNum, RbtNum;		// �ҷ��͵з�̹��ʣ������
BYTE PriceAttr;			// ��������
short PriceX, PriceY;	// ����λ��
THREAD_ID GLPtid;

BOOL LoadImage()
{
	DWORD width, height;
	if (GCLoadBmp("tank.bmp", NULL, 0, &width, &height) != NO_ERROR)
	{
		CUIPutS("Read tank.bmp error!");
		return FALSE;
	}
	if (width != BLOCK_WIDTH || height != 28 * BLOCK_WIDTH)
	{
		CUIPutS("Image size error!");
		return FALSE;
	}
	if ((ImageBuf = (DWORD *)malloc((32 + 20) * BLOCK_WIDTH * BLOCK_WIDTH * sizeof(DWORD))) == NULL)
	{
		CUIPutS("Out of memory!");
		return FALSE;
	}
	GCLoadBmp("mouse.bmp", ImageBuf, 28 * BLOCK_WIDTH * BLOCK_WIDTH, NULL, NULL);
	memcpy32(&ImageBuf[32 * BLOCK_WIDTH * BLOCK_WIDTH], &ImageBuf[8 * BLOCK_WIDTH * BLOCK_WIDTH], 20 * BLOCK_WIDTH * BLOCK_WIDTH); // ���Ƴ�̹�������ͼƬ
	for (DWORD *SrcImg = ImageBuf; SrcImg < &ImageBuf[8 * BLOCK_WIDTH * BLOCK_WIDTH]; SrcImg += BLOCK_WIDTH * BLOCK_WIDTH)		   // ����ÿ�������̹��ͼƬ
	{
		DWORD *DestImgDown = ImageBuf + 8 * BLOCK_WIDTH * BLOCK_WIDTH;
		DWORD *DestImgLeft = ImageBuf + 16 * BLOCK_WIDTH * BLOCK_WIDTH;
		DWORD *DestImgRight = ImageBuf + 24 * BLOCK_WIDTH * BLOCK_WIDTH;
		for (DWORD y = 0; y < BLOCK_WIDTH; y++)
			for (DWORD x = 0; x < BLOCK_WIDTH; x++)
			{
				DWORD color = SrcImg[x + y * BLOCK_WIDTH];
				DestImgDown[x + (BLOCK_WIDTH - 1 - y) * BLOCK_WIDTH] = color;
				DestImgLeft[y + x * BLOCK_WIDTH] = color;
				DestImgRight[y + (BLOCK_WIDTH - 1 - x) * BLOCK_WIDTH] = color;
			}
	}
	return TRUE;
}

void hua(ui x, ui y, uc n, uc pic[][16][16], uc spic[][4][4])
{
	int i, j;
	uc *p = pic[n % 8];
	x += y * 320;
	switch (n / 8)
	{
	case 1:
		p += 256;
		for (i = 15; i >= 0; i--)
		{
			p -= 16;
			for (j = 0; j < 16; j++)
				if (y = *(p + j))
					pokeb(0xa000, x + j, y);
			x += 320;
		}
		break;
	case 2:
		for (i = 0; i < 16; i++)
		{
			for (j = 0; j < 16; j++)
				if (y = *(p + j * 16 + i))
					pokeb(0xa000, x + j, y);
			x += 320;
		}
		break;
	case 3:
		for (i = 0; i < 16; i++)
		{
			for (j = 0; j < 16; j++)
				if (y = *(p + (15 - j) * 16 + i))
					pokeb(0xa000, x + j, y);
			x += 320;
		}
		break;
	default:
		if (n < 51)
		{
			if (n >= 32)
				p = pic[n - 24];
			for (i = 0; i < 16; i++)
			{
				for (j = 0; j < 16; j++)
					if (y = *(p + j))
						pokeb(0xa000, x + j, y);
				x += 320;
				p += 16;
			}
		}
		else
		{
			p = spic[n - 51];
			for (i = 0; i < 4; i++)
			{
				for (j = 0; j < 4; j++)
					if (y = *(p + j))
						pokeb(0xa000, x + j, y);
				x += 320;
				p += 4;
			}
		}
	}
}

#define KEY_UP 0x4800
#define KEY_LEFT 0x4B00
#define KEY_RIGHT 0x4D00
#define KEY_DOWN 0x5000

long MainMsgProc(THREAD_ID ptid, DWORD data[MSG_DATA_LEN])
{
	CTRL_WND *wnd = (CTRL_WND *)data[GUIMSG_GOBJ_ID];
	switch (data[MSG_API_ID] & MSG_API_MASK)
	{
	case GM_SETFOCUS:
		if (data[1])
			wnd->obj.style |= WND_STATE_FOCUS;
		else
			wnd->obj.style &= (~WND_STATE_FOCUS);
		break;
	case GM_LBUTTONDOWN: /*��갴��*/
		if (!(wnd->obj.style & WND_STATE_FOCUS))
			GUISetFocus(wnd->obj.gid);
		break;
	case GM_KEY: /*������Ϣ*/
		switch (data[1] & 0xFFFF)
		{
		case KEY_LEFT:
			tanks[0].action = DIR_LEFT;
			break;
		case KEY_RIGHT:
			tanks[0].action = DIR_RIGHT;
			break;
		case KEY_UP:
			tanks[0].action = DIR_UP;
			break;
		case KEY_DOWN:
			tanks[0].action = DIR_DOWN;
			break;
		}
		if (data[1] & KBD_STATE_LCTRL)
			tanks[0].action |= ACT_FIRE;
		break;
	}
	return GCWndDefMsgProc(ptid, data);
}

BOOL isPassable(short mapx, short mapy)
{
	BYTE attr = map[mapx + mapy * MAP_WIDTH].attr;
	return attr == ATTR_SPACE || attr == ATTR_ROAD || attr == ATTR_GRASS;
}

void GameLoopThread()
{
	// �������������
	for (TANK *tank = &tanks[1]; tank < &tanks[TANK_NUM]; tank++)
	{
		switch (TMGetRand() % 25)
		{
		case 0:
			tank->action = DIR_UP;
			break;
		case 1:
			tank->action = DIR_DOWN;
			break;
		case 2:
			tank->action = DIR_LEFT;
			break;
		case 3:
			tank->action = DIR_RIGHT;
			break;
		case 4:
			tank->action |= ACT_FIRE;
			break;
		}
	}
	// ̹���ж�
	for (TANK *tank = tanks; tank < &tanks[TANK_NUM]; tank++)
	{
		if (tank->clock) // ����Զ�״̬�Ƿ���
			tank->clock--;
		else // �����л�״̬
		{
			switch (tank->state)
			{
			case STATE_INIT: // ��ʼ�л�Ϊ�޵�
				tank->state = STATE_SUPER;
				tank->clock = CLK_INIT_SUPER;
				break;
			case STATE_FROZ: // ������޵�ʧЧ
			case STATE_SUPER:
				tank->state = STATE_IDLE;
				break;
			case STATE_DEAD:	   // �����󸴻�����
				if (tank == tanks) // �Ƿ����ҷ�
				{
					if (MyNum)
						MyNum--;
				}
				else
				{
					if (RbtNum)
						RbtNum--;
				}
				tank->state = STATE_INIT;
				tank->clock = CLK_INIT;
				break;
			}
		}
		if (tank->state == STATE_IDLE || tank->state == STATE_SUPER) // ���к��޵�״̬���ǿ��ж�״̬
		{
			short mapx, mapy;
			switch (tank->action & ACT_IDLE)
			{
			case DIR_UP: // �����н�
				tank->dir = DIR_UP;
				mapx = (tank->x + BLOCK_WIDTH - 1) / BLOCK_WIDTH;
				mapy = (tank->y - 1) / BLOCK_WIDTH;
				if (isPassable(tank->x / BLOCK_WIDTH, (tank->y - 1) / BLOCK_WIDTH))
				{
					if (isPassable(mapx, mapy))
					{
						tank->y -= tank->speed;
						if (tank->y < 0)
							tank->y = 0;
					}
					else
						tank->x &= ALIGN_MASK;
				}
				else if (isPassable(mapx, mapy))
					tank->x = ((tank->x - 1) & ALIGN_MASK) + ALIGN_SIZE;
				break;
			case DIR_DOWN: // �����н�
				tank->dir = DIR_DOWN;
				mapx = (tank->x + BLOCK_WIDTH - 1) / BLOCK_WIDTH;
				mapy = (tank->y + BLOCK_WIDTH) / BLOCK_WIDTH;
				if (isPassable(tank->x / BLOCK_WIDTH, (tank->y + BLOCK_WIDTH) / BLOCK_WIDTH))
				{
					if (isPassable(mapx, mapy))
					{
						tank->y += tank->speed;
						if (tank->y > (MAP_HEIGHT - 1) * BLOCK_WIDTH)
							tank->y = (MAP_HEIGHT - 1) * BLOCK_WIDTH;
					}
					else
						tank->x &= ALIGN_MASK;
				}
				else if (isPassable(mapx, mapy))
					tank->x = ((tank->x - 1) & ALIGN_MASK) + ALIGN_SIZE;
				break;
			case DIR_LEFT: // �����н�
				tank->dir = DIR_LEFT;
				mapx = (tank->x - 1) / BLOCK_WIDTH;
				mapy = (tank->y + BLOCK_WIDTH - 1) / BLOCK_WIDTH;
				if (isPassable((tank->x - 1) / BLOCK_WIDTH, tank->y / BLOCK_WIDTH))
				{
					if (isPassable(mapx, mapy))
					{
						tank->x -= tank->speed;
						if (tank->x < 0)
							tank->x = 0;
					}
					else
						tank->y &= ALIGN_MASK;
				}
				else if (isPassable(mapx, mapy))
					tank->y = ((tank->y - 1) & ALIGN_MASK) + ALIGN_SIZE;
				break;
			case DIR_RIGHT: // �����н�
				tank->dir = DIR_RIGHT;
				mapx = (tank->x + BLOCK_WIDTH) / BLOCK_WIDTH;
				mapy = (tank->y + BLOCK_WIDTH - 1) / BLOCK_WIDTH;
				if (isPassable((tank->x + BLOCK_WIDTH) / BLOCK_WIDTH, tank->y / BLOCK_WIDTH))
				{
					if (isPassable(mapx, mapy))
					{
						tank->x += tank->speed;
						if (tank->x > (MAP_WIDTH - 1) * BLOCK_WIDTH)
							tank->x = (MAP_WIDTH - 1) * BLOCK_WIDTH;
					}
					else
						tank->y &= ALIGN_MASK;
				}
				else if (isPassable(mapx, mapy))
					tank->y = ((tank->y - 1) & ALIGN_MASK) + ALIGN_SIZE;
				break;
			}
			if ((tank->action & ACT_FIRE) && tank->shell.speed == 0) // ����
			{
				tank->shell.x = tank->x;
				tank->shell.y = tank->y;
				tank->shell.clock = 0;
				tank->shell.dir = tank->dir;
				tank->shell.speed = tank->s_speed;
				tank->shell.energy = tank->s_energy;
			}
		}
		// �������
		if (PriceAttr != 0xFF)
		{
			if (tanks->x > PriceX - BLOCK_WIDTH && tanks->x < PriceX + BLOCK_WIDTH &&
				tanks->y > PriceY - BLOCK_WIDTH && tanks->y < PriceY + BLOCK_WIDTH)
			{
				short mapx, mapy;
				switch (PriceAttr)
				{
				case PRICE_SUPER:
					tanks->state = STATE_SUPER;
					tanks->clock = CLK_SUPER;
					break;
				case PRICE_ENERGY:
					if (tanks->s_speed < 8)
						tanks->s_speed++;
					if (tanks->s_energy < 4)
						tanks->s_energy++;
					break;
				case PRICE_ARMOR:
					if (tanks->speed < 4)
						tanks->speed++;
					if (tanks->armor < 4)
						tanks->armor++;
					break;
				case PRICE_NUM:
					MyNum++;
					break;
				case PRICE_BOMB:
					for (TANK *tank = &tanks[1]; tank < &tanks[TANK_NUM]; tank++)
					{
						tank->state = STATE_DEAD;
						tank->clock = CLK_DEAD;
					}
					RbtNum -= 5;
					if (RbtNum > 250)
						RbtNum = 0;
					break;
				case PRICE_HOME:
					for (mapy = HOME_Y - 1; mapy < HOME_Y + 1; mapy++)
						for (mapx = HOME_X - 1; mapx < HOME_X + 1; mapx++)
							if (mapx >= 0 && mapx < MAP_WIDTH && mapy >= 0 && mapy < MAP_HEIGHT)
								map[mapx + mapy * MAP_WIDTH].attr = ATTR_IRON;
					m[HOME_X + HOME_Y * MAP_WIDTH].attr = ATTR_HOME;
					break;
				}
			}
		}
	}

	KExitThread(NO_ERROR);
}

int main()
{
	THREAD_ID ptid;
	DWORD data[MSG_DATA_LEN];
	CTRL_ARGS args;
	long res;

	if ((res = InitMallocTab(0x1000000)) != NO_ERROR) /*����16MB���ڴ�*/
		return res;
	if (!LoadImage())
		return 1;
	if ((res = GCinit()) != NO_ERROR)
		return res;
	args.width = BLOCK_WIDTH * MAP_WIDTH;
	args.height = BLOCK_WIDTH * MAP_HEIGHT;
	args.x = 200;
	args.y = 100;
	args.style = WND_STYLE_CAPTION | WND_STYLE_BORDER | WND_STYLE_CLOSEBTN;
	args.MsgProc = MainMsgProc;
	GCWndCreate(&wnd, &args, 0, NULL, "̹�˴�ս");

	KCreateThread(GameLoopThread, 0x4000, NULL, &GLPtid);
	for (;;)
	{
		if ((res = KRecvMsg(&ptid, data, INVALID)) != NO_ERROR) /*�ȴ���Ϣ*/
			break;
		if (GCDispatchMsg(ptid, data) == NO_ERROR) /*����GUI��Ϣ*/
		{
			if ((data[MSG_API_ID] & MSG_API_MASK) == GM_DESTROY && data[GUIMSG_GOBJ_ID] == (DWORD)wnd) /*����������,�˳�����*/
				break;
		}
	}
	KKillThread(GLPtid.ThedID);
	return NO_ERROR;
}