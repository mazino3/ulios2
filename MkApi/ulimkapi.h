/*	ulimkapi.h for ulios program
	���ߣ�����
	���ܣ�ulios΢�ں�API�ӿڶ��壬����Ӧ�ó�����Ҫ�������ļ�
	����޸����ڣ�2009-07-28
*/

#ifndef _ULIMKAPI_H_
#define _ULIMKAPI_H_

#define DEBUG	/*������Լ��ں�ʱ�����˶���*/

/**********��������**********/
typedef unsigned char		BYTE;	/*8λ*/
typedef unsigned short		WORD;	/*16λ*/
typedef unsigned long		DWORD;	/*32λ*/
typedef unsigned long		BOOL;
typedef unsigned long long	QWORD;	/*64λ*/
typedef long long			SQWORD;	/*�з���64λ*/

typedef struct _THREAD_ID
{
	WORD ProcID;
	WORD ThedID;
}THREAD_ID;	/*�����߳�ID*/

/**********����**********/
#define TRUE	1
#define FALSE	0
#define NULL	((void *)0)
#define INVALID	(~0)

/**********�������**********/
#define NO_ERROR					0	/*�޴�*/

/*�ں���Դ����*/
#define KERR_OUT_OF_KNLMEM			-1	/*�ں��ڴ治��*/
#define KERR_OUT_OF_PHYMEM			-2	/*�����ڴ治��*/
#define KERR_OUT_OF_LINEADDR		-3	/*���Ե�ַ�ռ䲻��*/

/*�����̴߳���*/
#define KERR_INVALID_PROCID			-4	/*�Ƿ�����ID*/
#define KERR_PROC_NOT_EXIST			-5	/*���̲�����*/
#define KERR_PROC_NOT_ENOUGH		-6	/*���̹���ṹ����*/
#define KERR_PROC_KILL_SELF			-7	/*ɱ����������*/
#define KERR_INVALID_THEDID			-8	/*�Ƿ��߳�ID*/
#define KERR_THED_NOT_EXIST			-9	/*�̲߳�����*/
#define KERR_THED_NOT_ENOUGH		-10	/*�̹߳���ṹ����*/
#define KERR_THED_KILL_SELF			-11	/*ɱ���߳�����*/

/*�ں�ע������*/
#define KERR_INVALID_KPTNUN			-12	/*�Ƿ��ں˶˿ں�*/
#define KERR_KPT_ALREADY_REGISTERED	-13	/*�ں˶˿��ѱ�ע��*/
#define KERR_KPT_NOT_REGISTERED		-14	/*�ں˶˿�δ��ע��*/
#define KERR_INVALID_IRQNUM			-15	/*�Ƿ�IRQ��*/
#define KERR_IRQ_ALREADY_REGISTERED	-16	/*IRQ�ѱ�ע��*/
#define KERR_IRQ_NOT_REGISTERED		-17	/*IRQδ��ע��*/
#define KERR_CURPROC_NOT_REGISTRANT	-18	/*��ǰ���̲���ע����*/

/*��Ϣ����*/
#define KERR_INVALID_USERMSG_ATTR	-19	/*�Ƿ��û���Ϣ����*/
#define KERR_MSG_NOT_ENOUGH			-20	/*��Ϣ�ṹ����*/
#define KERR_MSG_QUEUE_FULL			-21	/*��Ϣ������*/
#define KERR_MSG_QUEUE_EMPTY		-22	/*��Ϣ���п�*/

/*��ַӳ�����*/
#define KERR_MAPSIZE_IS_ZERO		-23	/*ӳ�䳤��Ϊ0*/
#define KERR_MAPSIZE_TOO_LONG		-24	/*ӳ�䳤��̫��*/
#define KERR_PROC_SELF_MAPED		-25	/*ӳ���������*/
#define KERR_PAGE_ALREADY_MAPED		-26	/*Ŀ�����ҳ���Ѿ���ӳ��*/
#define KERR_ILLEGAL_PHYADDR_MAPED	-27	/*ӳ�䲻����������ַ*/
#define KERR_ADDRARGS_NOT_FOUND		-28	/*��ַ����δ�ҵ�*/

