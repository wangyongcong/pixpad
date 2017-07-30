#include "clipping.h"
#include "ImathVecExt.h"
#include "floatmath.h"
#include <cassert>
#include <algorithm>

namespace wyc
{
	// Liang-Barsky line clipping algorithm
	bool clip_line(Imath::V2f &v0, Imath::V2f &v1, const Imath::Box2f &clip_window)
	{
		float t1 = 0, t2 = 1.0f;
		float dx = v1.x - v0.x, dy = v1.y - v0.y;
		float p[4] = { -dx, dx, -dy, dy };
		float q[4] = {
			v0.x - clip_window.min.x,
			clip_window.max.x - v0.x,
			v0.y - clip_window.min.y,
			clip_window.max.y - v0.y
		};
		for (int i = 0; i < 4; ++i)
		{
			if (p[i] == 0) {
				if (q[i] < 0)
					return false;
			}
			else if (p[i] < 0)
			{
				t1 = std::max<float>(t1, q[i] / p[i]);
			}
			else
			{
				t2 = std::min<float>(t2, q[i] / p[i]);
			}
		}
		if (t1 > t2)
			return false;
		if (t1 > 0)
		{
			v0.x += dx * t1;
			v0.y += dy * t1;
		}
		if (t2 < 1)
		{
			t2 = 1 - t2;
			v1.x -= dx * t2;
			v1.y -= dy * t2;
		}
		return true;
	}

	void clip_polygon_by_plane(const Imath::V4f &plane, const std::vector<Imath::V3f> &vertices, std::vector<Imath::V3f> &out)
	{
		Imath::V3f prev = vertices.back();
		float pdot = prev ^ plane;
		out.reserve(vertices.size() + 1);
		for (auto &vert : vertices)
		{
			float dot = vert ^ plane;
			if (dot * pdot < 0)
				out.push_back(intersect(prev, pdot, vert, dot));
			if (dot >= 0)
				out.push_back(vert);
			prev = vert;
			pdot = dot;
		}
	}

	void clip_polygon(const std::vector<Imath::V4f> &planes, std::vector<Imath::V3f> &vertices)
	{
		for (auto plane : planes)
		{
			std::vector<Imath::V3f> tmp;
			clip_polygon_by_plane(plane, vertices, tmp);
			vertices = std::move(tmp);
			if (vertices.empty())
				return;
		}
	}

	void clip_polygon_homo(std::vector<Imath::V4f> &vertices)
	{
		std::vector<Imath::V4f> out;
		// clipped by 7 planes may result 7 more vertices at most
		out.reserve(vertices.size() + 7);
		// clipped by W=0
		constexpr float w_epsilon = 0.0001f;
		float pdot, dot;
		Imath::V4f prev = vertices.back();
		pdot = prev.w - w_epsilon;
		for (auto &vert : vertices)
		{
			dot = vert.w - w_epsilon;
			if (pdot * dot < 0)
				out.push_back(intersect(prev, pdot, vert, dot));
			if (dot >= 0)
				out.push_back(vert);
			prev = vert;
			pdot = dot;
		}
		vertices.swap(out);
		if (vertices.empty())
			return;
		out.clear();
		// clipped by positive plane: W=X, W=Y, W=Z
		for (int i = 0; i < 3; ++i)
		{
			prev = vertices.back();
			pdot = prev.w - prev[i], dot;
			for (auto &vert : vertices)
			{
				dot = vert.w - vert[i];
				if (pdot * dot < 0)
					out.push_back(intersect(prev, pdot, vert, dot));
				if (dot >= 0)
					out.push_back(vert);
				prev = vert;
				pdot = dot;
			}
			vertices.swap(out);
			if (vertices.empty())
				return;
			out.clear();
		}
		// clipped by negative plane: W=-X, W=-Y, W=-Z
		for (int i = 0; i < 3; ++i)
		{
			prev = vertices.back();
			pdot = prev.w + prev[i], dot;
			for (auto &vert : vertices)
			{
				dot = vert.w + vert[i];
				if (pdot * dot < 0)
					out.push_back(intersect(prev, pdot, vert, dot));
				if (dot >= 0)
					out.push_back(vert);
				prev = vert;
				pdot = dot;
			}
			vertices.swap(out);
			if (vertices.empty())
				return;
			out.clear();
		}
	}

	void intersect(float *v1, float d1, float *v2, float d2, size_t stride, float *out)
	{
		float t = d1 / (d1 - d2);
		/*
		// adjust t to make sure the intersection point is inside clipping plane
		if (d1 < 0) // go inside
			t = fast_ceil(t * 10000) * 0.0001f;
		else // // go outside
			t = fast_floor(t * 10000) * 0.0001f;
		*/
		// interpolate: v1 + (v2 - v1) * t;
		for (float *end = out + stride; out != end; ++out, ++v1, ++v2)
		{
			*out = (*v1) + (*v2 - *v1) * t;
		}
	}

