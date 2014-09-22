#include "mathex.h"

namespace wyc
{

// sin/cos look up table
static float g_xmath_sintab[361];
static float g_xmath_costab[361];

void build_sincos_table()
{
	// 0 ~ 360 degree
	float tmp;
	for(int ang=0;ang<=360;++ang)
	{
		tmp=DEG_TO_RAD(ang);
		g_xmath_sintab[ang]=sin(tmp);
		g_xmath_costab[ang]=cos(tmp);
	}
}

float fast_sin(float ang)
{
	ang=fmodf(ang,360);
	if(ang<0)
		ang+=360;
	int alpha=(int)ang;
	float beta=ang-alpha;
	return (g_xmath_sintab[alpha]+beta*(g_xmath_sintab[alpha+1]-g_xmath_sintab[alpha]));
}

float fast_cos(float ang)
{
	ang=fmodf(ang,360);
	if(ang<0)
		ang+=360;
	int alpha=(int)ang;
	float beta=ang-alpha;
	return (g_xmath_costab[alpha]+beta*(g_xmath_costab[alpha+1]-g_xmath_costab[alpha]));
}

void clamp(float32_t &f)
{
	FLOATBITS tmp;
	tmp.fval=f;
	tmp.ival&=~(tmp.ival>>31);
	tmp.fval-=1.0f;
	tmp.ival&=tmp.ival>>31;
	f=tmp.fval+1.0f;
}

float32_t mod(float32_t a, float32_t b)
{
	*(int32_t*)(&b)&=0x7FFFFFFF;
	int32_t n=int32_t(a/b);
	a-=n*b;
	if(a<0)
		a+=b;
	return a;
}

float32_t fast_invsqrt(float32_t x)
{
	float32_t xhalf=0.5f*x;
	int32_t i=*(int*)&x;
	i=0x5f37642f-(i>>1);
	x=*(float32_t*)&i;
	x=x*(1.5f-xhalf*x*x);
	return x;
}

float32_t fast_sqrt(float32_t x)
{
	FLOATBITS convertor, convertor2;
	convertor.fval = x;
	convertor2.fval = x;
	convertor.ival = 0x1FBCF800 + (convertor.ival >> 1);
	convertor2.ival = 0x5f37642f - (convertor2.ival >> 1);
	return 0.5f*(convertor.fval + (x * convertor2.fval));
}

} // namespace wyc