/*����ִ�д���*/
#define KERR_OUT_OF_TIME			-29	/*��ʱ����*/
#define KERR_ACCESS_ILLEGAL_ADDR	-30	/*���ʷǷ���ַ*/
#define KERR_WRITE_RDONLY_ADDR		-31	/*дֻ����ַ*/
#define KERR_THED_EXCEPTION			-32	/*�߳�ִ���쳣*/
#define KERR_THED_KILLED			-33	/*�̱߳�ɱ��*/

/*���ô���*/
#define KERR_INVALID_APINUM			-34	/*�Ƿ�ϵͳ���ú�*/
#define KERR_ARGS_TOO_LONG			-35	/*�����ִ�����*/
#define KERR_INVALID_MEMARGS_ADDR	-36	/*�Ƿ��ڴ������ַ*/
#define KERR_NO_DRIVER_PRIVILEGE	-37	/*û��ִ���������ܵ���Ȩ*/

/**********��Ϣ����**********/
#define MSG_DATA_LEN		8			/*��Ϣ����˫����*/

#define MSG_ATTR_MASK		0xFFFF0000	/*��Ϣ��������������*/
#define MSG_API_MASK		0x0000FFFF	/*����:��Ϣ���ݷ����ܺ�����*/
#define MSG_MAP_MASK		0xFFFE0000	/*ҳӳ����Ϣ��������*/

#define MSG_ATTR_ID			0			/*��Ϣ��������������*/
#define MSG_API_ID			0			/*����:��Ϣ���ݷ����ܺ�����*/
#define MSG_ADDR_ID			1			/*��Ϣ���ݵ�ַ������*/
#define MSG_SIZE_ID			2			/*��Ϣ�����ֽ�������*/
#define MSG_RES_ID			7			/*����:��Ϣ���ݷ��񷵻ؽ������*/

#define MSG_ATTR_ISR		0x00010000	/*Ӳ��������Ϣ*/
#define MSG_ATTR_IRQ		0x00020000	/*Ӳ���ж���Ϣ*/
#define MSG_ATTR_THEDEXIT	0x00030000	/*�߳��˳���Ϣ*/
#define MSG_ATTR_PROCEXIT	0x00040000	/*�����˳���Ϣ*/
#define MSG_ATTR_EXCEP		0x00050000	/*�쳣�˳���Ϣ*/
#define MSG_ATTR_ROMAP		0x00060000	/*ҳֻ��ӳ����Ϣ*/
#define MSG_ATTR_RWMAP		0x00070000	/*ҳ��дӳ����Ϣ*/
#define MSG_ATTR_UNMAP		0x00080000	/*���ҳӳ����Ϣ*/
#define MSG_ATTR_CNLMAP		0x00090000	/*ȡ��ҳӳ����Ϣ*/

#define MSG_ATTR_USER		0x01000000	/*�û��Զ�����Ϣ��Сֵ*/
#define MSG_ATTR_EXTTHEDREQ	0x01000001	/*����:�˳��߳�����*/
#define MSG_ATTR_EXTPROCREQ	0x01000002	/*����:�˳���������*/

#define EXEC_ATTR_DRIVER	0x00000002	/*KCreateProcess��attr����,����һ����������Ȩ�޵Ľ���*/

/**********ϵͳ���ýӿ�**********/

/*ȡ���߳�ID*/
static inline long KGetPtid(THREAD_ID *ptid, DWORD *ParThreadID, DWORD *ParProcessID)
{
	register long res;
	__asm__ __volatile__("int $0xF0": "=a"(res), "=b"(*ptid), "=c"(*ParThreadID), "=d"(*ParProcessID): "0"(0x000000));
	return res;
}

/*�������������*/
static inline long KGiveUp()
{
	register long res;
	__asm__ __volatile__("int $0xF0": "=a"(res): "0"(0x010000));
	return res;
}

/*˯��*/
static inline long KSleep(DWORD CentiSeconds)
{
	register long res;
	__asm__ __volatile__("int $0xF0": "=a"(res): "0"(0x020000), "b"(CentiSeconds));
	return res;
}

/*�����߳�*/
static inline long KCreateThread(void (*ThreadProc)(void *data), DWORD StackSize, void *data, THREAD_ID *ptid)
{
	register long res;
	__asm__ __volatile__("int $0xF0": "=a"(res), "=b"(*ptid): "0"(0x030000), "1"(ThreadProc), "c"(StackSize), "d"(data));
	return res;
}

