/*	cintr.c for ulios
	���ߣ�����
	���ܣ�C�ж�/�쳣/ϵͳ���ô���
	����޸����ڣ�2009-07-01
*/

#include "knldef.h"

/*C�쳣��*/
void (*IsrCallTable[])(DWORD edi, DWORD esi, DWORD ebp, DWORD esp, DWORD ebx, DWORD edx, DWORD ecx, DWORD eax, WORD gs, WORD fs, WORD es, WORD ds, DWORD IsrN, DWORD ErrCode, DWORD eip, WORD cs, DWORD eflags) = {
	IsrProc, IsrProc, IsrProc, IsrProc, IsrProc, IsrProc, IsrProc, FpuFaultProc,
	IsrProc, IsrProc, IsrProc, IsrProc, IsrProc, IsrProc, PageFaultProc, IsrProc,
	IsrProc, IsrProc, IsrProc, IsrProc
};

/*�жϱ�*/
void (*AsmIrqCallTable[])() = {
	AsmIrq0, AsmIrq1, AsmIrq2, AsmIrq3, AsmIrq4, AsmIrq5, AsmIrq6, AsmIrq7,
	AsmIrq8, AsmIrq9, AsmIrqA, AsmIrqB, AsmIrqC, AsmIrqD, AsmIrqE, AsmIrqF
};

/*Cϵͳ���ñ�*/
void (*ApiCallTable[])(DWORD *argv) = {
	ApiGetPtid, ApiGiveUp, ApiSleep, ApiCreateThread, ApiExitThread, ApiKillThread, ApiCreateProcess, ApiExitProcess,
	ApiKillProcess, ApiRegKnlPort, ApiUnregKnlPort, ApiGetKpToThed, ApiRegIrq, ApiUnregIrq, ApiSendMsg, ApiRecvMsg,
	ApiRecvProcMsg, ApiMapPhyAddr, ApiMapUserAddr, ApiFreeAddr, ApiWriteProcAddr, ApiReadProcAddr, ApiUnmapProcAddr, ApiCnlmapProcAddr,
	ApiGetClock, ApiLock
#ifdef DEBUG
	,ApiDebug
#endif
};

/*ע��IRQ�źŵ���Ӧ�߳�*/
long RegIrq(DWORD IrqN)
{
	PROCESS_DESC *CurProc;
	BYTE mask;

	CurProc = CurPmd;
	if (CurProc->attr & PROC_ATTR_APPS)
		return KERR_NO_DRIVER_PRIVILEGE;	/*����������ȨAPI*/
	if (IrqN >= IRQ_LEN)
		return KERR_INVALID_IRQNUM;
	cli();
	if (idt[0x20 + IrqN].d1)
	{
		sti();
		return KERR_IRQ_ALREADY_REGISTERED;
	}
	SetGate(&idt[0x20 + IrqN], KCODE_SEL, (DWORD)AsmIrqCallTable[IrqN], DESC_ATTR_P | DESC_ATTR_DPL0 | GATE_ATTR_T_INTR);
	IrqPort[IrqN] = CurProc->CurTmd->id;
	if (IrqN < 8)
	{
		mask = inb(MASTER8259_PORT);	/*��Ƭ*/
		mask &= (~(1ul << IrqN));
		outb(MASTER8259_PORT, mask);
	}
	else
	{
		mask = inb(SLAVER8259_PORT);	/*��Ƭ*/
		mask &= (~(1ul << (IrqN & 7)));
		outb(SLAVER8259_PORT, mask);
	}
	sti();
	return NO_ERROR;
}

