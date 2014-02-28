#ifndef WYC_HEADER_DRIVER
#define WYC_HEADER_DRIVER

#include <cstdint>

namespace wyc 
{

typedef float float32_t;

// ��������
enum XG_PIXEL_FORMAT
{
	PIXEL_FMT_UNKNOWN=-1,
	PIXEL_FMT_GRAY8=0,
	PIXEL_FMT_GRAY16,
	PIXEL_FMT_INDEX8,
	PIXEL_FMT_INDEX16,
	PIXEL_FMT_RGB555,
	PIXEL_FMT_RGB565,
	PIXEL_FMT_RGB888,
	PIXEL_FMT_RGBA8888,
	PIXEL_FMT_RGBA16,
	PIXEL_FMT_RGBA32,
	PIXEL_FMT_RED,
	PIXEL_FMT_GREEN,
	PIXEL_FMT_BLUE,
	PIXEL_FMT_ALPHA,
	PIXEL_FMT_RGBA32F,
	PIXEL_FMT_RED32F,
	PIXEL_FMT_GREEN32F,
	PIXEL_FMT_BLUE32F,
	PIXEL_FMT_ALPHA32F,
	NUM_PIXEL_FORMAT
};

// Сβ˳��
#define BYTE_ORDER_LITTLE_ENDIAN

// �߽����
#define BOUNDARY_ALIGNMENT 4

// ֱ���ڴ����
#define DIRECT_MEMORY_ACCESS

// ԭ��������������
typedef uint32_t pixel_t;

// ԭ�����ظ�ʽ
#define NATIVE_PIXEL_FORMAT PIXEL_FMT_RGBA8888

#ifdef BYTE_ORDER_LITTLE_ENDIAN // Сβ˳��

// ������������
#define MAKE_COLOR(r,g,b,a) ((uint32_t(a)<<24)|(uint32_t(b)<<16)|(uint32_t(g)<<8)|r)

#define CHANNEL_RED 0
#define CHANNEL_GREEN 1
#define CHANNEL_BLUE 2
#define CHANNEL_ALPHA 3

/*// ͨ������

#define RED_CHANNEL(c) ((c)&0xFF)
#define GREEN_CHANNEL(c) (((c)>>8)&0xFF)
#define BLUE_CHANNEL(c) (((c)>>16)&0xFF)
#define ALPHA_CHANNEL(c) (((c)>>24)&0xFF)

#define SET_RED_CHANNEL(c,chn) (((c)&0xFFFFFF00)|((chn)&0xFF))
#define SET_GREEN_CHANNEL(c,chn) (((c)&0xFFFF00FF)|(((chn)&0xFF)<<8))
#define SET_BLUE_CHANNEL(c,chn) (((c)&0xFF00FFFF)|(((chn)&0xFF)<<16))
#define SET_ALPHA_CHANNEL(c,chn) (((c)&0x00FFFFFF)|(((chn)&0xFF)<<24))
*/

#else // ��β˳��

#define MAKE_COLOR(r,g,b,a) ((uint32_t(r)<<24)|(uint32_t(g)<<16)|(uint32_t(b)<<8)|a)

#define CHANNEL_RED 3
#define CHANNEL_GREEN 2
#define CHANNEL_BLUE 1
#define CHANNEL_ALPHA 0

#endif

#define RED_CHANNEL(c) (((uint8_t*)&(c))[CHANNEL_RED])
#define GREEN_CHANNEL(c) (((uint8_t*)&(c))[CHANNEL_GREEN])
#define BLUE_CHANNEL(c) (((uint8_t*)&(c))[CHANNEL_BLUE])
#define ALPHA_CHANNEL(c) (((uint8_t*)&(c))[CHANNEL_ALPHA])

#define SET_RED_CHANNEL(c,chn) (((uint8_t*)&(c))[CHANNEL_RED]=(chn))
#define SET_GREEN_CHANNEL(c,chn) (((uint8_t*)&(c))[CHANNEL_GREEN]=(chn))
#define SET_BLUE_CHANNEL(c,chn) (((uint8_t*)&(c))[CHANNEL_BLUE]=(chn))
#define SET_ALPHA_CHANNEL(c,chn) (((uint8_t*)&(c))[CHANNEL_ALPHA]=(chn))

typedef void (*xplotter)(void *pctx, void *pdst, void *psrc);

//==�����������Ͷ���===========================================================================

template<XG_PIXEL_FORMAT fmt>
struct pixel_info {
	typedef uint8_t pixel_t;
};

template<>
struct pixel_info<PIXEL_FMT_GRAY16> {
	typedef uint16_t pixel_t;
};

template<>
struct pixel_info<PIXEL_FMT_INDEX16>
{
	typedef uint16_t pixel_t;
};

template<>
struct pixel_info<PIXEL_FMT_RGB555>
{
	typedef uint16_t pixel_t;
};

template<>
struct pixel_info<PIXEL_FMT_RGB565>
{
	typedef uint16_t pixel_t;
};

template<>
struct pixel_info<PIXEL_FMT_RGB888>
{
	typedef uint32_t pixel_t;
};

template<>
struct pixel_info<PIXEL_FMT_RGBA8888>
{
	typedef uint32_t pixel_t;
};

template<>
struct pixel_info<PIXEL_FMT_RGBA16>
{
	typedef uint16_t* pixel_t;
};

template<>
struct pixel_info<PIXEL_FMT_RGBA32>
{
	typedef uint32_t* pixel_t;
};

template<>
struct pixel_info<PIXEL_FMT_RGBA32F>
{
	typedef float32_t* pixel_t;
};

template<>
struct pixel_info<PIXEL_FMT_RED32F>
{
	typedef float32_t pixel_t;
};

template<>
struct pixel_info<PIXEL_FMT_GREEN32F>
{
	typedef float32_t pixel_t;
};

template<>
struct pixel_info<PIXEL_FMT_BLUE32F>
{
	typedef float32_t pixel_t;
};

template<>
struct pixel_info<PIXEL_FMT_ALPHA32F>
{
	typedef float32_t pixel_t;
};

// �������ݴ�С

#define PIXEL_SIZE(pixel_format) sizeof(pixel_info<pixel_format>::pixel_t)

extern unsigned g_pixel_size[NUM_PIXEL_FORMAT];

inline unsigned pixel_size(XG_PIXEL_FORMAT fmt) {
	return g_pixel_size[fmt];
}

// ���ظ�ʽת��

extern xplotter g_translater[NUM_PIXEL_FORMAT];
extern xplotter g_reverse_translater[NUM_PIXEL_FORMAT];

inline xplotter get_translater(XG_PIXEL_FORMAT srcfmt) {
	return g_translater[srcfmt];
}

inline xplotter get_reverse_translater(XG_PIXEL_FORMAT dstfmt) {
	return g_reverse_translater[dstfmt];
}

// ���ط���ģʽ

enum XG_PLOT_MODE
{
	PLOT_MODE_UNKNOWN=-1,
	PLOT_MODE_REPLACE=0,
	// ��ͨ��ģʽ
	PLOT_MODE_RED,
	PLOT_MODE_GREEN,
	PLOT_MODE_BLUE,
	PLOT_MODE_ALPHA,
	// �߼�ģʽ
	PLOT_MODE_ADD,
	PLOT_MODE_AND,
	PLOT_MODE_INVERSE,
	PLOT_MODE_NOT,
	PLOT_MODE_OR,
	PLOT_MODE_SUB,
	PLOT_MODE_XOR,
	// alpha���
	PLOT_MODE_BLEND,
	NUM_PLOT_MODE
};

extern xplotter g_plotmode[NUM_PLOT_MODE];

// ��ֱ��д������ȣ�ͨ��plotter����һ����������ֻ�ή��1%���ҵ�����
inline xplotter get_plotter(XG_PLOT_MODE mode) {
	return g_plotmode[mode];
}

}; // namespace wyc

#endif // end of WYC_HEADER_DRIVER
