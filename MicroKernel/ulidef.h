/*	ulidef.h for ulios
	���ߣ�����
	���ܣ�ulios�ں˴�������궨��
	����޸����ڣ�2009-05-26
*/

#ifndef	_ULIDEF_H_
#define	_ULIDEF_H_

#define DEBUG	/*������Լ��ں�ʱ�����˶���*/

typedef unsigned char		BYTE;	/*8λ*/
typedef unsigned short		WORD;	/*16λ*/
typedef unsigned long		DWORD;	/*32λ*/
typedef unsigned long		BOOL;
typedef unsigned long long	QWORD;	/*64λ*/

typedef struct _THREAD_ID
{
	WORD ProcID;
	WORD ThedID;
}THREAD_ID;	/*�����߳�ID*/

#define TRUE	1
#define FALSE	0
#define NULL	((void*)0)
#define INVALID	(~0)

/*32λ�ڴ�����*/
static inline void memset32(void *dest, DWORD d, DWORD n)
{
	register void *_dest;
	register DWORD _n;
	__asm__ __volatile__("cld;rep stosl": "=&D"(_dest), "=&c"(_n): "0"(dest), "a"(d), "1"(n): "flags", "memory");
}

/*32λ�ڴ渴��*/
static inline void memcpy32(void *dest, const void *src, DWORD n)
{
	register void *_dest;
	register const void *_src;
	register DWORD _n;
	__asm__ __volatile__("cld;rep movsl": "=&D"(_dest), "=&S"(_src), "=&c"(_n): "0"(dest), "1"(src), "2"(n): "flags", "memory");
}

/*�ַ�������*/
static inline void strcpy(BYTE *dest, const BYTE *src)
{
	BYTE *_dest;
	const BYTE *_src;
	__asm__ __volatile__
	(
		"cld\n"
		"1:\tlodsb\n"
		"stosb\n"
		"testb %%al, %%al\n"
		"jne 1b"
		: "=&D"(_dest), "=&S"(_src): "0"(dest), "1"(src): "flags", "al", "memory"
	);
}

/*���ж�*/
static inline void cli()
{
	__asm__("cli");
}

/*���ж�*/
static inline void sti()
{
	__asm__("sti");
}

/*�˿�����ֽ�*/
static inline void outb(WORD port, BYTE b)
{
	__asm__ __volatile__("outb %1, %w0":: "d"(port), "a"(b));
}

/*�˿������ֽ�*/
static inline BYTE inb(WORD port)
{
	register BYTE b;
	__asm__ __volatile__("inb %w1, %0": "=a"(b): "d"(port));
	return b;
}

/*���̵���*/
void schedul();

/*����������*/
static inline void lock(volatile DWORD *l)
{
	cli();
	while (*l)
		schedul();
	*l = TRUE;
	sti();
}

/*����������*/
static inline void ulock(volatile DWORD *l)
{
	*l = FALSE;
}

/*WORD����������*/
static inline void lockw(volatile WORD *l)
{
	cli();
	while (*l)
		schedul();
	*l = TRUE;
	sti();
}

/*WORD����������*/
static inline void ulockw(volatile WORD *l)
{
	*l = FALSE;
}

/*��������ֵ*/
static inline void lockset(volatile DWORD *l, DWORD val)
{
	cli();
	while (*l)
		schedul();
	*l = val;
	sti();
}

/*���жϲ���1*/
static inline void cliadd(volatile DWORD *l)
{
	cli();
	(*l)++;
	sti();
}

/*���жϲ���1*/
static inline void clisub(volatile DWORD *l)
{
	cli();
	(*l)--;
	sti();
}

/*��������������ж�*/
#define clilock(l) \
({\
	cli();\
	while (l)\
		schedul();\
})

#endif
