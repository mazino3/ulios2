/*	knldef.h for ulios
	���ߣ�����
	���ܣ��ں���ؽṹ�塢�����ֲ�����
	����޸����ڣ�2009-05-26
*/

#ifndef	_KNLDEF_H_
#define	_KNLDEF_H_

#include "x86cpu.h"
#include "error.h"
#include "kalloc.h"
#include "bootdata.h"
#include "cintr.h"
#include "exec.h"
#include "ipc.h"
#include "page.h"
#include "task.h"
#include "debug.h"

/**********�ں����Գ���**********/

#define KCODE_SEL	0x08	/*�ں˴����ѡ����*/
#define KDATA_SEL	0x10	/*�ں����ݶ�ѡ����*/
#define UCODE_SEL	0x1B	/*�û������ѡ����*/
#define UDATA_SEL	0x23	/*�û����ݶ�ѡ����*/
#define TSS_SEL		0x28	/*����״̬��ѡ����*/

#define UADDR_OFF	((void*)0x08000000)	/*�û���ַƫ��*/
#define BASESRV_OFF	((void*)0x08000000)	/*���������ַƫ��*/
#define UFDATA_OFF	((void*)0x40000000)	/*�û��������ݿռ�ƫ��*/
#define UFDATA_SIZ			0x80000000	/*�û��������ݿռ��С*/
#define SHRDLIB_OFF	((void*)0xC0000000)	/*�������ƫ��*/
#define SHRDLIB_SIZ			0x40000000	/*���������С*/

#define BOOTDAT_ADDR		0x00090000	/*�������������ַ*/
#define UPHYMEM_ADDR		0x00800000	/*�û��ڴ���ʼ�����ַ*/

/**********�ں����ݱ�**********/

/*����ģʽ��ر�*/
#define GDT_LEN		0x0100	/*256��*/
extern SEG_GATE_DESC gdt[];	/*ȫ����������2KB*/
#define IDT_LEN		0x0100	/*256��*/
extern SEG_GATE_DESC idt[];	/*�ж���������2KB*/
#define PDT_LEN		0x0400	/*1024��*/
extern PAGE_DESC kpdt[];	/*�ں�ҳĿ¼��4KB*/
extern PAGE_DESC pddt[];	/*ҳĿ¼���Ŀ¼��4KB*/
/*�ں˹����*/
#define FMT_LEN		0x0400	/*1024��*/
extern FREE_BLK_DESC kmmt[];/*�ں��������ݹ����*/
#define PMT_LEN		0x0400	/*1024��*/
extern PROCESS_DESC* pmt[];	/*���̹����4KB*/
#define KPT_LEN		0x0400	/*1024��*/
extern THREAD_ID kpt[];		/*�ں˶˿�ע���4KB*/

/**********�ں���ɢ����**********/

