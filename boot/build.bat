@ECHO OFF

:���������
nasm -o ..\out\setup\SETUP -f bin setup.asm
nasm -o ..\out\setup\SETUP386 -f bin setup386.asm
nasm -o ..\out\setup\F32BOOT -f bin fat32\f32boot.asm
nasm -o ..\out\setup\ULIBOOT -f bin ulifs\uliboot.asm

:��TASM��c0.asm����ΪC0T.OBJ��΢��ģʽ
C:\TC\TASM C0,C0T /D__TINY__ /MX;
:�ݴ�ԭ����C0T.OBJ���Է�д��
ren C:\TC\LIB\C0T.OBJ C0T.TMP
:�����µ�C0T.OBJ
copy C0T.OBJ C:\TC\LIB
del C0T.OBJ
C:
cd \TC
:��TCC�����16λ���ָ��
TCC -lt -mt -ef32ldr c:\ulios2\boot\fat32\f32ldr.c
copy OUT\F32LDR.COM c:\ulios2\out\setup\F32LDR
del OUT\F32LDR.*
TCC -lt -mt -eulildr c:\ulios2\boot\ulifs\ulildr.c
copy OUT\ULILDR.COM c:\ulios2\out\setup\ULILDR
del OUT\ULILDR.*
:ɾ�����ɵ�C0T.OBJ������ԭ����
del LIB\C0T.OBJ
ren LIB\C0T.TMP C0T.OBJ
cd c:\ulios2\boot