/*ע��IRQ�źŵ���Ӧ�߳�*/
long UnregIrq(DWORD IrqN)
{
	PROCESS_DESC *CurProc;
	BYTE mask;

	CurProc = CurPmd;
	if (CurProc->attr & PROC_ATTR_APPS)
		return KERR_NO_DRIVER_PRIVILEGE;	/*����������ȨAPI*/
	if (IrqN >= IRQ_LEN)
		return KERR_INVALID_IRQNUM;
	cli();
	if (idt[0x20 + IrqN].d1 == 0)
	{
		sti();
		return KERR_IRQ_NOT_REGISTERED;
	}
	if (IrqPort[IrqN].ProcID != CurProc->CurTmd->id.ProcID)
	{
		sti();
		return KERR_CURPROC_NOT_REGISTRANT;
	}
	if (IrqN < 8)
	{
		mask = inb(MASTER8259_PORT);	/*��Ƭ*/
		mask |= (1ul << IrqN);
		outb(MASTER8259_PORT, mask);
	}
	else
	{
		mask = inb(SLAVER8259_PORT);	/*��Ƭ*/
		mask |= (1ul << (IrqN & 7));
		outb(SLAVER8259_PORT, mask);
	}
	*(DWORD*)(&IrqPort[IrqN]) = INVALID;
	idt[0x20 + IrqN].d1 = 0;
	sti();
	return NO_ERROR;
}

/*ע���̵߳�����IRQ�ź�*/
void UnregAllIrq()
{
	PROCESS_DESC *CurProc;
	THREAD_ID ptid;
	DWORD i;
	BYTE mask;

	CurProc = CurPmd;
	if (CurProc->attr & PROC_ATTR_APPS)
		return;	/*����������ȨAPI*/
	ptid = CurProc->CurTmd->id;
	cli();
	for (i = 0; i < IRQ_LEN; i++)
		if (*(DWORD*)(&IrqPort[i]) == *(DWORD*)(&ptid))
		{
			if (i < 8)
			{
				mask = inb(MASTER8259_PORT);	/*��Ƭ*/
				mask |= (1ul << i);
				outb(MASTER8259_PORT, mask);
			}
			else
			{
				mask = inb(SLAVER8259_PORT);	/*��Ƭ*/
				mask |= (1ul << (i & 7));
				outb(SLAVER8259_PORT, mask);
			}
			*(DWORD*)(&IrqPort[i]) = INVALID;
			idt[0x20 + i].d1 = 0;
		}
	sti();
}

/*���ɻָ��쳣�������*/
void IsrProc(DWORD edi, DWORD esi, DWORD ebp, DWORD esp, DWORD ebx, DWORD edx, DWORD ecx, DWORD eax, WORD gs, WORD fs, WORD es, WORD ds, DWORD IsrN, DWORD ErrCode, DWORD eip, WORD cs, DWORD eflags)
{
	MESSAGE_DESC *msg;

	KPrint("ISR:0x%X\tcode:0x%X\tEIP:0x%X\tEFLAGS:0x%X\n", IsrN, ErrCode, eip, eflags);
	KPrint("EAX:0x%X\tEBX:0x%X\tECX:0x%X\tEDX:0x%X\n", eax, ebx, ecx, edx);
	KPrint("ESI:0x%X\tEDI:0x%X\tEBP:0x%X\tESP:0x%X\n", esi, edi, ebp, esp);
	KPrint("CS:0x%X\tDS:0x%X\tES:0x%X\tFS:0x%X\tGS:0x%X\n", cs, ds, es, fs, gs);
	if ((msg = AllocMsg()) != NULL)	/*֪ͨ���������������Ϣ*/
	{
		msg->ptid = kpt[REP_KPORT];
		msg->data[MSG_ATTR_ID] = MSG_ATTR_ISR;
		msg->data[1] = IsrN;
		msg->data[2] = ErrCode;
		msg->data[3] = eip;
		if (SendMsg(msg) != NO_ERROR)
			FreeMsg(msg);
	}
	ThedExit(KERR_THED_EXCEPTION);
}

