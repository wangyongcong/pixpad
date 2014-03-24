#ifndef WYC_HEADER_XRASTER
#define WYC_HEADER_XRASTER

#include <stdint.h>
#include "mathex/vecmath.h"
#include "util/rect.h"
#include "color.h"
#include "driver.h"
#include "render_buffer.h"

namespace wyc
{

struct xpattern
{
	unsigned width;
	unsigned height;
	unsigned cx;
	unsigned cy;
	uint8_t* bitmap;
};

class xraster
{
public:
	// ��դ��״̬
	enum RASTERSTATE
	{
		PEN_ST_WIDTH=0xF,
		PEN_ST_ANTI=0x10,
		PEN_ST_PATTERN=0x20,
		BITBLT_ST_COLORKEY=0x40
	};
	// �ʻ���ʽ
	enum PATTERN_STYLE
	{
		BRUSH_W3=0,
		BRUSH_W5,
		BRUSH_W7,
		NUM_PATTERN
	};

public:
	xraster();
	~xraster();
	// ��������
	bool attach_color_buffer(XG_PIXEL_FORMAT fmt, const xrender_buffer &new_buffer);
	bool attach_color_buffer(XG_PIXEL_FORMAT fmt, const xrender_buffer &sub_buffer, unsigned x, unsigned y, unsigned w, unsigned h);
	// ���������ݷ���
	inline uint8_t* color_buffer() {
		return m_colorBuffer.get_buffer();
	}
	inline uint8_t* get_pixel(int x, int y) {
		return m_colorBuffer.get_elem(x,y);
	}
	inline unsigned pitch() const {
		return m_colorBuffer.pitch();
	}
	inline unsigned width() const {
		return m_colorBuffer.width();
	}
	inline unsigned height() const {
		return m_colorBuffer.height();
	}
	inline unsigned stride() const {
		return m_colorBuffer.size_elem();
	}
	inline XG_PIXEL_FORMAT pixel_format() const {
		return m_pixelfmt;
	}
	// ������ɫ
	inline void set_index(unsigned index) {
		if(m_palette && index<m_colorNum) 
			m_color=m_palette[index];
	}
	inline const pixel_t& get_color() const {
		return m_color;
	}
	inline void set_color(pixel_t &c) {
		m_color=c;
	}
	inline void set_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a=255) {
		m_color=MAKE_COLOR(r,g,b,a);
	}
	inline pixel_t get_bkcolor() const {
		return m_bkcolor;
	}
	inline void set_bkcolor(uint8_t r, uint8_t g, uint8_t b, uint8_t a=255) {
		m_bkcolor=MAKE_COLOR(r,g,b,a);
	}
	void enable_colorkey(bool b) {
		b?add_state(m_state,BITBLT_ST_COLORKEY):remove_state(m_state,BITBLT_ST_COLORKEY);
	}
	bool is_colorkey() const {
		return have_state(m_state,BITBLT_ST_COLORKEY);
	}
	inline pixel_t colorkey() const {
		return m_colorkey;
	}
	inline void set_colorkey(uint8_t r, uint8_t g, uint8_t b, uint8_t a=255) {
		m_colorkey=MAKE_COLOR(r,g,b,a);
	}
	// ���õ�ɫ��
	bool create_palette(unsigned numColor);
	inline void set_palette(int index, const pixel_t &color) {
		m_palette[index]=color;
	}
	inline unsigned get_palette_color_num() const {
		return m_colorNum;
	}
	inline const pixel_t& get_palette_color(unsigned index) const {
		return m_palette[index];
	}
	//  ���û���ģʽ
	void set_plot_mode(XG_PLOT_MODE mode);
	inline XG_PLOT_MODE plot_mode() const {
		return m_plotmode;
	}
	// ���û��ʿ��
	inline void set_pen_width(unsigned wpen) {
		set_state(m_state,PEN_ST_WIDTH,wpen);
	}
	inline unsigned pen_width() const {
		return m_state&PEN_ST_WIDTH;
	}
	// ���������
	inline void enable_anti(bool b) {
		b?add_state(m_state,PEN_ST_ANTI):remove_state(m_state,PEN_ST_ANTI);
	}
	inline bool is_anti() const {
		return have_state(m_state,PEN_ST_ANTI);
	}
	// �����ʻ���ʽ
	inline void enable_pattern(bool b) {
		b?add_state(m_state,PEN_ST_PATTERN):remove_state(m_state,PEN_ST_PATTERN);
	}
	inline bool is_pattern() const {
		return have_state(m_state,PEN_ST_PATTERN);
	}
	// ���ñʻ���ʽ
	void set_pattern(const xpattern &pat);
	inline void set_pattern(PATTERN_STYLE pat) {
		set_pattern(s_PreDefinedPattern[pat]);
	}
	inline const xpattern& cur_pattern() const {
		return m_pattern;
	}
	// ���òü�����
	inline void set_clip_rect(int xmin, int ymin, int xmax, int ymax) {
		m_xmin=MAX(0,xmin), m_ymin=MAX(0,ymin), m_xmax=MIN(int(width()),xmax), m_ymax=MIN(int(height()),ymax);
	}
	inline void set_clip_rect(const xrecti_t &rect) {
		m_xmin=MAX(0,rect.left), m_ymin=MAX(0,rect.top), m_xmax=MIN(int(width()),rect.right), m_ymax=MIN(int(height()),rect.bottom);
	}
	inline void get_clip_rect(xrecti_t &rect) const {
		rect.xmin=m_xmin, rect.ymin=m_ymin, rect.xmax=m_xmax, rect.ymax=m_ymax;
	}
	inline void reset_clip_rect() {
		m_xmin=0, m_ymin=0, m_xmax=width(), m_ymax=height();
	}
	// �ñ���ɫ����
	inline void clear_screen() {
		m_colorBuffer.clear(m_bkcolor);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// ���ػ���
	////////////////////////////////////////////////////////////////////////////////////////////////
	
	// ��ȡָ��λ�ô���������ɫ
	pixel_t read_pixel(unsigned x, unsigned y);
	// ��ȡ������������
	void read_pixel(unsigned x, unsigned y, unsigned w, unsigned h, uint8_t *pret);

	////////////////////////////////////////////////////////////////////////////////////////////////
	// ���ػ���
	////////////////////////////////////////////////////////////////////////////////////////////////
	
	// ����λͼ
	void bitmap(int x, int y, uint8_t *pbits, int pitch, int srcw, int srch, bool trans=false);
	// ���ƻ���������
	void bitblt(int x, int y, int srcx, int srcy, int srcw, int srch);
	// ���������ʽ����
	void bitblt(int x, int y, uint8_t *pbits, XG_PIXEL_FORMAT srcfmt, int pitch, int srcx, int srcy, int srcw, int srch);
	// ��������,�ɽ�������
	void bitblt(int x, int y, int w, int h, uint8_t *pbits, XG_PIXEL_FORMAT srcfmt, int pitch, int srcx, int srcy, int srcw, int srch, int hint=1);
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// ����ͼԪ
	////////////////////////////////////////////////////////////////////////////////////////////////
	
	// ���Ƶ�
	void point(int x, int y);
	// ����ֱ��
	void line(int x0, int y0, int x1, int y1);
	// ���ƾ���
	void rect(int x, int y, int w, int h);
	inline void rect(const xrecti_t area) 
	{
		rect(area.left,area.top,area.width(),area.height());
	}
	// ����Բ�Ǿ���
	void round_rect(int x, int y, int w, int h, int radius);
	inline void round_rect(const xrecti_t area, int radius) 
	{
		round_rect(area.left,area.top,area.width(),area.height(),radius);
	}
	// ����Բ
	void circle(int cx, int cy, int radius);
	// ������Բ
	void ellipse(int cx, int cy, int rx, int ry);
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// ���ͼԪ
	////////////////////////////////////////////////////////////////////////////////////////////////
	
	// ������
	void fill_rect(int x, int y, int w, int h);
	inline void fill_rect(const xrecti_t area) {
		fill_rect(area.left,area.top,area.width(),area.height());
	}
	// ���Բ�Ǿ���
	void fill_round_rect(int x, int y, int w, int h, int radius);
	inline void fill_round_rect(const xrecti_t area, int radius) {
		fill_round_rect(area.left,area.top,area.width(),area.height(),radius);
	}
	// ���Բ
	void fill_circle(int cx, int cy, int radius);
	// �����Բ
	void fill_ellipse(int cx, int cy, int rx, int ry);
	// �����������
	void poly_fill_beg();
	void poly_fill_add(int x, int y);
	void poly_fill_end();
	// �������������
	void flood_fill(int x, int y);

protected:
	////////////////////////////////////////////////////////////////////////////////////////////////
	// �������ƽӿ�: ���еĹ�դ������ʹ��(�ҽ�ʹ��)������Щ�ӿڶ�д��ɫ����
	////////////////////////////////////////////////////////////////////////////////////////////////

	inline pixel_t transparency(float t) 
	{
		// ע��uint8_t*float��uint32_t*float��ܶ�
		pixel_t c=m_color;
		SET_ALPHA_CHANNEL(c,uint8_t(ALPHA_CHANNEL(m_color)*t));
		return c;
	}
	inline void plot(int x, int y) 
	{
		assert(x>=m_xmin && x<m_xmax && y>=m_ymin && y<m_ymax);
		m_plotter(this,m_colorBuffer.get_elem(x,y),&m_color);
	}
	inline void plot(int x, int y, pixel_t c) 
	{
		assert(x>=m_xmin && x<m_xmax && y>=m_ymin && y<m_ymax);
		m_plotter(this,m_colorBuffer.get_elem(x,y),&c);
	}
	inline void plot_s(int x, int y) // safe plot
	{
		if(x>=m_xmin && x<m_xmax && y>=m_ymin && y<m_ymax)
			m_plotter(this,m_colorBuffer.get_elem(x,y),&m_color);
	}
	inline void plot_s(int x, int y, pixel_t c) // safe plot
	{
		if(x>=m_xmin && x<m_xmax && y>=m_ymin && y<m_ymax)
			m_plotter(this,m_colorBuffer.get_elem(x,y),&c);
	}
	void scan_line(int y, int x0, int x1);
	void vert_line(int x, int y0, int y1);

	inline void plot_quadrant(int cx, int cy, int x, int y)
	{
		plot_s(cx+x,cy+y);
		plot_s(cx-x,cy+y);
		plot_s(cx+x,cy-y);
		plot_s(cx-x,cy-y);
	}
	inline void plot_quadrant(int cx, int cy, int x, int y, pixel_t c)
	{
		plot_s(cx+x,cy+y,c);
		plot_s(cx-x,cy+y,c);
		plot_s(cx+x,cy-y,c);
		plot_s(cx-x,cy-y,c);
	}
	inline void plot_octant(int cx, int cy, int x, int y)
	{
		plot_quadrant(cx,cy,x,y);
		plot_quadrant(cx,cy,y,x);
	}
	inline void plot_octant(int cx, int cy, int x, int y, pixel_t c)
	{
		plot_quadrant(cx,cy,x,y,c);
		plot_quadrant(cx,cy,y,x,c);
	}

	void scan_line_s(int y, int x0, int x1);
	void vert_line_s(int x, int y0, int y1);

	void scan_line_w(int y, int x0, int x1, int w);
	void vert_line_w(int x, int y0, int y1, int w);

	void make_pattern(float trans=1.0f);
	void plot_pattern(int x, int y);
	inline void plot_quadrant_pattern(int cx, int cy, int x, int y)
	{
		plot_pattern(cx+x,cy+y);
		plot_pattern(cx-x,cy+y);
		plot_pattern(cx+x,cy-y);
		plot_pattern(cx-x,cy-y);
	}
	inline void plot_octant_pattern(int cx, int cy, int x, int y)
	{
		plot_quadrant_pattern(cx,cy,x,y);
		plot_quadrant_pattern(cx,cy,y,x);
	}
	void scan_line_pattern(int y, int x0, int x1);
	void vert_line_pattern(int x, int y0, int y1);

	////////////////////////////////////////////////////////////////////////////////////////////////
	// ����ͼԪ��դ������
	////////////////////////////////////////////////////////////////////////////////////////////////

	// ����bresenhamֱ��
	void line_bresenham(int x0, int y0, int x1, int y1);
	// �����bresenhamֱ��
	void line_bresenham_anti(int x0, int y0, int x1, int y1);
	// bresenham��ֱ��
	void line_bresenham_fat(int x0, int y0, int x1, int y1, int w);
	// �����bresenham��ֱ��(δʵ��)
//	void line_bresenham_fat_anti(int x0, int y0, int x1, int y1, int w);
	// patternֱ��
	void line_bresenham_pattern(int x0, int y0, int x1, int y1);
	// �㻭��(δʵ��)
//	void line_bresenham_stipple(int x0, int y0, int x1, int y1);
	// XialonWuֱ��
	void line_xw(int x0, int y0, int x1, int y1);
	// XialonWu�Գ�ֱ��
	void line_xw_sym(int x0, int y0, int x1, int y1);

	// ����bresenhamԲ
	void circle_outline(int cx, int cy, int radius);
	// �����bresenhamԲ
	void circle_outline_anti(int cx, int cy, int radius);
	// patternģʽԲ
	void circle_outline_pattern(int cx, int cy, int radius);
	// ���Բ(flatģʽ)
	void fill_circle_flat(int cx, int cy, int radius);
	// ���Բ,ƽ������
	void fill_circle_flat_anti(int cx, int cy, int radius);

	// ����bresenham��Բ
	void ellipse_outline(int cx, int cy, int rx, int ry);
	// ���ƿ����bresenham��Բ
	void ellipse_outline_anti(int cx, int cy, int rx, int ry);
	// patternģʽ��Բ
	void ellipse_outline_pattern(int cx, int cy, int rx, int ry);
	// �����Բ(flatģʽ)
	void fill_ellipse_flat(int cx, int cy, int rx, int ry);
	// �����Բ,ƽ������
	void fill_ellipse_flat_anti(int cx, int cy, int rx, int ry);

	// ����������
	void curve_bezier(int nPoints, const xpt2i_t &beg, const xpt2i_t &ctrl1, const xpt2i_t &ctrl2, const xpt2i_t &end);
	// ������
	void curve_parabola(int nPoints, float hs, float vs, float dt);

	// ����Բ��
	void circle_part(int cx0, int cy0, int cx1, int cy1, int radius);
	// ����Բ��(�����)
	void circle_part_anti(int cx0, int cy0, int cx1, int cy1, int radius);
	// ����Բ��(patternģʽ)
	void circle_part_pattern(int cx0, int cy0, int cx1, int cy1, int radius);
	// ���Բ��
	void fill_circle_part(int cx0, int cy0, int cx1, int cy1, int radius);
	// ����Բ��(��Եƽ��)
	void fill_circle_part_anti(int cx0, int cy0, int cx1, int cy1, int radius);

	////////////////////////////////////////////////////////////////////////////////////////////////
	// ֱ�߲ü�
	////////////////////////////////////////////////////////////////////////////////////////////////

	// ���ڱ�Ǽ��Ĳü��㷨(Cohen-Sutherland)
	bool line_clip_2d_cs(int &x0, int &y0, int &x1, int &y1, const xrecti_t &rect);
	// ���ڲ���ֱ�ߵ�ֱ�߲ü��㷨(���Ѷ�-Barsky)
	bool line_clip_2d_lb(int &x0, int &y0, int &x1, int &y1, const xrecti_t &rect);

	////////////////////////////////////////////////////////////////////////////////////////////////
	// ������������
	////////////////////////////////////////////////////////////////////////////////////////////////

	// ����λͼ,0�ñ���ɫ����,1��ǰ��ɫ����
	void _bitmap(unsigned dstx, unsigned dsty, uint8_t *pbits, unsigned pitch, unsigned srcx, unsigned srcy, unsigned srcw, unsigned srch);
	// ����λͼ,0������,1��ǰ��ɫ����
	void _bitmap_transparent(unsigned dstx, unsigned dsty, uint8_t *pbits, unsigned pitch, unsigned srcx, unsigned srcy, unsigned srcw, unsigned srch);

	// ���������ʽ����
	void _bitblt(unsigned dstx, unsigned dsty, uint8_t *pbits, XG_PIXEL_FORMAT fmt, unsigned pitch, unsigned srcx, unsigned srcy, unsigned srcw, unsigned srch);
	// ��������,����ɫ�����
	void _bitblt_colorkey(unsigned dstx, unsigned dsty, uint8_t *pbits, XG_PIXEL_FORMAT fmt, unsigned pitch, unsigned srcx, unsigned srcy, unsigned srcw, unsigned srch);
	// ��������,������,ʹ����ʽ�˲�
	void _bitblt_stretch_neareast(unsigned dstx, unsigned dsty, unsigned dstw, unsigned dsth, uint8_t *pbits, XG_PIXEL_FORMAT fmt, unsigned pitch, unsigned srcx, unsigned srcy, unsigned srcw, unsigned srch, float u0, float v0, float u1, float v1);
	// ��������,������,ʹ��˫���Բ�ֵ�˲�(NATIVE_PIXEL_FORMAT����ΪRGBA8888)
	void _bitblt_stretch_bilinear(unsigned dstx, unsigned dsty, unsigned dstw, unsigned dsth, uint8_t *pbits, XG_PIXEL_FORMAT fmt, unsigned pitch, unsigned srcx, unsigned srcy, unsigned srcw, unsigned srch, float u0, float v0, float u1, float v1);

	// �Ը������꾫�Ȼ�������
	void _bitblt_blur(float fdstx, float fdsty, uint8_t *pbits, XG_PIXEL_FORMAT fmt, unsigned pitch, unsigned srcx, unsigned srcy, unsigned srcw, unsigned srch);

	// ��ֲ��fblend: ����͸�����(NATIVE_PIXEL_FORMAT����ΪRGBA8888)
	void _fast_blend(unsigned dstx, unsigned dsty, uint8_t *pbits, unsigned pitch, unsigned srcx, unsigned srcy, unsigned srcw, unsigned srch, uint8_t fact);

#ifdef _DEBUG
	////////////////////////////////////////////////////////////////////////////////////////////////
	// debug �ӿ�
	////////////////////////////////////////////////////////////////////////////////////////////////
	bool poly_fill_start();
	bool poly_fill_step();
	void poly_fill_clear();
	void poly_dump();
#endif // _DEBUG

private:
	// ������
	xrender_buffer m_colorBuffer;
	XG_PIXEL_FORMAT m_pixelfmt;
	// ����ģʽ
	xplotter m_plotter;
	XG_PLOT_MODE m_plotmode;
	// ��ǰ״̬
	pixel_t m_color;
	pixel_t m_bkcolor;
	pixel_t m_colorkey;
	// ��դ��״̬
	uint32_t m_state;
	// �ü�����
	int m_xmin, m_ymin,
		m_xmax, m_ymax;
	// ��ɫ��
	pixel_t *m_palette;
	unsigned m_colorNum;
	// ������ʽ
	static xpattern s_PreDefinedPattern[NUM_PATTERN];
	xpattern m_pattern;
	unsigned m_patternPitch;
	uint8_t *m_patternBuffer;
	// �����
	void *m_pfiller;
	// �������ظ�ʽ������
	static void general_format_plotter(void *pctx, void *pdst, void *psrc);

};

}; // end of namespace wyc

#endif // end of WYC_HEADER_XRASTER

