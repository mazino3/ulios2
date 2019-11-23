/*	string.h for ulios
	���ߣ�����
	���ܣ��û��ַ�������
	����޸����ڣ�2019-11-23
*/

#ifndef	_STRING_H_
#define	_STRING_H_

#include "../MkApi/ulimkapi.h"

/*˫��ת��Ϊ����*/
char *itoa(char *buf, DWORD n, DWORD r);

/*��ʽ�����*/
void sprintf(char *buf, const char *fmtstr, ...);

/*10�����ַ���ת��Ϊ�޷�������*/
DWORD atoi10(const char *str);

/*16�����ַ���ת��Ϊ�޷�������*/
DWORD atoi16(const char *str);

#endif
