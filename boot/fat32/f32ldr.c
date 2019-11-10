/*	f32ldr.c for ulios
	���ߣ�����
	���ܣ���FAT32�ļ�ϵͳ�����������ں�
	����޸����ڣ�2009-10-30
	��ע��ʹ��Turbo C TCC�����������16λCOM�ļ�
*/
typedef unsigned short	BOOL;
typedef unsigned char	BYTE;
typedef unsigned short	WORD;
typedef unsigned long	DWORD;

#define FAR2LINE(addr)	((WORD)(addr) + (((addr) & 0xFFFF0000) >> 12))
#define LINE2FAR(addr)	((WORD)(addr) | (((addr) & 0xFFFF0000) << 12))

typedef struct _DAP
{
	BYTE	PacketSize;	/*���ݰ��ߴ�=16*/
	BYTE	Reserved;	/*0*/
	WORD	BlockCount;	/*Ҫ�����������*/
	DWORD	BufferAddr;	/*���仺���ַ(segment:offset)*/
	DWORD	BlockAddr[2];/*������ʼ���Կ��ַ*/
}DAP;	/*���̵�ַ���ݰ�*/

typedef struct _BPB0
{
	BYTE	DRV_num;	/*��������*/
	BYTE	DRV_count;	/*��������*/
	BYTE	DRV_res;	/*����*/
	BYTE	BS_OEMName[8];	/*OEM ID(tian&uli2k_X)*/
	WORD	bps;	/*ÿ�����ֽ���*/
	BYTE	spc;	/*ÿ��������*/
	WORD	res;	/*����������*/
	BYTE	nf;		/*FAT��*/
	WORD	nd;		/*��Ŀ¼����*/
	WORD	sms;	/*С������(FAT32����)*/
	BYTE	md;		/*ý��������*/
	WORD	spf16;	/*ÿFAT������(FAT32����)*/
	WORD	spt;	/*ÿ��������*/
	WORD	nh;		/*��ͷ��*/
	DWORD	hs;		/*����������*/
	DWORD	ls;		/*��������*/
	DWORD	spf;	/*ÿFAT������(FAT32ר��)*/
	WORD	ef;		/*��չ��־(FAT32ר��)*/
	WORD	fv;		/*�ļ�ϵͳ�汾(FAT32ר��)*/
	DWORD	rcn;	/*��Ŀ¼�غ�(FAT32ר��)*/
	WORD	fsis;	/*�ļ�ϵͳ��Ϣ������(FAT32ר��)*/
	WORD	backup;	/*������������(FAT32ר��)*/
	DWORD	res1[3];/*����(FAT32ר��)*/

	BYTE	pdn;	/*������������*/
	BYTE	res2;	/*����*/
	BYTE	ebs;	/*��չ������ǩ*/
	DWORD	vsn;	/*�������*/
	BYTE	vl[11];	/*���*/
	BYTE	sid[8];	/*ϵͳID*/

	WORD	ldroff;	/*ulios���������������ƫ��*/
	DWORD	secoff;	/*�����ڴ����ϵ���ʼ����ƫ��*/
	BYTE	BootPath[32];/*�����б��ļ�·��*/
}BPB0;	/*FAT32������¼ԭʼ����*/

typedef struct _BPB
{
	BYTE	DRV_num;	/*��������*/
	BYTE	DRV_count;	/*��������*/
	WORD	bps;	/*ÿ�����ֽ���*/
	WORD	spc;	/*ÿ��������*/
	WORD	res;	/*����������,����������¼*/
	DWORD	secoff;	/*�����ڴ����ϵ���ʼ����ƫ��*/
	DWORD	cluoff;	/*�������״ؿ�ʼ����ƫ��*/
}BPB;	/*FAT32������¼����*/

typedef struct _DIR
{
	BYTE name[11];	/*�ļ���*/
	BYTE attr;		/*����*/
	BYTE reserved;	/*����*/
	BYTE crtmils;	/*����ʱ��10����λ*/
	WORD crttime;	/*����ʱ��*/
	WORD crtdate;	/*��������*/
	WORD acsdate;	/*��������*/
	WORD idxh;		/*�״ظ�16λ*/
	WORD chgtime;	/*�޸�ʱ��*/
	WORD chgdate;	/*�޸�����*/
	WORD idxl;		/*�״ص�16λ*/
	DWORD len;		/*����*/
}DIR;	/*FAT32Ŀ¼��ṹ*/

