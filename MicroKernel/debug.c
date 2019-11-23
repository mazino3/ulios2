/*	debug.c for ulios
	���ߣ�����
	���ܣ��ں˼�������Ϣ���
	����޸����ڣ�2019-09-01
*/

#include "knldef.h"

#ifdef DEBUG

#define VM		((WORD *)0xB8000)
#define WIDTH	80
#define HEIGHT	25
#define COLOR	0x0700

DWORD CursX = 0, CursY = 0;		/*���λ��*/

/*���ù��*/
static void SetTextCurs()
{
	DWORD PortData;

	PortData = CursX + CursY * WIDTH;
	outb(0x3D4, 0xF);
	outb(0x3D5, PortData & 0xFF);
	outb(0x3D4, 0xE);
	outb(0x3D5, (PortData >> 8));
}

/*����ַ�*/
static void KPutC(char c)
{
	switch (c)
	{
	case '\n':
		CursX = WIDTH;
		break;
	case '\t':
		do
			VM[CursX + CursY * WIDTH] = ' ' | COLOR;
		while (++CursX & 7);
		break;
	default:
		VM[CursX + CursY * WIDTH] = (BYTE)c | COLOR;
		CursX++;
		break;
	}
	if (CursX >= WIDTH)	/*��������һ�У��ص�ǰ��*/
	{
		CursX = 0;
		CursY++;
	}
	if (CursY >= HEIGHT)	/*��������һ�У����Ϲ���*/
	{
		CursY--;
		memcpy32(VM, VM + WIDTH, WIDTH * (HEIGHT - 1) / 2);
		memset32(VM + WIDTH * (HEIGHT - 1), COLOR | (COLOR << 16), WIDTH / 2);
	}
}

/*��������ַ�*/
static void KPutNum(DWORD n, DWORD r)
{
	static const char num[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	char buf[16], *bufp;

	bufp = buf;
	do
	{
		*bufp++ = num[n % r];
		n /= r;
	}
	while (n);
	do
		KPutC(*--bufp);
	while (bufp > buf);
}

/*����ַ���*/
static void KPutStr(const char *str)
{
	while(*str)
		KPutC(*str++);
}

/*��ʽ�����*/
void KPrint(const char *fmtstr, ...)
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
					KPutC('-');
					num = -num;
				}
				KPutNum(num, 10);
				break;
			case 'u':
				KPutNum(*((DWORD*)++args), 10);
				break;
			case 'x':
			case 'X':
				KPutNum(*((DWORD*)++args), 16);
				break;
			case 'o':
				KPutNum(*((DWORD*)++args), 8);
				break;
			case 'b':
				KPutNum(*((DWORD*)++args), 2);
				break;
			case 's':
				KPutStr(*((const char**)++args));
				break;
			case 'c':
				KPutC(*((char*)++args));
				break;
			default:
				KPutC(*fmtstr);
			}
		}
		else
			KPutC(*fmtstr);
		fmtstr++;
	}
	SetTextCurs();
}

#endif