/*�˳��߳�*/
static inline void KExitThread(long ExitCode)
{
	__asm__ __volatile__("int $0xF0":: "a"(0x040000), "b"(ExitCode));
}

/*ɱ���߳�*/
static inline long KKillThread(DWORD ThreadID)
{
	register long res;
	__asm__ __volatile__("int $0xF0": "=a"(res): "0"(0x050000), "b"(ThreadID));
	return res;
}

/*��������*/
static inline long KCreateProcess(DWORD attr, const char *exec, const char *args, THREAD_ID *ptid)
{
	register long res;
	__asm__ __volatile__("int $0xF0": "=a"(res), "=b"(*ptid): "0"(0x060000), "1"(attr), "D"(exec), "S"(args));
	return res;
}

/*�˳�����*/
static inline void KExitProcess(long ExitCode)
{
	__asm__ __volatile__("int $0xF0":: "a"(0x070000), "b"(ExitCode));
}

/*ɱ������*/
static inline long KKillProcess(DWORD ProcessID)
{
	register long res;
	__asm__ __volatile__("int $0xF0": "=a"(res): "0"(0x080000), "b"(ProcessID));
	return res;
}

/*ע���ں˶˿ڶ�Ӧ�߳�*/
static inline long KRegKnlPort(DWORD KnlPort)
{
	register long res;
	__asm__ __volatile__("int $0xF0": "=a"(res): "0"(0x090000), "b"(KnlPort));
	return res;
}

/*ע���ں˶˿ڶ�Ӧ�߳�*/
static inline long KUnregKnlPort(DWORD KnlPort)
{
	register long res;
	__asm__ __volatile__("int $0xF0": "=a"(res): "0"(0x0A0000), "b"(KnlPort));
	return res;
}

/*ȡ���ں˶˿ڶ�Ӧ�߳�*/
static inline long KGetKptThed(DWORD KnlPort, THREAD_ID *ptid)
{
	register long res;
	__asm__ __volatile__("int $0xF0": "=a"(res), "=b"(*ptid): "0"(0x0B0000), "1"(KnlPort));
	return res;
}

/*ע��IRQ�źŵ���Ӧ�߳�*/
static inline long KRegIrq(DWORD irq)
{
	register long res;
	__asm__ __volatile__("int $0xF0": "=a"(res): "0"(0x0C0000), "b"(irq));
	return res;
}

/*ע��IRQ�źŵ���Ӧ�߳�*/
static inline long KUnregIrq(DWORD irq)
{
	register long res;
	__asm__ __volatile__("int $0xF0": "=a"(res): "0"(0x0D0000), "b"(irq));
	return res;
}

/*������Ϣ*/
static inline long KSendMsg(THREAD_ID *ptid, DWORD data[MSG_DATA_LEN], DWORD CentiSeconds)
{
	register long res;
	__asm__ __volatile__("int $0xF0": "=a"(res), "=b"(*ptid): "0"(0x0E0000), "1"(*ptid), "c"(CentiSeconds), "S"(data): "memory");
	return res;
}

/*������Ϣ*/
static inline long KRecvMsg(THREAD_ID *ptid, DWORD data[MSG_DATA_LEN], DWORD CentiSeconds)
{
	register long res;
	__asm__ __volatile__("int $0xF0": "=a"(res), "=b"(*ptid): "0"(0x0F0000), "c"(CentiSeconds), "S"(data): "memory");
	return res;
}

/*����ָ�����̵���Ϣ*/
static inline long KRecvProcMsg(THREAD_ID *ptid, DWORD data[MSG_DATA_LEN], DWORD CentiSeconds)
{
	register long res;
	__asm__ __volatile__("int $0xF0": "=a"(res), "=b"(*ptid): "0"(0x100000), "1"(*ptid), "c"(CentiSeconds), "S"(data): "memory");
	return res;
}

/*ӳ�������ַ*/
static inline long KMapPhyAddr(void **addr, DWORD PhyAddr, DWORD siz)
{
	register long res;
	__asm__ __volatile__("int $0xF0": "=a"(res), "=S"(*addr): "0"(0x110000), "b"(PhyAddr), "c"(siz): "memory");
	return res;
}

