#include "transform.h"

namespace wyc
{

	xtransform::xtransform()
	{
		m_position.zero();
		m_forward = { 0, 0, 1 };
		m_up = { 0, 1, 0 };
		m_right = { 1, 0, 0 };
		m_scale = { 1, 1, 1 };
		m_local2world.identity();
		m_world2local.identity();
		m_flag = 0;

	}

	bool xtransform::update(xtransform *parent_trans, bool rebuild)
	{
		if (rebuild || m_flag&LOCAL_2_WORLD) {
			rebuild_local2world();
			m_flag |= WORLD_2_LOCAL;
			if (parent_trans)
				m_local2world = parent_trans->local2world()*m_local2world;
			rebuild_world2local();
			if (parent_trans)
				m_world2local *= parent_trans->world2local();
			return true;
		}
		return false;
	}

	void xtransform::rebuild_local2world()
	{
		m_local2world.set_col(0, m_right*m_scale.x);
		m_local2world.set_col(1, m_up*m_scale.y);
		m_local2world.set_col(2, m_forward*m_scale.z);
		m_local2world.set_col(3, m_position);
		xvec4f_t vec;
		vec = { 0, 0, 0, 1 };
		m_local2world.set_row(3, vec);
		m_flag &= ~LOCAL_2_WORLD;
	}

	void xtransform::rebuild_world2local()
	{
		m_world2local.set_row(0, m_right*(1.0f / m_scale.x));
		m_world2local.set_row(1, m_up*(1.0f / m_scale.y));
		m_world2local.set_row(2, m_forward*(1.0f / m_scale.z));
		xvec4f_t vec;
		vec = { 0, 0, 0, 1 };
		m_world2local.set_row(3, vec);
		xvec3f_t tpos = m_world2local*-m_position;
		m_world2local.set_col(3, tpos);
		m_flag &= ~WORLD_2_LOCAL;
	}

	void xtransform::set_forward(const xvec3f_t &forward, const xvec3f_t &up)
	{
		m_forward = forward;
		m_up = up;
		m_right = m_up.cross(m_forward);
		if (fequal(m_right.length2(), 0.0f)) {
			unsigned mind = 0;
			if (fabs(m_forward.x) > fabs(m_forward.y))
				mind = 1;
			if (fabs(m_forward.y) > fabs(m_forward.z))
				mind = 2;
			m_right.zero();
			m_right[mind] = 1.0f;
			m_up = m_forward.cross(m_right);
		}
		else {
			m_up = m_forward.cross(m_right);
			m_right.normalize();
		}
		m_forward.normalize();
		m_up.normalize();
		m_flag |= ROTATE;
	}

	void xtransform::rotate(const xvec3f_t &axis, float angle)
	{
		if (wyc::fabs(angle) > 360)
			angle = wyc::mod(angle, 360);
		xmat3f_t mat;
		matrix_rotate3d(mat, axis, DEG_TO_RAD(angle));
		m_forward = m_forward*mat;
		m_up = m_up*mat;
		m_right = m_right*mat;
		m_flag |= ROTATE;
	}

	void xtransform::rotate_forward(float angle)
	{
		if (wyc::fabs(angle) > 360)
			angle = wyc::mod(angle, 360);
		xmat3f_t mat;
		matrix_rotate3d(mat, m_forward, DEG_TO_RAD(angle));
		m_up = m_up*mat;
		m_right = m_right*mat;
		m_flag |= ROTATE;
	}

	void xtransform::rotate_up(float angle)
	{
		if (wyc::fabs(angle) > 360)
			angle = wyc::mod(angle, 360);
		xmat3f_t mat;
		matrix_rotate3d(mat, m_up, DEG_TO_RAD(angle));
		m_forward = m_forward*mat;
		m_right = m_right*mat;
		m_flag |= ROTATE;
	}

	void xtransform::rotate_right(float angle)
	{
		if (wyc::fabs(angle) > 360)
			angle = wyc::mod(angle, 360);
		xmat3f_t mat;
		matrix_rotate3d(mat, m_right, DEG_TO_RAD(angle));
		m_forward = m_forward*mat;
		m_up = m_up*mat;
		m_flag |= ROTATE;
	}

}; // namespace wyc