/*����Э�������쳣�������*/
void FpuFaultProc(DWORD edi, DWORD esi, DWORD ebp, DWORD esp, DWORD ebx, DWORD edx, DWORD ecx, DWORD eax, WORD gs, WORD fs, WORD es, WORD ds, DWORD IsrN, DWORD ErrCode, DWORD eip, WORD cs, DWORD eflags)
{
	THREAD_DESC *CurThed;
	I387 *CurI387;

	CurThed = CurPmd->CurTmd;
	CurI387 = CurThed->i387;
	CurThed->attr &= (~THED_ATTR_APPS);	/*����ϵͳ����̬*/
	if (CurI387 == NULL)	/*�߳��״�ִ��Э������ָ��*/
	{
		if ((CurI387 = (I387*)LockKmalloc(sizeof(I387))) == NULL)
			ThedExit(KERR_OUT_OF_KNLMEM);
	}
	cli();
	ClearTs();
	if (LastI387 != CurI387)	/*ʹ��Э���������̲߳���ʱ�����л�*/
	{
		if (LastI387)
			__asm__("fwait;fnsave %0": "=m"(*LastI387));	/*����Э�������Ĵ���*/
		if (CurThed->i387)	/*Э�������Ѿ�����*/
			__asm__("frstor %0":: "m"(*CurI387));	/*����Э�������Ĵ���*/
		else
		{
			__asm__("fninit");	/*��ʼ��Э������*/
			CurThed->i387 = CurI387;
		}
		LastI387 = CurI387;
	}
	sti();
	CurThed->attr |= THED_ATTR_APPS;	/*�뿪ϵͳ����̬*/
}

/*�����ж��źŵ��ܵ�����*/
void IrqProc(DWORD edi, DWORD esi, DWORD ebp, DWORD esp, DWORD ebx, DWORD edx, DWORD ecx, DWORD eax, WORD gs, WORD fs, WORD es, WORD ds, DWORD IrqN)
{
	THREAD_DESC *CurThed;

	/*�����жϴ��������ǰ�ж��Ѿ��ر�*/
	if (IrqN == 0)
	{
		clock++;
		if (SleepList && clock >= SleepList->WakeupClock)	/*��ʱ�������������Ѿ���ʱ���߳�*/
			wakeup(SleepList);
		else
		{
			schedul();
			CurThed = CurPmd ? CurPmd->CurTmd : NULL;
			if (CurThed && (CurThed->attr & (THED_ATTR_APPS | THED_ATTR_KILLED)) == (THED_ATTR_APPS | THED_ATTR_KILLED))	/*�߳���Ӧ��̬�±�ɱ��*/
			{
				CurThed->attr &= (~THED_ATTR_KILLED);
				sti();
				ThedExit(KERR_THED_KILLED);
			}
		}
	}
	else	/*���жϴ�����̷�����Ϣ*/
	{
		MESSAGE_DESC *msg;

		sti();
		CurThed = CurPmd ? CurPmd->CurTmd : NULL;
		if (CurThed)
			CurThed->attr &= (~THED_ATTR_APPS);	/*����ϵͳ����̬*/
		if ((msg = AllocMsg()) == NULL)	/*�ڴ治��*/
		{
			if (CurThed)
				CurThed->attr |= THED_ATTR_APPS;	/*�뿪ϵͳ����̬*/
			return;
		}
		msg->ptid = IrqPort[IrqN];
		msg->data[MSG_ATTR_ID] = MSG_ATTR_IRQ;
		msg->data[1] = IrqN;
		if (SendMsg(msg) != NO_ERROR)
			FreeMsg(msg);
		if (CurThed)
			CurThed->attr |= THED_ATTR_APPS;	/*�뿪ϵͳ����̬*/
	}
}

/*ϵͳ���ýӿ�*/
void ApiCall(DWORD edi, DWORD esi, DWORD ebp, DWORD esp, DWORD ebx, DWORD edx, DWORD ecx, volatile DWORD eax)
{
	THREAD_DESC *CurThed;

	CurThed = CurPmd->CurTmd;
	if (CurThed->attr & THED_ATTR_KILLED)	/*�߳��ѱ�ɱ��*/
	{
		CurThed->attr &= (~THED_ATTR_KILLED);
		sti();
		ThedExit(KERR_THED_KILLED);
	}
	if (eax >= ((sizeof(ApiCallTable) / sizeof(void*)) << 16))
	{
		eax = KERR_INVALID_APINUM;
		return;
	}
	CurThed->attr &= (~THED_ATTR_APPS);	/*����ϵͳ����̬*/
	ApiCallTable[eax >> 16](&edi);
	CurThed->attr |= THED_ATTR_APPS;	/*�뿪ϵͳ����̬*/
}