/*ӳ���û���ַ*/
static inline long KMapUserAddr(void **addr, DWORD siz)
{
	register long res;
	__asm__ __volatile__("int $0xF0": "=a"(res), "=S"(*addr): "0"(0x120000), "c"(siz));
	return res;
}

/*�����û���ַ��*/
static inline long KFreeAddr(void *addr)
{
	register long res;
	__asm__ __volatile__("int $0xF0": "=a"(res): "0"(0x130000), "S"(addr): "memory");
	return res;
}

/*ӳ����̵�ַд��*/
static inline long KWriteProcAddr(const void *addr, DWORD siz, THREAD_ID *ptid, DWORD data[MSG_DATA_LEN], DWORD CentiSeconds)
{
	register long res;
	__asm__ __volatile__("int $0xF0": "=a"(res), "=b"(*ptid): "0"(0x140000), "1"(*ptid), "c"(siz), "d"(CentiSeconds), "S"(data), "D"(addr): "memory");
	return res;
}

/*ӳ����̵�ַ��ȡ*/
static inline long KReadProcAddr(void *addr, DWORD siz, THREAD_ID *ptid, DWORD data[MSG_DATA_LEN], DWORD CentiSeconds)
{
	register long res;
	__asm__ __volatile__("int $0xF0": "=a"(res), "=b"(*ptid): "0"(0x150000), "1"(*ptid), "c"(siz), "d"(CentiSeconds), "S"(data), "D"(addr): "memory");
	return res;
}

/*����ӳ����̵�ַ*/
static inline long KUnmapProcAddr(void *addr, const DWORD data[MSG_DATA_LEN])
{
	register long res;
	__asm__ __volatile__("int $0xF0": "=a"(res): "0"(0x160000), "S"(data), "D"(addr): "memory");
	return res;
}

/*ȡ��ӳ����̵�ַ*/
static inline long KCnlmapProcAddr(void *addr, const DWORD data[MSG_DATA_LEN])
{
	register long res;
	__asm__ __volatile__("int $0xF0": "=a"(res): "0"(0x170000), "S"(data), "D"(addr): "memory");
	return res;
}

/*ȡ�ÿ���������ʱ��*/
static inline long KGetClock(DWORD *clock)
{
	register long res;
	__asm__ __volatile__("int $0xF0": "=a"(res), "=b"(*clock): "0"(0x180000));
	return res;
}

/*�߳�ͬ��������*/
static inline long KLock(volatile DWORD *addr, DWORD val, DWORD CentiSeconds)
{
	register long res;
	__asm__ __volatile__("int $0xF0": "=a"(res): "0"(0x190000), "b"(CentiSeconds), "c"(val), "S"(addr): "memory");
	return res;
}

/*�߳�ͬ����������*/
static inline void KUlock(volatile DWORD *addr)
{
	*addr = FALSE;
}

#ifdef DEBUG

/*�ں˼�����*/
static inline long KDebug(const char *addr)
{
	register long res;
	__asm__ __volatile__("int $0xF0": "=a"(res): "0"(0x1A0000), "S"(addr));
	return res;
}

#else
#define KDebug(...)
#endif

/**********ͨ�ò���**********/

/*�ڴ�����*/
static inline void memset8(void *dest, BYTE b, DWORD n)
{
	void *_dest;
	DWORD _n;
	__asm__ __volatile__("cld;rep stosb": "=&D"(_dest), "=&c"(_n): "0"(dest), "a"(b), "1"(n): "flags", "memory");
}

/*�ڴ渴��*/
static inline void memcpy8(void *dest, const void *src, DWORD n)
{
	void *_dest;
	const void *_src;
	DWORD _n;
	__asm__ __volatile__("cld;rep movsb": "=&D"(_dest), "=&S"(_src), "=&c"(_n): "0"(dest), "1"(src), "2"(n): "flags", "memory");
}

/*32λ�ڴ�����*/
static inline void memset32(void *dest, DWORD d, DWORD n)
{
	void *_dest;
	DWORD _n;
	__asm__ __volatile__("cld;rep stosl": "=&D"(_dest), "=&c"(_n): "0"(dest), "a"(d), "1"(n): "flags", "memory");
}