#define BUF_COU		0x4000							/*���ݻ�������*/
#define IDX_COU		(BUF_COU / sizeof(DWORD))		/*������������*/

#define SETUP		((void (*)())0x1A00)			/*Setup����λ��*/
#define BIN_ADDR	((DWORD *)0x0080)				/*Bin�������������*/
#define SYS_DIR		((BYTE *)0x0280)				/*ϵͳĿ¼�ַ���*/
#define VESA_MODE	((DWORD *)0x02FC)				/*����VESAģʽ��*/

#define TRUE	1
#define FALSE	0

#define NO_ERROR	0
#define NOT_FOUND	1

void memcpy(BYTE *dst, BYTE *src, WORD size)
{
	while (size--)
		*dst++ = *src++;
}

void strcpy(BYTE *dst, BYTE *src)
{
	while ((*dst++ = *src++) != '\0');
}

WORD namecmp(BYTE *name, BYTE *str)
{
	BYTE newnam[11], *namep;

	for (namep = newnam; namep < newnam + 11; namep++)
		*namep = ' ';
	for (namep = newnam; *str != '.' && *str; namep++, str++)
	{
		if (namep >= newnam + 8)
			return 1;
		if (*str == 0xE5 && namep == newnam)
			*namep = 0x05;
		else if (*str >= 'a' && *str <= 'z')
			*namep = *str - 0x20;
		else
			*namep = *str;
	}
	if (*str == '.')
		str++;
	for (namep = newnam + 8; *str; namep++, str++)
	{
		if (namep >= newnam + 11)
			return 1;
		if (*str >= 'a' && *str <= 'z')
			*namep = *str - 0x20;
		else
			*namep = *str;
	}
	for (namep = newnam; namep < newnam + 11; namep++, name++)
		if (*namep != *name)
			return 1;
	return 0;
}

DWORD atol(BYTE *str)
{
	DWORD i;

	for (i = 0; *str >= '0' && *str <= '9'; str++)
		i = i * 10 + *str - '0';
	return i;
}

void PutChar(BYTE c)
{
	_AL = c;
	_AH = 0x0E;
	_BX = 0x0007;
	asm int 10h;
}

void PutS(BYTE *str)
{
	while (*str)
	{
		_AL = *str;
		_AH = 0x0E;
		_BX = 0x0007;
		asm int 10h;
		str++;
	}
}

/*��ȡ����*/
void ReadSector(BYTE DrvNum,		/*���룺��������*/
				DWORD BlockAddr,	/*���룺��ʼ������*/
				WORD BlockCount,	/*���룺������*/
				DWORD BufferAddr)	/*���������������Ե�ַ*/
{
	DAP dap;

	dap.PacketSize = 0x10;
	dap.Reserved = 0;
	dap.BlockCount = BlockCount;
	dap.BufferAddr = LINE2FAR(BufferAddr);
	dap.BlockAddr[0] = BlockAddr;
	dap.BlockAddr[1] = 0;

	_AH = 0x42;
	_DL = DrvNum;
	_SI = (WORD)(&dap);
	asm int 13h;
	asm jc short ReadError;
	return;
ReadError:	/*ע�⣺��չINT13���ܲ��ᴦ��DMA�߽���󣬵��ñ�����ʱ��ע�����*/
	PutS(" INT13 ERROR!");
	for (;;);
}

