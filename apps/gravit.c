/*	gravit.c for ulios application
	���ߣ�����
	���ܣ�3D����������ʾ����
	����޸����ڣ�2016-11-03
*/

#include "../driver/basesrv.h"
#include "../lib/gdi.h"
#include "../lib/math.h"

#define G	6.674e-11	// ������������(������/ǧ��ƽ����)
#define PI	3.1415926535898	// Բ����
#define T	60	// ģ��ʱ�䲽��(��)

struct star
{
	double m, r;	// ����(ǧ��)���뾶(��)
	double s[3];	// λ��/λ��(��)
	double v[3];	// �ٶ�(��/��)
	double a[3];	// ���ٶ�(��/ƽ����)
};

// ����stars����������curi����������
void LocationToGravit(struct star stars[], int len, int curi)
{
	int i;
	struct star* curStar = stars + curi;
	curStar->a[2] = curStar->a[1] = curStar->a[0] = 0.0;
	for (i = 0; i < len; i++)
	{
		if (i != curi)
		{
			double s[3], d2, d, a;
			struct star* starx = stars + i;
			s[0] = starx->s[0] - curStar->s[0];
			s[1] = starx->s[1] - curStar->s[1];
			s[2] = starx->s[2] - curStar->s[2];
			d2 = s[0] * s[0] + s[1] * s[1] + s[2] * s[2];
			d = sqrt(d2);
			a = G * starx->m / d2;
			curStar->a[0] += s[0] / d * a;
			curStar->a[1] += s[1] / d * a;
			curStar->a[2] += s[2] / d * a;
		}
	}
}

// ģ��һ���˶�
void Simulation(struct star stars[], int len, int CalcLen)
{
	int i;
	for (i = 0; i < len; i++)
	{
		double a[3], s[3];
		struct star* curStar = stars + i;
		s[0] = curStar->s[0];	// �ݴ�ԭʼλ��
		s[1] = curStar->s[1];
		s[2] = curStar->s[2];
		LocationToGravit(stars, CalcLen, i);	// �����ʼʱ������
		a[0] = curStar->a[0];	// �ݴ��ʼʱ������
		a[1] = curStar->a[1];
		a[2] = curStar->a[2];
		curStar->s[0] += curStar->v[0] * T + a[0] * T * T * 0.5;	// �������λ��
		curStar->s[1] += curStar->v[1] * T + a[1] * T * T * 0.5;
		curStar->s[2] += curStar->v[2] * T + a[2] * T * T * 0.5;
		LocationToGravit(stars, CalcLen, i);	// �������ʱ������
		curStar->a[0] = (a[0] + curStar->a[0]) * 0.5;	// �ϳɽ�������ֵ
		curStar->a[1] = (a[1] + curStar->a[1]) * 0.5;
		curStar->a[2] = (a[2] + curStar->a[2]) * 0.5;
		curStar->s[0] = s[0];	// �ָ�ԭʼλ��
		curStar->s[1] = s[1];
		curStar->s[2] = s[2];
	}
	for (i = 0; i < len; i++)
	{
		struct star* curStar = stars + i;
		GDIPutPixel(curStar->s[0] / 3e9 + GDIwidth / 2, GDIheight / 2 - curStar->s[1] / 3e9, 0);
		curStar->s[0] += curStar->v[0] * T + curStar->a[0] * T * T * 0.5;	// ����ģ��λ��
		curStar->s[1] += curStar->v[1] * T + curStar->a[1] * T * T * 0.5;
		curStar->s[2] += curStar->v[2] * T + curStar->a[2] * T * T * 0.5;
		GDIPutPixel(curStar->s[0] / 3e9 + GDIwidth / 2, GDIheight / 2 - curStar->s[1] / 3e9, curStar->m > 1000000.0 ? 0xFF00 : 0xFFFFFF);
		curStar->v[0] += curStar->a[0] * T;	// ����ģ���ٶ�
		curStar->v[1] += curStar->a[1] * T;
		curStar->v[2] += curStar->a[2] * T;
	}
}