/*����ΪAPI�ӿں���ʵ��*/
#define EDI_ID	0
#define ESI_ID	1
#define EBP_ID	2
#define ESP_ID	3
#define EBX_ID	4
#define EDX_ID	5
#define ECX_ID	6
#define EAX_ID	7

/*ȡ���߳�ID*/
void ApiGetPtid(DWORD *argv)
{
	THREAD_DESC *CurThed;

	CurThed = CurPmd->CurTmd;
	argv[EBX_ID] = *(DWORD*)(&CurThed->id);
	argv[ECX_ID] = CurThed->par;
	argv[EDX_ID] = CurPmd->par;
	argv[EAX_ID] = NO_ERROR;
}

/*�������������*/
void ApiGiveUp(DWORD *argv)
{
	cli();
	schedul();
	sti();
	argv[EAX_ID] = NO_ERROR;
}

/*˯��*/
void ApiSleep(DWORD *argv)
{
	if (argv[EBX_ID])
		CliSleep(FALSE, argv[EBX_ID]);
	argv[EAX_ID] = NO_ERROR;
}

/*�����߳�*/
void ApiCreateThread(DWORD *argv)
{
	argv[EAX_ID] = CreateThed(argv[EBX_ID], argv[ECX_ID], argv[EDX_ID], (THREAD_ID*)&argv[EBX_ID]);
}

/*�˳��߳�*/
void ApiExitThread(DWORD *argv)
{
	ThedExit(argv[EBX_ID]);
}

/*ɱ���߳�*/
void ApiKillThread(DWORD *argv)
{
	argv[EAX_ID] = KillThed(argv[EBX_ID]);
}

/*��������*/
void ApiCreateProcess(DWORD *argv)
{
	BYTE *addr;

	addr = (BYTE*)argv[ESI_ID];
	if (addr >= (BYTE*)UADDR_OFF && addr <= (BYTE*)(0 - PROC_ARGS_SIZE))
	{
		DWORD siz;

		for (siz = 0; *addr; addr++)
			if (++siz >= PROC_ARGS_SIZE)
			{
				argv[EAX_ID] = KERR_ARGS_TOO_LONG;
				return;
			}
		addr = (BYTE*)argv[ESI_ID];
		CurPmd->CurTmd->attr &= (~THED_ATTR_APPS);	/*��ֹ�����û��ڴ�ʱ����ҳ�쳣,���½���ϵͳ����̬*/
	}
	else
		addr = NULL;
	argv[EAX_ID] = CreateProc(argv[EBX_ID] & (~EXEC_ARGV_BASESRV), argv[EDI_ID], (DWORD)addr, (THREAD_ID*)&argv[EBX_ID]);
}

/*�˳�����*/
void ApiExitProcess(DWORD *argv)
{
	DeleteProc(argv[EBX_ID]);
}

/*ɱ������*/
void ApiKillProcess(DWORD *argv)
{
	argv[EAX_ID] = KillProc(argv[EBX_ID]);
}

/*ע���ں˶˿ڶ�Ӧ�߳�*/
void ApiRegKnlPort(DWORD *argv)
{
	argv[EAX_ID] = RegKnlPort(argv[EBX_ID]);
}

/*ע���ں˶˿ڶ�Ӧ�߳�*/
void ApiUnregKnlPort(DWORD *argv)
{
	argv[EAX_ID] = UnregKnlPort(argv[EBX_ID]);
}

/*ȡ���ں˶˿ڶ�Ӧ�߳�*/
void ApiGetKpToThed(DWORD *argv)
{
	argv[EAX_ID] = GetKptThed(argv[EBX_ID], (THREAD_ID*)&argv[EBX_ID]);
}

