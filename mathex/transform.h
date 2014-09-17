#ifndef __HEADER_WYC_TRANSFORM
#define __HEADER_WYC_TRANSFORM

#include "vecmath.h"

namespace wyc
{

class xtransform
{
	xvec3f_t m_position;
	xvec3f_t m_forward, m_up, m_right;
	xvec3f_t m_scale;
	xmat4f_t m_local2world; // �о���
	xmat4f_t m_world2local; // �о���
	enum {
		TRANSLATE=0x11,
		ROTATE=0x22,
		LOCAL_2_WORLD=0x0F,
		WORLD_2_LOCAL=0xF0,
	};
	unsigned m_flag;
public:
	xtransform();
	bool update(xtransform *parent_trans=0, bool rebuild=false);
	// ����
	inline void set_orientation(const xvec3f_t &forward, const xvec3f_t &up, const xvec3f_t &right) {
		m_forward=forward;
		m_up=up;
		m_right=right;
		m_flag|=ROTATE;
	}
	void set_forward(const xvec3f_t &forward, const xvec3f_t &up);
	inline const xvec3f_t& forward() const {
		return m_forward;
	}
	inline const xvec3f_t& up() const {
		return m_up;
	}
	inline const xvec3f_t& right() const {
		return m_right;
	}
	// ƽ��
	inline void set_position(const xvec3f_t &pos) {
		m_position=pos;
		m_flag|=TRANSLATE;
	}
	inline void set_position(float x, float y, float z) {
		m_position.set(x,y,z);
		m_flag|=TRANSLATE;
	}
	inline const xvec3f_t& position() const {
		return m_position;
	}
	inline void translate(float forward, float up, float right) {
		m_position+=m_forward*forward;
		m_position+=m_up*up;
		m_position+=m_right*right;
		m_flag|=TRANSLATE;
	}
	inline void translate_forward(float d) {
		m_position+=m_forward*d;
		m_flag|=TRANSLATE;
	}
	inline void translate_up(float d) {
		m_position+=m_up*d;
		m_flag|=TRANSLATE;
	}
	inline void translate_right(float d) {
		m_position+=m_right*d;
		m_flag|=TRANSLATE;
	}
	// ��ת
	void rotate(const xvec3f_t &axis, float angle);
	void rotate_forward(float angle);
	void rotate_up(float angle);
	void rotate_right(float angle);
	// ����
	inline void scale(float x, float y, float z) {
		m_scale.set(x,y,z);
		m_flag|=ROTATE;
	}
	inline void scale(float s) {
		m_scale.set(s,s,s);
		m_flag|=ROTATE;
	}
	inline void scale(const xvec3f_t &scale) {
		m_scale=scale;
		m_flag|=ROTATE;
	}
	inline const xvec3f_t& scaling () const {
		return m_scale;
	}
	// �任(�о���)
	inline const xmat4f_t& local2world() const {
		return m_local2world;
	}
	inline const xmat4f_t& world2local() const {
		return m_world2local;
	}
	// ״̬��ѯ
	inline bool need_update() const {
		return 0!=(m_flag&LOCAL_2_WORLD);
	}
	inline bool is_moved() const {
		return 0!=(m_flag&TRANSLATE);
	}
	inline bool is_rotated() const {
		return 0!=(m_flag&ROTATE);
	}
private:
	void rebuild_local2world();
	void rebuild_world2local();
};

/// ��������ͶӰ����(�о���)
void set_orthograph(xmat4f_t &mat, float xmin, float ymin, float zmin, float xmax, float ymax, float zmax);

/**	@brief ����͸�Ӿ���(����)
	@param yfov ��ֱ��׶��
	@param aspect ��߱�
	@param fnear ���ü�����ԭ��ľ��룬�������0
	@param ffar Զ�ü�����ԭ��ľ��룬�������fnear
*/
void set_perspective(xmat4f_t &proj, float fov, float aspect, float fnear, float ffar);

/// ����UIͶӰ����(�о���)
void set_ui_projection(xmat4f_t &proj, float screen_width, float screen_height, float z_range);

}; // namespace wyc

#endif // __HEADER_WYC_TRANSFORM