// �ýǶ������ʼ��λ�ú��ٶ�
void InitStars(struct star stars[], int len, double angle[])
{
	int i;
	for (i = 0; i < len; i++)
	{
		double val, rad;
		struct star* curStar = stars + i;
		rad = angle[i] * PI / 180.0;
		val = curStar->s[0];
		curStar->s[0] = val * cos(rad);
		curStar->s[1] = val * sin(rad);
		val = curStar->v[1];
		curStar->v[0] = val * (-sin(rad));
		curStar->v[1] = val * cos(rad);
	}
}

DWORD RandSeed;

DWORD rand(DWORD x)
{
	RandSeed = RandSeed * 1103515245 + 12345;
	return RandSeed % x;
}

// ���������ʼ��λ�ú��ٶ�
void RandStars(struct star stars[], int len)
{
	int i;
	for (i = 2; i < len; i++) {
		double val, rad;
		struct star* curStar = stars + i;
		curStar->m = 1.0;
		rad = ((double)rand(0x10000000)) * PI * 2 / 0x10000000;
		val = ((double)rand(0x10000000)) * 400e9 / 0x10000000 + 20e9;
		curStar->s[0] = stars[i % 2].s[0] + val * cos(rad);
		curStar->s[1] = stars[i % 2].s[1] + val * sin(rad);
		curStar->s[2] = 0.0;
		val = sqrt(G * stars[i % 2].m / val);
		curStar->v[0] = stars[i % 2].v[0] + val * (-sin(rad));
		curStar->v[1] = stars[i % 2].v[1] + val * cos(rad);
		curStar->v[2] = 0.0;
	}
}

int main()
{
//	struct star solar[] = {
//		{1.9891e30,	696000000,	{0, 0, 0},			{0, 0, 0},			{0, 0, 0}},	// ̫��
//		{3.3022e23,	2440000,	{56.6720e9, 0, 0},	{0, 47.88e3, 0},	{0, 0, 0}},	// ˮ��
//		{4.869e24,	6051800,	{108.2065e9, 0, 0},	{0, 35.02e3, 0},	{0, 0, 0}},	// ����
//		{5.965e24,	6378150,	{149.5791e9, 0, 0},	{0, 29.79e3, 0},	{0, 0, 0}},	// ����
//		{6.4219e23,	3397000,	{226.9024e9, 0, 0},	{0, 24.13e3, 0},	{0, 0, 0}},	// ����
//		{1.900e27,	71492000,	{777.5e9, 0, 0},	{0, 13.06e3, 0},	{0, 0, 0}},	// ľ��
//		{5.688e26,	60268000,	{1424.6317e9, 0, 0},{0, 9.46e3, 0},		{0, 0, 0}},	// ����
//		{8.683e25,	25559000,	{2867.7797e9, 0, 0},{0, 6.81e3, 0},		{0, 0, 0}},	// ������
//		{1.0247e26,	24766000,	{4498.0842e9, 0, 0},{0, 5.44e3, 0},		{0, 0, 0}},	// ������
//		{1.305e22,	1137000,	{5913.52e9, 0, 0},	{0, 4.75e3, 0},		{0, 0, 0}},	// ڤ����
//		{7.349e22,	1738000,	{149.5791e9-384400000, 384400000, 0},	{0, 29.79e3-1023, 0},	{0, 0, 0}}	// ����
//	};
//	double angle[] = {0, 253.78494444, 182.60408333, 100.36890633, 359.44775000, 36.29466667, 45.72225000, 316.41866667, 303.92886111, 100.36890633};	// 2000��1��1��12ʱ�������Ļƾ�
	struct star solar2[2002] = {
		{1.9891e30,	696000000,	{-500e9, -30e9, 0},	{50e3, 0, 0},	{0, 0, 0}},	// ̫��
		{1.9891e30,	696000000,	{500e9, 30e9, 0},	{-50e3, 0, 0},	{0, 0, 0}},	// ̫��
	};
	long res;
	RandSeed = (DWORD)TMGetRand();
//	InitStars(solar, 10, angle);
	RandStars(solar2, 2002);
	if ((res = GDIinit()) != NO_ERROR)
		return res;
	GDIFillRect(0, 0, GDIwidth, GDIheight, 0);
	GDIDrawStr(0, 0, "Welcome to ulios gravit simulation program!", 0xFFFFFF);

	for (;;)
		Simulation(solar2, 2002, 2);
	GDIrelease();
	return NO_ERROR;
}