/*ע��IRQ�źŵ���Ӧ�߳�*/
void ApiRegIrq(DWORD *argv)
{
	argv[EAX_ID] = RegIrq(argv[EBX_ID]);
}

/*ע��IRQ�źŵ���Ӧ�߳�*/
void ApiUnregIrq(DWORD *argv)
{
	argv[EAX_ID] = UnregIrq(argv[EBX_ID]);
}

/*������Ϣ*/
void ApiSendMsg(DWORD *argv)
{
	MESSAGE_DESC *msg;
	void *addr;
	DWORD data[MSG_DATA_LEN];

	addr = (void*)argv[ESI_ID];
	if (addr < UADDR_OFF || addr > (void*)(0 - sizeof(data)))
	{
		argv[EAX_ID] = KERR_INVALID_MEMARGS_ADDR;
		return;
	}
	memcpy32(data, addr, MSG_DATA_LEN);	/*�������ݵ��ں˿ռ�*/
	CurPmd->CurTmd->attr &= (~THED_ATTR_APPS);	/*��ֹ�����û��ڴ�ʱ����ҳ�쳣,���½���ϵͳ����̬*/
	if (data[MSG_ATTR_ID] < MSG_ATTR_USER)
	{
		argv[EAX_ID] = KERR_INVALID_USERMSG_ATTR;
		return;
	}
	if ((msg = AllocMsg()) == NULL)
	{
		argv[EAX_ID] = KERR_MSG_NOT_ENOUGH;
		return;
	}
	if (((THREAD_ID*)&argv[EBX_ID])->ThedID == 0xFFFF && ((THREAD_ID*)&argv[EBX_ID])->ProcID < KPT_LEN)	/*�����ں˶˿ڲ���*/
		argv[EBX_ID] = *((DWORD*)&kpt[((THREAD_ID*)&argv[EBX_ID])->ProcID]);
	msg->ptid = *((THREAD_ID*)&argv[EBX_ID]);
	memcpy32(msg->data, data, MSG_DATA_LEN);
	argv[EAX_ID] = SendMsg(msg);
	if (argv[EAX_ID] != NO_ERROR)
		FreeMsg(msg);
	else if (argv[ECX_ID] && (argv[EAX_ID] = RecvProcMsg(&msg, *((THREAD_ID*)&argv[EBX_ID]), argv[ECX_ID])) == NO_ERROR)	/*�ȴ�������Ϣ*/
	{
		argv[EBX_ID] = *((DWORD*)&msg->ptid);
		memcpy32(data, msg->data, MSG_DATA_LEN);
		FreeMsg(msg);
		memcpy32(addr, data, MSG_DATA_LEN);	/*�������ݵ��û��ռ�*/
	}
}

/*������Ϣ*/
void ApiRecvMsg(DWORD *argv)
{
	MESSAGE_DESC *msg;
	void *addr;
	DWORD data[MSG_DATA_LEN];

	addr = (void*)argv[ESI_ID];
	if (addr < UADDR_OFF || addr > (void*)(0 - sizeof(data)))
	{
		argv[EAX_ID] = KERR_INVALID_MEMARGS_ADDR;
		return;
	}
	if ((argv[EAX_ID] = RecvMsg(&msg, argv[ECX_ID])) == NO_ERROR)
	{
		argv[EBX_ID] = *((DWORD*)&msg->ptid);
		memcpy32(data, msg->data, MSG_DATA_LEN);
		FreeMsg(msg);
		memcpy32(addr, data, MSG_DATA_LEN);	/*�������ݵ��û��ռ�*/
	}
}

