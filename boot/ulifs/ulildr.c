/*	ulildr.c for ulios
	���ߣ�����
	���ܣ���ULIFS�ļ�ϵͳ�����������ں�
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

typedef struct _BPB
{
	BYTE	DRV_num;	/*��������*/
	BYTE	DRV_count;	/*��������*/
	BYTE	DRV_res[2];	/*����*/
	BYTE	BS_OEMName[8];	/*OEM ID(tian&uli2k_X)*/
	BYTE	fsid[4];/*�ļ�ϵͳ��־"ULTN",�ļ�ϵͳ����:0x7C*/
	WORD	ver;	/*�汾��0,����Ϊ0�汾��BPB*/
	WORD	bps;	/*ÿ�����ֽ���*/
	WORD	spc;	/*ÿ��������*/
	WORD	res;	/*����������,����������¼*/
	DWORD	secoff;	/*�����ڴ����ϵ���ʼ����ƫ��*/
	DWORD	seccou;	/*����ռ��������,����ʣ������*/
	WORD	spbm;	/*ÿ��λͼռ������*/
	WORD	cluoff;	/*�������״ؿ�ʼ����ƫ��*/
	DWORD	clucou;	/*���ݴ���Ŀ*/
	BYTE	BootPath[88];/*�����б��ļ�·��*/
}BPB;	/*ULIFS������¼����*/

typedef struct _BLKID
{
	DWORD fst;	/*�״�*/
	DWORD cou;	/*����*/
}BLKID;	/*�������ڵ�*/

typedef struct _DIR
{
	BYTE name[80];	/*utf8�ַ���*/
	DWORD CreateTime;	/*����ʱ��1970-01-01����������*/
	DWORD ModifyTime;	/*�޸�ʱ��*/
	DWORD AccessTime;	/*����ʱ��*/
	DWORD attr;		/*����*/
	DWORD len[2];	/*�ļ�����,��FAT32��ͬĿ¼�ļ��ĳ�����Ч*/
	BLKID idx[3];	/*�����3���Դ���������,����һ���ļ��п����ò���������*/
}DIR;	/*ULIFSĿ¼��ṹ*/

#define BUF_COU		0x4000							/*���ݻ�������*/
#define IDX_COU		(BUF_COU / sizeof(BLKID))		/*������������*/

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

WORD strcmp(BYTE *str1, BYTE *str2)
{
	while (*str1 == *str2)
	{
		if (*str1 == 0)
			return 0;
		str1++;
		str2++;
	}
	return 1;
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
				BLKID *idx,			/*�޸ģ���������*/
				DIR *SrcDir,		/*���룺�ļ�Ŀ¼��*/
				DWORD BufferAddr)	/*������������Զ��ַ*/
{
	WORD idxcou, idxi, idxblkcou;	/*ÿ�ζ�ȡ�������ڵ���,�����ڵ�����,ÿ�ζ�ȡ������������*/
	DWORD idxfstblk, brd;	/*������������,�Ѷ�ȡ�ֽ���*/

	if (SrcDir->len[0] == 0)	/*���ļ�*/
		return BufferAddr;
	idxcou = bpb->bps / sizeof(BLKID) * bpb->spc;
	if (idxcou > IDX_COU)	/*���峤��*/
		idxcou = IDX_COU;
	idxi = idxcou - 3;
	idxblkcou = idxcou * sizeof(BLKID) / bpb->bps;
	brd = 0;
	memcpy((BYTE *)(idx + idxi), (BYTE *)(SrcDir->idx), sizeof(BLKID) * 3);	/*����Ŀ¼���е�����*/
	for (;;)
	{
		for (;;)	/*����������*/
		{
			WORD blkcou, i;
			DWORD fstblk;
			if (idxi >= idxcou)	/*��ȡ��һ����*/
			{
				idxfstblk += idxblkcou;
				break;
			}
			else if ((blkcou = idx[idxi].cou) == 0)	/*��ȡ��һ������*/
			{
				idxfstblk = bpb->secoff + bpb->cluoff + bpb->spc * idx[idxi].fst;
				break;
			}
			blkcou *= bpb->spc;	/*ȡ��Ҫ��ȡ��������*/
			fstblk = bpb->secoff + bpb->cluoff + bpb->spc * idx[idxi].fst;	/*ȡ��Ҫ��ȡ����������*/
			for (i = 0; i < blkcou; i++)
			{
				ReadSector(bpb->DRV_num, fstblk, 1, BufferAddr);	/*��ȡ��������*/
				PutChar('.');
				fstblk++;
				BufferAddr += bpb->bps;
				brd += bpb->bps;
				if (brd >= SrcDir->len[0])	/*��ȡ���*/
					return BufferAddr;
			}
			idxi++;
		}
		ReadSector(bpb->DRV_num, idxfstblk, idxblkcou, FAR2LINE((DWORD)((void far *)idx)));
		idxi = 0;
	}
}