/*��ȡ�����ļ�*/
DWORD ReadFile(	BPB *bpb,			/*���룺����BPB*/
				DWORD *idx,			/*�޸ģ���������*/
				DIR *SrcDir,		/*���룺�ļ�Ŀ¼��*/
				DWORD BufferAddr)	/*������������Զ��ַ*/
{
	DWORD prei, clui, brd;	/*�غż���,��һ�غ�,�Ѷ�ȡ�ֽ���*/

	if (SrcDir->len == 0)	/*���ļ�*/
		return BufferAddr;
	prei = 0xFFFFFFFF;
	clui = ((DWORD)(SrcDir->idxh) << 16) | SrcDir->idxl;
	brd = 0;
	for (;;)
	{
		DWORD i, fstblk;

		fstblk = bpb->secoff + bpb->cluoff + bpb->spc * clui;	/*ȡ��Ҫ��ȡ����������*/
		for (i = 0; i < bpb->spc; i++)
		{
			ReadSector(bpb->DRV_num, fstblk, 1, BufferAddr);	/*��ȡ��������*/
			PutChar('.');
			fstblk++;
			BufferAddr += bpb->bps;
			brd += bpb->bps;
			if (brd >= SrcDir->len)	/*��ȡ���*/
				return BufferAddr;
		}
		if ((i = clui / (bpb->bps >> 2)) != prei / (bpb->bps >> 2))	/*��һ�غŲ��ڻ�����*/
			ReadSector(bpb->DRV_num, bpb->secoff + bpb->res + i, 1, FAR2LINE((DWORD)((void far *)idx)));
		prei = clui;
		clui = idx[clui % (bpb->bps >> 2)];
	}
}

/*����Ŀ¼��*/
WORD SearchDir(	BPB *bpb,			/*���룺����BPB*/
				DWORD *idx,			/*�޸ģ���������*/
				BYTE *buf,			/*�޸ģ����ݻ���*/
				DIR *SrcDir,		/*���룺������Ŀ¼*/
				BYTE *FileName,		/*���룺��������Ŀ¼������*/
				BOOL isDir,			/*���룺�Ƿ���Ŀ¼*/
				DIR *DstDir)		/*������ҵ���Ŀ¼��*/
{
	DWORD prei, clui;/*�غż���,��һ�غ�*/

	prei = 0xFFFFFFFF;
	clui = ((DWORD)(SrcDir->idxh) << 16) | SrcDir->idxl;
	for (;;)
	{
		DWORD i;
		DWORD fstblk;

		fstblk = bpb->secoff + bpb->cluoff + bpb->spc * clui;	/*ȡ��Ҫ��ȡ����������*/
		for (i = 0; i < bpb->spc; i++)
		{
			DIR *CurDir;

			ReadSector(bpb->DRV_num, fstblk, 1, FAR2LINE((DWORD)((void far *)buf)));	/*��ȡ��������*/
			for (CurDir = (DIR *)buf; CurDir < (DIR *)(buf + bpb->bps); CurDir++)
				if (namecmp(CurDir->name, FileName) == 0)	/*�����ɹ�*/
					if ((isDir && (CurDir->attr & 0x10)) || (!isDir && !(CurDir->attr & 0x18)))
					{
						*DstDir = *CurDir;	/*�����ҵ���Ŀ¼��*/
						return NO_ERROR;
					}
			fstblk++;
		}
		if ((i = clui / (bpb->bps >> 2)) != prei / (bpb->bps >> 2))	/*��һ�غŲ��ڻ�����*/
			ReadSector(bpb->DRV_num, bpb->secoff + bpb->res + i, 1, FAR2LINE((DWORD)((void far *)idx)));
		prei = clui;
		if ((clui = idx[clui % (bpb->bps >> 2)] & 0x0FFFFFFF) >= 0x0FFFFFF8)	/*�ļ�����*/
			return NOT_FOUND;
	}
}

