/*	cintr.c for ulios
	���ߣ�����
	���ܣ�C�ж�/�쳣/ϵͳ���ô���
	����޸����ڣ�2019-09-01
*/

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "ulidef.h"

#ifdef DEBUG

/*��ʽ�����*/
void KPrint(const char *fmtstr, ...);

#else

#define KPrint(...)

#endif

#endif