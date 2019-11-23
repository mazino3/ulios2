/*	ulios.h for ulios
	���ߣ�����
	���ܣ�ulios�ں�һ�γ�ʼ������
	����޸����ڣ�2010-09-13
*/

#include "knldef.h"

/*�ں˱�����ʼ��*/
static inline void InitKnlVal()
{
	KPrint("Initing... kernel variables\n");
	memset32(KnlValue, 0, KVAL_LEN * sizeof(BYTE) / sizeof(DWORD));	/*�����ɢ����*/
	memset32(kpt, INVALID, KPT_LEN * sizeof(THREAD_ID) / sizeof(DWORD));	/*��ʼ���ں˶˿�ע���*/
}

/*�ں��ڴ��ʼ��*/
static inline void InitKFMT()
{
	KPrint("Initing... kernel heap manager\n");
	InitFbt(kmmt, FMT_LEN, kdat, KDAT_SIZ);
}

/*�����ڴ�����ʼ��*/
static inline long InitPMM()
{
	DWORD i;
	MEM_ARDS *CurArd;

	KPrint("Initing... physical memory manager\n");
	i = (((MemEnd - UPHYMEM_ADDR) + 0x0001FFFF) >> 17);	/*ȡ�ý�������ҳ�������*/
	if ((pmmap = (DWORD*)kmalloc(i * sizeof(DWORD))) == NULL)	/*�����û��ڴ�λͼ,��4�ֽ�Ϊ��λ*/
		return KERR_OUT_OF_KNLMEM;
	memset32(pmmap, 0xFFFFFFFF, i);	/*�ȱ��Ϊ����*/
	PmmLen = (i << 5);	/*�û��ڴ���ҳ��*/
	PmpID = INVALID;
	RemmSiz = 0;
	KPrint("E820 MemType\tAddress\tSize\n");
	for (CurArd = ards; CurArd->addr != INVALID; CurArd++)
	{
		KPrint("%d\t0x%X\t0x%X\n", CurArd->type, CurArd->addr, CurArd->siz);
		if (CurArd->type == ARDS_TYPE_RAM && CurArd->addr + CurArd->siz > UPHYMEM_ADDR)	/*�ڴ�����Ч�Ұ����˽����ڴ�ҳ��*/
		{
			DWORD fst, cou, end, tcu;	/*ҳ����ʼ��,����,ѭ������ֵ,��ʱ����*/

			if (CurArd->addr < UPHYMEM_ADDR)	/*��ַת��Ϊ�����ڴ���Ե�ַ*/
			{
				fst = 0;
				cou = (CurArd->addr + CurArd->siz - UPHYMEM_ADDR) >> 12;
			}
			else
			{
				fst = (CurArd->addr + 0x00000FFF - UPHYMEM_ADDR) >> 12;	/*���ֽڵ�ַ�߶˵�ҳ����ʼ*/
				cou = CurArd->siz >> 12;
			}
			if (PmpID > fst)
				PmpID = fst;
			RemmSiz += (cou << 12);
			end = (fst + 0x0000001F) & 0xFFFFFFE0;
			tcu = end - fst;
			if (fst + cou < end)	/*32ҳ�߽��ڵ�С����*/
			{
				cou = (fst + cou) & 0x0000001F;
				pmmap[fst >> 5] &= ((0xFFFFFFFF >> tcu) | (0xFFFFFFFF << cou));
				continue;
			}
			pmmap[fst >> 5] &= (0xFFFFFFFF >> tcu);	/*32ҳ�߽翪ʼ�������*/
			fst = end;
			cou -= tcu;
			memset32(&pmmap[fst >> 5], 0, cou >> 5);	/*������*/
			fst += (cou & 0xFFFFFFE0);
			cou &= 0x0000001F;
			if (cou)	/*32ҳ�߽�����������*/
				pmmap[fst >> 5] &= (0xFFFFFFFF << cou);
		}
	}
	KPrint("Available Memory Size: 0x%X\n", RemmSiz);
	return NO_ERROR;
}

/*��Ϣ�����ʼ��*/
static inline long InitMsg()
{
	MESSAGE_DESC *msg;

	KPrint("Initing... message manager\n");
	if ((FstMsg = (MESSAGE_DESC*)kmalloc(MSGMT_LEN * sizeof(MESSAGE_DESC))) == NULL)
		return KERR_OUT_OF_KNLMEM;
	for (msg = FstMsg; msg < &FstMsg[MSGMT_LEN - 1]; msg++)
		msg->nxt = msg + 1;
	msg->nxt = NULL;
	return NO_ERROR;
}