#define KVAL_LEN	0x1000	/*4096��*/
extern BYTE KnlValue[];		/*�ں���ɢ������4KB*/
/*���̹���*/
#define FstPmd		(*((PROCESS_DESC***)(KnlValue + 0x0000)))	/*�׸��ս�����ָ��*/
#define EndPmd		(*((PROCESS_DESC***)(KnlValue + 0x0004)))	/*ĩ���ǿս�����ָ��*/
#define CurPmd		(*((PROCESS_DESC**)	(KnlValue + 0x0008)))	/*��ǰ����ָ��*/
#define PmdCou		(*((DWORD*)			(KnlValue + 0x000C)))	/*���н�������*/
/*�û������ڴ����*/
#define pmmap		(*((DWORD**)		(KnlValue + 0x0010)))	/*����ҳ��ʹ��λͼָ��*/
#define PmmLen		(*((DWORD*)			(KnlValue + 0x0014)))	/*����ҳ�������*/
#define PmpID		(*((DWORD*)			(KnlValue + 0x0018)))	/*�׸�������ҳID*/
#define RemmSiz		(*((DWORD*)			(KnlValue + 0x001C)))	/*ʣ���ڴ��ֽ���*/
/*��Ϣ����*/
#define FstMsg		(*((MESSAGE_DESC**)	(KnlValue + 0x0020)))	/*��Ϣ�����ָ��*/
/*��ַӳ�����*/
#define FstMap		(*((MAPBLK_DESC**)	(KnlValue + 0x0024)))	/*��ַӳ������ָ��*/
/*��ɢ����*/
#define clock		(*((volatile DWORD*)(KnlValue + 0x0028)))	/*ʱ�Ӽ�����*/
#define SleepList	(*((THREAD_DESC**)	(KnlValue + 0x002C)))	/*��ʱ��������ָ��*/
#define LastI387	(*((I387**)			(KnlValue + 0x0030)))	/*���Ƴٱ����Э�������Ĵ���ָ��*/
/*������*/
#define Kmalloc_l	(*((volatile DWORD*)(KnlValue + 0x0034)))	/*kmalloc��*/
#define AllocPage_l	(*((volatile DWORD*)(KnlValue + 0x0038)))	/*AllocPage��*/
#define MapMgr_l	(*((volatile DWORD*)(KnlValue + 0x003C)))	/*ӳ�������*/
/*��ṹ*/
#define IRQ_LEN		0x10	/*IRQ�ź�����16��*/
#define IrqPort		((THREAD_ID*)		(KnlValue + 0x0F58))	/*Irq�˿�ע���*/
#define KnlTss		(*((TSS*)			(KnlValue + 0x0F98)))	/*�ں��̵߳�TSS*/

/**********�ں˴��������**********/

#define KDAT_SIZ	0x00700000	/*��С7M*/
extern BYTE kdat[];		/*�ں���������*/

/**********��ҳ������ر�**********/

#define PDT_ID	2
#define PT_ID	3
#define PT0_ID	4
#define PT2_ID	5
#define PG0_ID	6

#define PT_LEN	0x00100000	/*1048576��*/
#define PG_LEN	0x00400000	/*4194304��*/
extern PAGE_DESC pdt[];		/*���н���ҳĿ¼��4MB*/
extern PAGE_DESC pt[];		/*��ǰ����ҳ��4MB*/
extern PAGE_DESC pt0[];		/*��ǰ���̸���ҳ��4MB*/
extern PAGE_DESC pt2[];		/*��ϵ����ҳ��4MB*/
extern BYTE pg0[];			/*��ǰҳ����4MB*/

/**********���ߺ���**********/

/*ע���ں˶˿ڶ�Ӧ�߳�*/
long RegKnlPort(DWORD PortN);

/*ע���ں˶˿ڶ�Ӧ�߳�*/
long UnregKnlPort(DWORD PortN);

/*ȡ���ں˶˿ڶ�Ӧ�߳�*/
long GetKptThed(DWORD PortN, THREAD_ID *ptid);

/*ע���̵߳������ں˶˿�*/
long UnregAllKnlPort();

/*�ں��ڴ����*/
static inline void *kmalloc(DWORD siz)
{
	return alloc(kmmt, siz);
}

/*�ں��ڴ����*/
static inline void kfree(void *addr, DWORD siz)
{
	free(kmmt, addr, siz);
}

/*�ں��ڴ����(������ʽ)*/
static inline void *LockKmalloc(DWORD siz)
{
	void *res;
	lock(&Kmalloc_l);
	res = kmalloc(siz);
	ulock(&Kmalloc_l);
	return res;
}

/*�ں��ڴ����(������ʽ)*/
static inline void LockKfree(void *addr, DWORD siz)
{
	lock(&Kmalloc_l);
	kfree(addr, siz);
	ulock(&Kmalloc_l);
}

/*��������ҳ(������ʽ)*/
static inline DWORD LockAllocPage()
{
	DWORD res;
	lock(&AllocPage_l);
	res = AllocPage();
	ulock(&AllocPage_l);
	return res;
}

/*��������ҳ(������ʽ)*/
static inline void LockFreePage(DWORD pgaddr)
{
	lock(&AllocPage_l);
	FreePage(pgaddr);
	ulock(&AllocPage_l);
}

#endif