/*����Ŀ¼��*/
WORD SearchDir(	BPB *bpb,			/*���룺����BPB*/
				BLKID *idx,			/*�޸ģ���������*/
				BYTE *buf,			/*�޸ģ����ݻ���*/
				DIR *SrcDir,		/*���룺������Ŀ¼*/
				BYTE *FileName,		/*���룺��������Ŀ¼������*/
				BOOL isDir,			/*���룺�Ƿ���Ŀ¼*/
				DIR *DstDir)		/*������ҵ���Ŀ¼��*/
{
	WORD idxcou, idxi, idxblkcou;	/*ÿ�ζ�ȡ�������ڵ����������ڵ�������ÿ�ζ�ȡ������������*/
	DWORD idxfstblk, brd;	/*���������������Ѷ�ȡ�ֽ���*/

	if (SrcDir->len[0] == 0)	/*��Ŀ¼*/
		return NOT_FOUND;
	idxcou = bpb->bps / sizeof(BLKID) * bpb->spc;
	if (idxcou > IDX_COU)	/*���峤��*/
		idxcou = IDX_COU;
	idxi = idxcou - 3;
	idxblkcou = idxcou * sizeof(BLKID) / bpb->bps;
	brd = 0;
	memcpy((BYTE *)(idx + idxi), (BYTE *)(SrcDir->idx), sizeof(BLKID) * 3);	/*����Ŀ¼���е�����*/
	for (;;)
	{
		for (;;)	/*����������*/
		{
			WORD blkcou, i;
			DWORD fstblk;

			if (idxi >= idxcou)	/*��ȡ��һ����*/
			{
				idxfstblk += idxblkcou;
				break;
			}
			else if ((blkcou = idx[idxi].cou) == 0)	/*��ȡ��һ������*/
			{
				idxfstblk = bpb->secoff + bpb->cluoff + bpb->spc * idx[idxi].fst;
				break;
			}
			blkcou *= bpb->spc;	/*ȡ��Ҫ��ȡ��������*/
			fstblk = bpb->secoff + bpb->cluoff + bpb->spc * idx[idxi].fst;	/*ȡ��Ҫ��ȡ����������*/
			for (i = 0; i < blkcou; i += idxblkcou)
			{
				DIR *CurDir;

				ReadSector(bpb->DRV_num, fstblk, idxblkcou, FAR2LINE((DWORD)((void far *)buf)));	/*��ȡ��������*/
				for (CurDir = (DIR *)buf; CurDir < (DIR *)(buf + bpb->bps * idxblkcou); CurDir++)
					if (strcmp(CurDir->name, FileName) == 0)	/*�����ɹ�*/
						if ((isDir && (CurDir->attr & 0x10)) || (!isDir && !(CurDir->attr & 0x10)))
						{
							*DstDir = *CurDir;	/*�����ҵ���Ŀ¼��*/
							return NO_ERROR;
						}
				brd += bpb->bps * idxblkcou;
				if (brd >= SrcDir->len[0])	/*��ȡ���*/
					return NOT_FOUND;
				fstblk += idxblkcou;
			}
			idxi++;
		}
		ReadSector(bpb->DRV_num, idxfstblk, idxblkcou, FAR2LINE((DWORD)((void far *)idx)));
		idxi = 0;
	}
}

/*��·����ȡ�����ļ�*/
DWORD ReadPath(	BPB *bpb,			/*���룺����BPB*/
				BLKID *idx,			/*�޸ģ���������*/
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

void memzero(BYTE far *dst, WORD size)
{
	while (size--)
		*dst++ = 0;
}

/*Loader����������,����Ϊ�Զ������*/
void main(BPB *bpb)
{
	BLKID idx[IDX_COU];	/*16K�ֽ���������*/
	BYTE buf[BUF_COU];	/*16K�ֽ����ݻ���*/
	DIR RootDir, SrcDir, DstDir;	/*Ŀ¼���*/
	BYTE BootList[4096], *cmd;	/*�����б�*/
	DWORD addr = 0x10000, *BinAddr = BIN_ADDR, end;	/*�ں˴��λ��, ���������ָ��*/
	DWORD *VesaMode = VESA_MODE;

	if (bpb->bps > BUF_COU)	/*���ÿ�����ֽ����Ƿ�Ϸ�*/
		goto errip;
	ReadSector(bpb->DRV_num, bpb->secoff + bpb->cluoff, 1, FAR2LINE((DWORD)((void far *)buf)));	/*��ȡ��Ŀ¼����������*/
	SrcDir = RootDir = *((DIR*)buf);
	end = ReadPath(bpb, idx, buf, &SrcDir, &DstDir, bpb->BootPath, FAR2LINE((DWORD)((void far *)BootList)));
	if (end == 0)
		goto errfnf;
	if (end == FAR2LINE((DWORD)((void far *)BootList)))
		goto errfie;
	cmd = BootList;
	BootList[DstDir.len[0]] = 0;
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
				end = ReadPath(bpb, idx, buf, &SrcDir, &DstDir, cmd, addr);
				if (end == 0)
					goto errfnf;
				if (end == addr)
					break;
				end = (end + 0x00000FFF) & 0xFFFFF000;		/*������4K�߽�*/
				DstDir.len[0] = (0x1000 - (DstDir.len[0] & 0xFFF)) & 0xFFF;
				memzero(LINE2FAR(end - DstDir.len[0]), (WORD)DstDir.len[0]);	/*����ļ�β�ڴ�*/
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