/*����ָ�����̵���Ϣ*/
void ApiRecvProcMsg(DWORD *argv)
{
	MESSAGE_DESC *msg;
	void *addr;
	DWORD data[MSG_DATA_LEN];

	addr = (void*)argv[ESI_ID];
	if (addr < UADDR_OFF || addr > (void*)(0 - sizeof(data)))
	{
		argv[EAX_ID] = KERR_INVALID_MEMARGS_ADDR;
		return;
	}
	if (((THREAD_ID*)&argv[EBX_ID])->ThedID == 0xFFFF && ((THREAD_ID*)&argv[EBX_ID])->ProcID < KPT_LEN)	/*�����ں˶˿ڲ���*/
		argv[EBX_ID] = *((DWORD*)&kpt[((THREAD_ID*)&argv[EBX_ID])->ProcID]);
	if ((argv[EAX_ID] = RecvProcMsg(&msg, *((THREAD_ID*)&argv[EBX_ID]), argv[ECX_ID])) == NO_ERROR)
	{
		argv[EBX_ID] = *((DWORD*)&msg->ptid);
		memcpy32(data, msg->data, MSG_DATA_LEN);
		FreeMsg(msg);
		memcpy32(addr, data, MSG_DATA_LEN);	/*�������ݵ��û��ռ�*/
	}
}

/*ӳ�������ַ*/
void ApiMapPhyAddr(DWORD *argv)
{
	argv[EAX_ID] = MapPhyAddr((void**)&argv[ESI_ID], argv[EBX_ID], argv[ECX_ID]);
}

/*ӳ���û���ַ*/
void ApiMapUserAddr(DWORD *argv)
{
	argv[EAX_ID] = MapUserAddr((void**)&argv[ESI_ID], argv[ECX_ID]);
}

/*�����û���ַ��*/
void ApiFreeAddr(DWORD *argv)
{
	argv[EAX_ID] = UnmapAddr((void*)argv[ESI_ID]);
}

/*ӳ����̵�ַд��*/
void ApiWriteProcAddr(DWORD *argv)
{
	void *addr;
	DWORD data[MSG_DATA_LEN];

	addr = (void*)argv[ESI_ID];
	if (addr < UADDR_OFF || addr > (void*)(0 - sizeof(data)))
	{
		argv[EAX_ID] = KERR_INVALID_MEMARGS_ADDR;
		return;
	}
	memcpy32(data, addr, MSG_DATA_LEN);	/*�������ݵ��ں˿ռ�*/
	CurPmd->CurTmd->attr &= (~THED_ATTR_APPS);	/*��ֹ�����û��ڴ�ʱ����ҳ�쳣,���½���ϵͳ����̬*/
	if (((THREAD_ID*)&argv[EBX_ID])->ThedID == 0xFFFF && ((THREAD_ID*)&argv[EBX_ID])->ProcID < KPT_LEN)	/*�����ں˶˿ڲ���*/
		argv[EBX_ID] = *((DWORD*)&kpt[((THREAD_ID*)&argv[EBX_ID])->ProcID]);
	if ((argv[EAX_ID] = MapProcAddr((void*)argv[EDI_ID], argv[ECX_ID], (THREAD_ID*)&argv[EBX_ID], FALSE, TRUE, data, argv[EDX_ID])) == NO_ERROR)
		memcpy32(addr, data, MSG_DATA_LEN);	/*�������ݵ��û��ռ�*/
}

/*ӳ����̵�ַ��ȡ*/
void ApiReadProcAddr(DWORD *argv)
{
	void *addr;
	DWORD data[MSG_DATA_LEN];

	addr = (void*)argv[ESI_ID];
	if (addr < UADDR_OFF || addr > (void*)(0 - sizeof(data)))
	{
		argv[EAX_ID] = KERR_INVALID_MEMARGS_ADDR;
		return;
	}
	memcpy32(data, addr, MSG_DATA_LEN);	/*�������ݵ��ں˿ռ�*/
	CurPmd->CurTmd->attr &= (~THED_ATTR_APPS);	/*��ֹ�����û��ڴ�ʱ����ҳ�쳣,���½���ϵͳ����̬*/
	if (((THREAD_ID*)&argv[EBX_ID])->ThedID == 0xFFFF && ((THREAD_ID*)&argv[EBX_ID])->ProcID < KPT_LEN)	/*�����ں˶˿ڲ���*/
		argv[EBX_ID] = *((DWORD*)&kpt[((THREAD_ID*)&argv[EBX_ID])->ProcID]);
	if ((argv[EAX_ID] = MapProcAddr((void*)argv[EDI_ID], argv[ECX_ID], (THREAD_ID*)&argv[EBX_ID], TRUE, TRUE, data, argv[EDX_ID])) == NO_ERROR)
		memcpy32(addr, data, MSG_DATA_LEN);	/*�������ݵ��û��ռ�*/
}

