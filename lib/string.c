/*	malloc.c for ulios
	���ߣ�����
	���ܣ��û��ڴ�ѹ���ʵ��
	����޸����ڣ�2009-05-28
*/

#include "string.h"

/*˫��ת��Ϊ����*/
char *itoa(char *buf, DWORD n, DWORD r)
{
	static const char num[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	char *p, *q;

	q = p = buf;
	do
	{
		*p++ = num[n % r];
		n /= r;
	}
	while (n);
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
void sprintf(char *buf, const char *fmtstr, ...)
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
					num = -num;
				}
				buf = itoa(buf, num, 10);
				break;
			case 'u':
				buf = itoa(buf, *((DWORD*)++args), 10);
				break;
			case 'x':
			case 'X':
				buf = itoa(buf, *((DWORD*)++args), 16);
				break;
			case 'o':
				buf = itoa(buf, *((DWORD*)++args), 8);
				break;
			case 'b':
				buf = itoa(buf, *((DWORD*)++args), 2);
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

/*10�����ַ���ת��Ϊ�޷�������*/
DWORD atoi10(const char *str)
{
	DWORD res;

	res = 0;
	for (;;)
	{
		if (*str >= '0' && *str <= '9')
			res = res * 10 + (*str - '0');
		else
			break;
		str++;
	}
	return res;
}

/*16�����ַ���ת��Ϊ�޷�������*/
DWORD atoi16(const char *str)
{
	DWORD res;

	res = 0;
	for (;;)
	{
		if (*str >= '0' && *str <= '9')
			res = res * 16 + (*str - '0');
		else if (*str >= 'A' && *str <= 'F')
			res = res * 16 + (*str - 'A' + 10);
		else if (*str >= 'a' && *str <= 'f')
			res = res * 16 + (*str - 'a' + 10);
		else
			break;
		str++;
	}
	return res;
}