/*��·����ȡ�����ļ�*/
DWORD ReadPath(	BPB *bpb,			/*���룺����BPB*/
				DWORD *idx,			/*�޸ģ���������*/
				BYTE *buf,			/*�޸ģ����ݻ���*/
				DIR *SrcDir,		/*�޸ģ��ļ�Ŀ¼��*/
				DIR *DstDir,		/*�޸ģ�Ŀ��Ŀ¼��*/
				BYTE *path,			/*���룺�ļ�·��*/
				DWORD BufferAddr)	/*������������Զ��ַ*/
{
	while (*path)
	{
		BYTE *cp = path;

		while (*cp != '/' && *cp != 0)
			cp++;
		if (*cp)
		{
			*cp++ = 0;
			PutS(path);
			if (SearchDir(bpb, idx, buf, SrcDir, path, TRUE, DstDir) != NO_ERROR)	/*ȡ��Ŀ¼��*/
				return 0;
			*SrcDir = *DstDir;
			path = cp;
			PutChar('/');
		}
		else
		{
			DWORD end;

			PutS(path);
			if (SearchDir(bpb, idx, buf, SrcDir, path, FALSE, DstDir) != NO_ERROR)	/*ȡ���ļ�Ŀ¼��*/
				return 0;
			end = ReadFile(bpb, idx, DstDir, BufferAddr);	/*��ȡ�ļ�*/
			if (end == BufferAddr)
				PutS("<-File is empty!");
			PutChar(13);
			PutChar(10);
			return end;
		}
	}
}

/*Loader����������,����Ϊ�Զ������*/
void main(BPB0 *bpb0)
{
	DWORD idx[IDX_COU];	/*16K�ֽ���������*/
	BYTE buf[BUF_COU];	/*16K�ֽ����ݻ���*/
	BPB bpb;
	DIR RootDir, SrcDir, DstDir;	/*Ŀ¼���*/
	BYTE BootList[4096], *cmd;	/*�����б�*/
	DWORD addr = 0x10000, *BinAddr = BIN_ADDR, end;	/*�ں˴��λ��, ���������ָ��*/
	DWORD *VesaMode = VESA_MODE;

	if (bpb0->bps > BUF_COU)	/*���ÿ�����ֽ����Ƿ�Ϸ�*/
		goto errip;
	bpb.DRV_num = bpb0->DRV_num;	/*��������*/
	bpb.bps = bpb0->bps;	/*ÿ�����ֽ���*/
	bpb.spc = bpb0->spc;	/*ÿ��������*/
	bpb.res = bpb0->res;	/*����������,����������¼*/
	bpb.secoff = bpb0->secoff;	/*�����ڴ����ϵ���ʼ����ƫ��*/
	bpb.cluoff = bpb0->res + (bpb0->spf * bpb0->nf) - (bpb0->spc << 1);	/*�������״ؿ�ʼ����ƫ��*/
	RootDir.attr = 0x10;
	RootDir.idxh = (bpb0->rcn >> 16);
	RootDir.idxl = (WORD)bpb0->rcn;
	RootDir.len = 0;
	SrcDir = RootDir;
	end = ReadPath(&bpb, idx, buf, &SrcDir, &DstDir, bpb0->BootPath, FAR2LINE((DWORD)((void far *)BootList)));
	if (end == 0)
		goto errfnf;
	if (end == FAR2LINE((DWORD)((void far *)BootList)))
		goto errfie;
	cmd = BootList;
	BootList[DstDir.len] = 0;
	*VesaMode = 0;
	for (;;)
	{
		BYTE *cp = cmd;

		while (*cp != '\n' && *cp != 0)
			cp++;
		if (*cp)
			*cp++ = 0;
		switch (*cmd++)
		{
		case 'F':	/*File*/
		case 'f':
			if (BinAddr < BIN_ADDR + 30)	/*15�������ļ�����*/
			{
				SrcDir = RootDir;
				end = ReadPath(&bpb, idx, buf, &SrcDir, &DstDir, cmd, addr);
				if (end == 0)
					goto errfnf;
				if (end == addr)
					break;
				end = (end + 0x00000FFF) & 0xFFFFF000;		/*������4K�߽�*/
				*BinAddr++ = addr;
				*BinAddr++ = end - addr;
				addr = end;
			}
			break;
		case 'S':	/*SysDir*/
		case 's':
			strcpy(SYS_DIR, cmd);
			break;
		case 'V':	/*VesaMode*/
		case 'v':
			*VesaMode = atol(cmd);
			break;
		}
		if (*cp)	/*��������*/
			cmd = cp;
		else
		{
			*BinAddr = 0;
			SETUP();
		}
	}
errip:
	PutS("invalid partition!");
	for (;;);
errfnf:
	PutS("<-File not found!");
errfie:
	for (;;);
}
