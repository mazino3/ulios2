;	uliboot.asm for ulios
;	���ߣ�����
;	���ܣ���ULIFS�ļ�ϵͳ���̱�����������ulildr��ִ��
;	����޸����ڣ�2009-05-19
;	��ע��ʹ��NASM����

;������������ַ
BaseOfLoader	equ	0x9000	;loader�ε�ַ
OffsetOfLoader	equ	0x1000	;loader����ƫ��
DRV_num		equ	0x0000	;1�ֽ���������
DRV_count	equ	0x0001	;1�ֽ���������
OffsetOfBPB	equ	0x0004	;BPB����

[BITS 16]
[ORG 0x7C00]
	jmp	short	start
	times	4 - ($ - $$)	DB	0
;----------------------------------------
;ULIFS������¼����(��ʽ��ʱ�Զ�����)
BPB:
BS_OEMName	DB	'TUX soft'	;0004h OEM ID(tian&uli2k_X)
BPB_fsid	DB	'ULTN'		;�ļ�ϵͳ��־"ULTN",�ļ�ϵͳ����:0x7C
BPB_ver		DW	0		;�汾��0,����Ϊ0�汾��BPB
BPB_bps		DW	512		;ÿ�����ֽ���512
BPB_spc		DW	2		;ÿ��������
BPB_res		DW	8		;����������,����������¼
BPB_secoff	DD	3459519		;�����ڴ����ϵ���ʼ����ƫ��
BPB_seccou	DD	774081		;����ռ��������,����ʣ������
BPB_spbm	DW	95		;ÿ��λͼռ������
BPB_cluoff	DW	198		;�������״ؿ�ʼ����ƫ��
BPB_clucou	DD	386941		;���ݴ���Ŀ
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
	mov	cx,	SizeOfBPB / 2
	rep
	movsw	;DS:SI���Ƶ�ES:DI

	;�������ݶ�
	mov	ds,	ax

	;ȡ������������
	mov	[DRV_num],	dl	;������������
	mov	ah,	8
	int	0x13
	jc	short	ShowDrvErr
	mov	[DRV_count],	dl	;��������

	;��ȡulildr
	mov	byte	[DAP_size],	0x10
	mov	byte	[DAP_res],	0
	mov	ax,	[BPB_res - $$]
	dec	ax			;��ȥ����������1��
	mov	word	[DAP_count],	ax
	mov	dword	[DAP_BufAddr],	BaseOfLoader * 0x10000 + OffsetOfLoader
	mov	eax,	[BPB_secoff - $$]
	inc	eax			;��������������1��
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

	;��ת��ulildrִ��
	jmp	BaseOfLoader:OffsetOfLoader
;----------------------------------------
ShowDrvErr:	;��ʾ���̴���
	mov	ax,	cs
	mov	ds,	ax
	mov	si,	DrvErrMsg
	call	Print
	jmp	short	$
DrvErrMsg	DB	" Disk Error!", 0
;----------------------------------------
ShowDrvExtErr:	;��ʾ������չINT13������
	mov	ax,	cs
	mov	ds,	ax
	mov	si,	DrvExtErrMsg
	call	Print
	jmp	short	$
DrvExtErrMsg	DB	" Disk Ext INT 13 read Error!", 0
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
LdrMsg		DB	"Loading ulildr...", 0
DoneMsg		DB	" Done", 13, 10, 0
;----------------------------------------
	times	510 - ($ - $$)	DB	0
	DW	0xAA55