/*����ӳ����̵�ַ*/
void ApiUnmapProcAddr(DWORD *argv)
{
	void *addr;
	DWORD data[MSG_DATA_LEN];

	addr = (void*)argv[ESI_ID];
	if (addr < UADDR_OFF || addr > (void*)(0 - sizeof(data)))
	{
		argv[EAX_ID] = KERR_INVALID_MEMARGS_ADDR;
		return;
	}
	memcpy32(data, addr, MSG_DATA_LEN);	/*�������ݵ��ں˿ռ�*/
	CurPmd->CurTmd->attr &= (~THED_ATTR_APPS);	/*��ֹ�����û��ڴ�ʱ����ҳ�쳣,���½���ϵͳ����̬*/
	argv[EAX_ID] = UnmapProcAddr((void*)argv[EDI_ID], data);
}

/*ȡ��ӳ����̵�ַ*/
void ApiCnlmapProcAddr(DWORD *argv)
{
	void *addr;
	DWORD data[MSG_DATA_LEN];

	addr = (void*)argv[ESI_ID];
	if (addr < UADDR_OFF || addr > (void*)(0 - sizeof(data)))
	{
		argv[EAX_ID] = KERR_INVALID_MEMARGS_ADDR;
		return;
	}
	memcpy32(data, addr, MSG_DATA_LEN);	/*�������ݵ��ں˿ռ�*/
	CurPmd->CurTmd->attr &= (~THED_ATTR_APPS);	/*��ֹ�����û��ڴ�ʱ����ҳ�쳣,���½���ϵͳ����̬*/
	argv[EAX_ID] = CnlmapProcAddr((void*)argv[EDI_ID], data);
}

/*ȡ�ÿ���������ʱ��*/
void ApiGetClock(DWORD *argv)
{
	argv[EBX_ID] = clock;
	argv[EAX_ID] = NO_ERROR;
}

/*�߳�ͬ��������*/
void ApiLock(DWORD *argv)
{
	void *addr;
	DWORD OutClock;

	addr = (void*)argv[ESI_ID];
	if (addr < UADDR_OFF || addr > (void*)(0 - sizeof(DWORD)))
	{
		argv[EAX_ID] = KERR_INVALID_MEMARGS_ADDR;
		return;
	}
	argv[EDX_ID] = *((volatile DWORD*)addr);	/*��ǰ��������*/
	CurPmd->CurTmd->attr &= (~THED_ATTR_APPS);	/*��ֹ�����û��ڴ�ʱ����ҳ�쳣,���½���ϵͳ����̬*/
	OutClock = (argv[EBX_ID] == INVALID ? INVALID : clock + argv[EBX_ID]);
	cli();
	while (*((volatile DWORD*)addr))
	{
		if (clock >= OutClock)
		{
			sti();
			argv[EAX_ID] = KERR_OUT_OF_TIME;
			return;
		}
		schedul();
	}
	*((volatile DWORD*)addr) = argv[ECX_ID];
	sti();
	argv[EAX_ID] = NO_ERROR;
}

#ifdef DEBUG

/*�ں˼�����*/
void ApiDebug(DWORD *argv)
{
	void *addr;
	DWORD data[MSG_DATA_LEN];

	addr = (void*)argv[ESI_ID];
	if (addr < UADDR_OFF || addr > (void*)(0 - sizeof(data)))
	{
		argv[EAX_ID] = KERR_INVALID_MEMARGS_ADDR;
		return;
	}
	KPrint((const char *)addr);
	CurPmd->CurTmd->attr &= (~THED_ATTR_APPS);	/*��ֹ�����û��ڴ�ʱ����ҳ�쳣,���½���ϵͳ����̬*/
}

#endif