/*��ַӳ������ʼ��*/
static inline long InitMap()
{
	MAPBLK_DESC *map;

	KPrint("Initing... address mapping manager\n");
	if ((FstMap = (MAPBLK_DESC*)kmalloc(MAPMT_LEN * sizeof(MAPBLK_DESC))) == NULL)
		return KERR_OUT_OF_KNLMEM;
	for (map = FstMap; map < &FstMap[MAPMT_LEN - 1]; map++)
		map->nxt = map + 1;
	map->nxt = NULL;
	return NO_ERROR;
}

/*���̹�����ʼ��*/
static inline void InitPMT()
{
	KPrint("Initing... process manager\n");
	memset32(pmt, 0, PMT_LEN * sizeof(PROCESS_DESC*) / sizeof(DWORD));
	EndPmd = FstPmd = pmt;
/*	CurPmd = NULL;
	PmdCou = 0;
	clock = 0;
	SleepList = NULL;
	LastI387 = NULL;
*/
}

/*��ʼ���ں˽���*/
static inline void InitKnlProc()
{
	KPrint("Initing... kernel process\n");
/*	memset32(&KnlTss, 0, sizeof(TSS) / sizeof(DWORD));
*/	KnlTss.cr3 = (DWORD)kpdt;
	KnlTss.io = sizeof(TSS);
	SetSegDesc(&gdt[UCODE_SEL >> 3], 0, 0xFFFFF, DESC_ATTR_P | DESC_ATTR_DPL3 | DESC_ATTR_DT | STOSEG_ATTR_T_E | STOSEG_ATTR_T_A | SEG_ATTR_G | STOSEG_ATTR_D);	/*��ʼ���û�GDT��*/
	SetSegDesc(&gdt[UDATA_SEL >> 3], 0, 0xFFFFF, DESC_ATTR_P | DESC_ATTR_DPL3 | DESC_ATTR_DT | STOSEG_ATTR_T_D_W | STOSEG_ATTR_T_A | SEG_ATTR_G | STOSEG_ATTR_D);
	SetSegDesc(&gdt[TSS_SEL >> 3], (DWORD)&KnlTss, sizeof(TSS) - 1, DESC_ATTR_P | SYSSEG_ATTR_T_TSS);
	__asm__("ltr %%ax":: "a"(TSS_SEL));
}

/*�쳣IDT����(������ַ�޷�����λ����,ֻ���üӼ���)*/
SEG_GATE_DESC IsrIdtTable[] = {
	{(DWORD)AsmIsr00 - 0x00010000 + (KCODE_SEL << 16), 0x00010000 | DESC_ATTR_P | DESC_ATTR_DPL0 | GATE_ATTR_T_TRAP},
	{(DWORD)AsmIsr01 - 0x00010000 + (KCODE_SEL << 16), 0x00010000 | DESC_ATTR_P | DESC_ATTR_DPL0 | GATE_ATTR_T_TRAP},
	{(DWORD)AsmIsr02 - 0x00010000 + (KCODE_SEL << 16), 0x00010000 | DESC_ATTR_P | DESC_ATTR_DPL0 | GATE_ATTR_T_TRAP},
	{(DWORD)AsmIsr03 - 0x00010000 + (KCODE_SEL << 16), 0x00010000 | DESC_ATTR_P | DESC_ATTR_DPL3 | GATE_ATTR_T_TRAP},
	{(DWORD)AsmIsr04 - 0x00010000 + (KCODE_SEL << 16), 0x00010000 | DESC_ATTR_P | DESC_ATTR_DPL3 | GATE_ATTR_T_TRAP},
	{(DWORD)AsmIsr05 - 0x00010000 + (KCODE_SEL << 16), 0x00010000 | DESC_ATTR_P | DESC_ATTR_DPL3 | GATE_ATTR_T_TRAP},
	{(DWORD)AsmIsr06 - 0x00010000 + (KCODE_SEL << 16), 0x00010000 | DESC_ATTR_P | DESC_ATTR_DPL0 | GATE_ATTR_T_TRAP},
	{(DWORD)AsmIsr07 - 0x00010000 + (KCODE_SEL << 16), 0x00010000 | DESC_ATTR_P | DESC_ATTR_DPL0 | GATE_ATTR_T_TRAP},
	{(DWORD)AsmIsr08 - 0x00010000 + (KCODE_SEL << 16), 0x00010000 | DESC_ATTR_P | DESC_ATTR_DPL0 | GATE_ATTR_T_TRAP},
	{(DWORD)AsmIsr09 - 0x00010000 + (KCODE_SEL << 16), 0x00010000 | DESC_ATTR_P | DESC_ATTR_DPL0 | GATE_ATTR_T_TRAP},
	{(DWORD)AsmIsr0A - 0x00010000 + (KCODE_SEL << 16), 0x00010000 | DESC_ATTR_P | DESC_ATTR_DPL0 | GATE_ATTR_T_TRAP},
	{(DWORD)AsmIsr0B - 0x00010000 + (KCODE_SEL << 16), 0x00010000 | DESC_ATTR_P | DESC_ATTR_DPL0 | GATE_ATTR_T_TRAP},
	{(DWORD)AsmIsr0C - 0x00010000 + (KCODE_SEL << 16), 0x00010000 | DESC_ATTR_P | DESC_ATTR_DPL0 | GATE_ATTR_T_TRAP},
	{(DWORD)AsmIsr0D - 0x00010000 + (KCODE_SEL << 16), 0x00010000 | DESC_ATTR_P | DESC_ATTR_DPL0 | GATE_ATTR_T_TRAP},
	{(DWORD)AsmIsr0E - 0x00010000 + (KCODE_SEL << 16), 0x00010000 | DESC_ATTR_P | DESC_ATTR_DPL0 | GATE_ATTR_T_INTR},
	{(DWORD)AsmIsr0F - 0x00010000 + (KCODE_SEL << 16), 0x00010000 | DESC_ATTR_P | DESC_ATTR_DPL0 | GATE_ATTR_T_TRAP},
	{(DWORD)AsmIsr10 - 0x00010000 + (KCODE_SEL << 16), 0x00010000 | DESC_ATTR_P | DESC_ATTR_DPL0 | GATE_ATTR_T_TRAP},
	{(DWORD)AsmIsr11 - 0x00010000 + (KCODE_SEL << 16), 0x00010000 | DESC_ATTR_P | DESC_ATTR_DPL0 | GATE_ATTR_T_TRAP},
	{(DWORD)AsmIsr12 - 0x00010000 + (KCODE_SEL << 16), 0x00010000 | DESC_ATTR_P | DESC_ATTR_DPL0 | GATE_ATTR_T_TRAP},
	{(DWORD)AsmIsr13 - 0x00010000 + (KCODE_SEL << 16), 0x00010000 | DESC_ATTR_P | DESC_ATTR_DPL0 | GATE_ATTR_T_TRAP}
};