/*32λ�ڴ渴��*/
static inline void memcpy32(void *dest, const void *src, DWORD n)
{
	void *_dest;
	const void *_src;
	DWORD _n;
	__asm__ __volatile__("cld;rep movsl": "=&D"(_dest), "=&S"(_src), "=&c"(_n): "0"(dest), "1"(src), "2"(n): "flags", "memory");
}

/*�ַ�������*/
static inline DWORD strlen(const char *str)
{
	DWORD d0;
	register DWORD _res;
	__asm__ __volatile__
	(
		"cld\n"
		"repne\n"
		"scasb\n"
		"notl %0\n"
		"decl %0"
		: "=c"(_res), "=&D"(d0): "1"(str), "a"(0), "0"(0xFFFFFFFFU): "flags"
	);
	return _res;
}

/*�ַ�������*/
static inline char *strcpy(char *dest, const char *src)
{
	char *_dest;
	const char *_src;
	__asm__ __volatile__
	(
		"cld\n"
		"1:\tlodsb\n"
		"stosb\n"
		"testb %%al, %%al\n"
		"jne 1b"
		: "=&D"(_dest), "=&S"(_src): "0"(dest), "1"(src): "flags", "al", "memory"
	);
	return _dest;
}

/*�ַ�����������*/
static inline void strncpy(char *dest, const char *src, DWORD n)
{
	char *_dest;
	const char *_src;
	DWORD _n;
	__asm__ __volatile__
	(
		"cld\n"
		"1:\tdecl %2\n"
		"js 2f\n"
		"lodsb\n"
		"stosb\n"
		"testb %%al, %%al\n"
		"jne 1b\n"
		"rep stosb\n"
		"2:"
		: "=&D"(_dest), "=&S"(_src), "=&c"(_n): "0"(dest), "1"(src), "2"(n): "flags", "al", "memory"
	);
}

/*�˿�����ֽ�(����ר��)*/
static inline void outb(WORD port, BYTE data)
{
	__asm__ __volatile__("outb %1, %w0":: "d"(port), "a"(data));
}

/*�˿������ֽ�(����ר��)*/
static inline BYTE inb(WORD port)
{
	register BYTE data;
	__asm__ __volatile__("inb %w1, %0": "=a"(data): "d"(port));
	return data;
}

/*�˿������(����ר��)*/
static inline void outw(WORD port, WORD data)
{
	__asm__ __volatile__("outw %1, %w0":: "d"(port), "a"(data));
}

/*�˿�������(����ר��)*/
static inline WORD inw(WORD port)
{
	register WORD data;
	__asm__ __volatile__("inw %w1, %0": "=a"(data): "d"(port));
	return data;
}

/*�˿����˫��(����ר��)*/
static inline void outl(WORD port, DWORD data)
{
	__asm__ __volatile__("outl %1, %w0":: "d"(port), "a"(data));
}

/*�˿�����˫��(����ר��)*/
static inline DWORD inl(WORD port)
{
	register DWORD data;
	__asm__ __volatile__("inl %w1, %0": "=a"(data): "d"(port));
	return data;
}

/*���ж�(����ר��)*/
static inline void cli()
{
	__asm__("cli");
}

/*���ж�(����ר��)*/
static inline void sti()
{
	__asm__("sti");
}

/*����������(����ר��)*/
static inline void lock(volatile DWORD *l)
{
	cli();
	while (*l)
		KGiveUp();
	*l = TRUE;
	sti();
}

/*����������(����ר��)*/
static inline void ulock(volatile DWORD *l)
{
	*l = FALSE;
}

/*�����˳��߳�����*/
static inline long SendExitThedReq(THREAD_ID ptid)
{
	DWORD data[MSG_DATA_LEN];
	data[MSG_ATTR_ID] = MSG_ATTR_EXTTHEDREQ;
	return KSendMsg(&ptid, data, 0);
}

/*�����˳���������*/
static inline long SendExitProcReq(THREAD_ID ptid)
{
	DWORD data[MSG_DATA_LEN];
	data[MSG_ATTR_ID] = MSG_ATTR_EXTPROCREQ;
	return KSendMsg(&ptid, data, 0);
}

#endif
