;	f32boot.asm for ulios
;	���ߣ�����
;	���ܣ���FAT32�ļ�ϵͳ���̱�����������f32ldr��ִ��
;	����޸����ڣ�2009-05-19
;	��ע��ʹ��NASM����

;������������ַ
BaseOfLoader	equ	0x9000	;loader�ε�ַ
OffsetOfLoader	equ	0x1000	;loader����ƫ��
DRV_num		equ	0x0000	;1�ֽ���������
DRV_count	equ	0x0001	;1�ֽ���������
OffsetOfBPB	equ	0x0003	;BPB����

[BITS 16]
[ORG 0x7C00]
	jmp	short	start
	times	3 - ($ - $$)	DB	0
;----------------------------------------
;FAT32������¼����(��ʽ��ʱ�Զ�����)
BPB:
BS_OEMName	DB	'TUX soft'	;0003h OEM ID(tian&uli2k_X)
BPB_bps		DW	512		;ÿ�����ֽ���
BPB_spc		DB	4		;ÿ��������
BPB_res		DW	32		;����������
BPB_nf		DB	2		;FAT��
BPB_nd		DW	0		;��Ŀ¼����(FAT32����)
BPB_sms		DW	0		;С������(FAT32����)
BPB_md		DB	248		;ý��������
BPB_spf16	DW	0		;ÿFAT������(FAT32����)
BPB_spt		DW	63		;ÿ��������
BPB_nh		DW	128		;��ͷ��
BPB_hs		DD	63		;����������
BPB_ls		DD	774081		;��������
BPB_spf		DD	1508		;ÿFAT������(FAT32ר��)
BPB_ef		DW	0		;��չ��־(FAT32ר��)
BPB_fv		DW	0		;�ļ�ϵͳ�汾(FAT32ר��)
BPB_rcn		DD	2226		;��Ŀ¼�غ�(FAT32ר��)
BPB_fsis	DW	1		;�ļ�ϵͳ��Ϣ������(FAT32ר��)
BPB_backup	DW	6		;������������(FAT32ר��)
BPB_res1	DD	0, 0, 0		;����(FAT32ר��)
;FAT32��չ������¼����(��ʽ��ʱ�Զ�����)
BPB_pdn		DB	128		;������������
BPB_res2	DB	0		;����
BPB_ebs		DB	41		;��չ������ǩ
BPB_vsn		DD	1256925113	;�������
BPB_vl	times	11	DB	0	;���
BPB_sid		DB	'FAT32   '	;ϵͳID
;ULIOS��չ������¼����(��ʽ��ʱ�Զ�����)
BPB_ldroff	DW	8		;ulios���������������ƫ��
BPB_secoff	DD	2685375		;�����ڴ����ϵ���ʼ����ƫ��
BPB_BootPath	DB	'ulios/bootlist';�����б��ļ�·��
	times	128 - ($ - $$)	DB	0
SizeOfBPB	equ	$ - BPB
;----------------------------------------
;��չINT13H���̵�ַ���ݰ�
DAP		equ	OffsetOfBPB + SizeOfBPB
DAP_size	equ	DAP		;���ݰ��ߴ�=16
DAP_res		equ	DAP_size + 1	;0
DAP_count	equ	DAP_res + 1	;Ҫ�������������
DAP_BufAddr	equ	DAP_count + 2	;���仺���ַ(segment:offset)
DAP_BlkAddr	equ	DAP_BufAddr + 4	;������ʼ���Կ��ַ
;----------------------------------------
start:	;���ö�
	mov	ax,	cs
	mov	ds,	ax
	mov	es,	ax
	mov	ss,	ax
	mov	sp,	0x7C00

	;��ʾ��Ϣ
	mov	si,	LdrMsg
	call	Print

	;����BPB����
	mov	si,	BPB		;Դƫ��
	mov	ax,	BaseOfLoader	;Ŀ�Ķ�
	mov	es,	ax
	mov	di,	OffsetOfBPB	;Ŀ��ƫ��
	mov	cx,	SizeOfBPB
	rep
	movsb	;DS:SI���Ƶ�ES:DI

	;�������ݶ�
	mov	ds,	ax

	;ȡ������������
	mov	[DRV_num],	dl	;������������
	mov	ah,	8
	int	0x13
	jc	short	ShowDrvErr
	mov	[DRV_count],	dl	;��������

	;��ȡf32ldr
	mov	byte	[DAP_size],	0x10
	mov	byte	[DAP_res],	0
	xor	edx,	edx
	mov	dx,	[BPB_ldroff - $$]
	mov	ax,	[BPB_res - $$]
	sub	ax,	dx	;��ȥldr��ǰ������
	mov	word	[DAP_count],	ax
	mov	dword	[DAP_BufAddr],	BaseOfLoader * 0x10000 + OffsetOfLoader
	mov	eax,	[BPB_secoff - $$]
	add	eax,	edx	;����ldr��ǰ������
	mov	dword	[DAP_BlkAddr],	eax
	mov	dword	[DAP_BlkAddr + 4],	0
	mov	ah,	0x42
	mov	dl,	[DRV_num]
	mov	si,	DAP
	int	0x13
	jc	short	ShowDrvExtErr

	;��ʾDone��Ϣ
	mov	ax,	cs
	mov	ds,	ax
	mov	es,	ax
	mov	si,	DoneMsg
	call	Print

	;��ת��f32ldrִ��
	jmp	BaseOfLoader:OffsetOfLoader
;----------------------------------------
ShowDrvErr:	;��ʾ���̴���
	mov	ax,	cs
	mov	ds,	ax
	mov	si,	DrvErrMsg
	call	Print
	jmp	short	$
DrvErrMsg	db	" Disk Error!", 0
;----------------------------------------
ShowDrvExtErr:	;��ʾ������չINT13������
	mov	ax,	cs
	mov	ds,	ax
	mov	si,	DrvExtErrMsg
	call	Print
	jmp	short	$
DrvExtErrMsg	db	" Disk Ext INT 13 read Error!", 0
;----------------------------------------
Print:		;��ʾ��Ϣ����
	mov	ah,	0x0E		;��ʾģʽ
	mov	bx,	0x0007		;��������
PrintNext:
	lodsb
	or	al,	al
	jz	short	PrintEnd
	int	0x10			;��ʾ
	jmp	short	PrintNext
PrintEnd:
	ret
;----------------------------------------
LdrMsg		DB	"Loading f32ldr...", 0
DoneMsg		DB	" Done", 13, 10, 0
;----------------------------------------
	times	510 - ($ - $$)	DB	0
	DW	0xAA55