/*�жϴ����ʼ��*/
static inline void InitINTR()
{
	KPrint("Initing... interrupts & exceptions\n");
	memcpy32(idt, IsrIdtTable, sizeof(IsrIdtTable) / sizeof(DWORD));	/*����20��ISR����������*/
	SetGate(&idt[INTN_APICALL], KCODE_SEL, (DWORD)AsmApiCall, DESC_ATTR_P | DESC_ATTR_DPL3 | GATE_ATTR_T_TRAP);	/*����ϵͳ����*/
	memset32(IrqPort, INVALID, IRQ_LEN * sizeof(THREAD_ID) / sizeof(DWORD));	/*��ʼ��IRQ�˿�ע���*/
	/*��ʱ�Ӻʹ�Ƭ8259�ж�*/
	SetGate(&idt[0x20 + IRQN_TIMER], KCODE_SEL, (DWORD)AsmIrq0, DESC_ATTR_P | DESC_ATTR_DPL0 | GATE_ATTR_T_INTR);
	SetGate(&idt[0x20 + IRQN_SLAVE8259], KCODE_SEL, (DWORD)AsmIrq2, DESC_ATTR_P | DESC_ATTR_DPL0 | GATE_ATTR_T_INTR);
	/*��8259�ж�оƬ���б��,�ο�linux 0.11*/
	outb(0x20, 0x11);
	outb(0xA0, 0x11);
	outb(MASTER8259_PORT, 0x20);
	outb(SLAVER8259_PORT, 0x28);
	outb(MASTER8259_PORT, 0x04);
	outb(SLAVER8259_PORT, 0x02);
	outb(MASTER8259_PORT, 0x01);
	outb(SLAVER8259_PORT, 0x01);
	outb(MASTER8259_PORT, 0xFA);	/*��8259Aֻ����ʱ��*/
	outb(SLAVER8259_PORT, 0xFF);	/*��8259Aȫ����ֹ*/
	/*��8253ʱ��оƬ���б��,�ο�linux 0.00*/
	outb(0x43, 0x36);
	outb(0x40, 0x9C);	/*ʱ���ж�Ƶ��:100HZ,д����:1193180/100*/
	outb(0x40, 0x2E);
}

/*��ʼ����������*/
static inline long InitBaseSrv()
{
	PHYBLK_DESC *CurSeg;
	THREAD_ID ptid;
	long res;

	KPrint("Loading... basic services\n");
	for (CurSeg = &BaseSrv[1]; CurSeg->addr; CurSeg++)
	{
		KPrint("0x%X[0x%X]\n", CurSeg->addr, CurSeg->siz);
		if ((res = CreateProc(EXEC_ARGV_BASESRV | EXEC_ARGV_DRIVER, CurSeg->addr, CurSeg->siz, &ptid)) != NO_ERROR)
			return res;
	}
	return NO_ERROR;
}