	float* clip_polygon_stream(float *vertex_in, float *vertex_out, size_t &vertex_count, size_t stride)
	{
		constexpr float w_epsilon = 0.0001f;
		float *prev_vert = vertex_in + stride * (vertex_count - 1);
		float *cur_vert = vertex_in;
		float *out_vert = vertex_out;
		float *end = cur_vert + (vertex_count * stride);
		// pos = {x, y, z, w}
		float *pos;
		float pdot, dot;
		// clipped by W=0
		pdot = prev_vert[3] - w_epsilon;
		vertex_count = 0;
		for (; cur_vert < end; cur_vert += stride)
		{
			pos = cur_vert;
			dot = pos[3] - w_epsilon;
			if (pdot * dot < 0) {
				intersect(prev_vert, pdot, cur_vert, dot, stride, out_vert);
				out_vert += stride;
				vertex_count += 1;
			}
			if (dot >= 0) {
				memcpy(out_vert, cur_vert, sizeof(float) * stride);
				out_vert += stride;
				vertex_count += 1;
			}
			prev_vert = cur_vert;
			pdot = dot;
		}
		if (!vertex_count)
			return nullptr;
		// clipped by positive plane: W=X, W=Y, W=Z
		for (int i = 0; i < 3; ++i)
		{
			std::swap(vertex_in, vertex_out);
			prev_vert = out_vert - stride;
			end = out_vert;
			cur_vert = vertex_in;
			out_vert = vertex_out;
			vertex_count = 0;
			pdot = prev_vert[3] - prev_vert[i];
			for (; cur_vert < end; cur_vert += stride)
			{
				pos = cur_vert;
				dot = pos[3] - pos[i];
				if (pdot * dot < 0) {
					//assert(out_vert < vertex_out + cache_size && "vertex cache overflow");
					intersect(prev_vert, pdot, cur_vert, dot, stride, out_vert);
					out_vert += stride;
					vertex_count += 1;
				}
				if (dot >= 0) {
					//assert(out_vert < vertex_out + cache_size && "vertex cache overflow");
					memcpy(out_vert, cur_vert, sizeof(float) * stride);
					out_vert += stride;
					vertex_count += 1;
				}
				prev_vert = cur_vert;
				pdot = dot;
			}
			if (!vertex_count)
				return nullptr;
		}
		// clipped by negative plane: W=-X, W=-Y, W=-Z
		for (int i = 0; i < 3; ++i)
		{
			std::swap(vertex_in, vertex_out);
			prev_vert = out_vert - stride;
			end = out_vert;
			cur_vert = vertex_in;
			out_vert = vertex_out;
			vertex_count = 0;
			pdot = prev_vert[3] + prev_vert[i];
			for (; cur_vert < end; cur_vert += stride)
			{
				pos = cur_vert;
				dot = pos[3] + pos[i];
				if (pdot * dot < 0)
				{
					//assert(out_vert < vertex_out + cache_size && "vertex cache overflow");
					intersect(prev_vert, pdot, cur_vert, dot, stride, out_vert);
					out_vert += stride;
					vertex_count += 1;
				}
				if (dot >= 0)
				{
					//assert(out_vert < vertex_out + cache_size && "vertex cache overflow");
					memcpy(out_vert, cur_vert, sizeof(float) * stride);
					out_vert += stride;
					vertex_count += 1;
				}
				prev_vert = cur_vert;
				pdot = dot;
			}
			if (!vertex_count)
				return nullptr;
		}
		return vertex_out;
	}

	void clip_polygon_stream(std::vector<float> &vertices, std::vector<unsigned> &indices_in, std::vector<unsigned> &indices_out, unsigned stride)
	{
		constexpr float w_epsilon = 0.0001f;
		// pos = {x, y, z, w}
		float pdot, dot;
		unsigned prev_i;
		if (!indices_out.empty())
			indices_out.clear();

		// clipped by W=0
		prev_i = indices_in.back();
		pdot = vertices[prev_i + 3] - w_epsilon;
		for (auto i : indices_in)
		{
			dot = vertices[i + 3] - w_epsilon;
			if (pdot * dot < 0) {
				unsigned j = vertices.size();
				vertices.resize(j + stride);
				intersect(&vertices[prev_i], pdot, &vertices[i], dot, stride, &vertices[j]);
				indices_out.push_back(j);
			}
			if (dot >= 0) {
				indices_out.push_back(i);
			}
			prev_i = i;
			pdot = dot;
		}
		if (indices_out.empty())
			return;

		// clipped by positive plane: W=X, W=Y, W=Z
		for (int k = 0; k < 3; ++k)
		{
			indices_in.swap(indices_out);
			indices_out.clear();
			prev_i = indices_in.back();
			pdot = vertices[prev_i + 3] - vertices[prev_i + k];
			for (auto i : indices_in) {
				dot = vertices[i + 3] - vertices[i + k];
				if (pdot * dot < 0) {
					unsigned j = vertices.size();
					vertices.resize(j + stride);
					intersect(&vertices[prev_i], pdot, &vertices[i], dot, stride, &vertices[j]);
					indices_out.push_back(j);
				}
				if (dot >= 0) {
					indices_out.push_back(i);
				}
				prev_i = i;
				pdot = dot;
			}
			if (indices_out.empty())
				return;
		}

		// clipped by negative plane: W=-X, W=-Y, W=-Z
		for (int k = 0; k < 3; ++k)
		{
			indices_in.swap(indices_out);
			indices_out.clear();
			prev_i = indices_in.back();
			pdot = vertices[prev_i + 3] + vertices[prev_i + k];
			for (auto i : indices_in) {
				dot = vertices[i + 3] + vertices[i + k];
				if (pdot * dot < 0) {
					unsigned j = vertices.size();
					vertices.resize(j + stride);
					intersect(&vertices[prev_i], pdot, &vertices[i], dot, stride, &vertices[j]);
					indices_out.push_back(j);
				}
				if (dot >= 0) {
					indices_out.push_back(i);
				}
				prev_i = i;
				pdot = dot;
			}
			if (indices_out.empty())
				return;
		}
	}

} // namespace wyc