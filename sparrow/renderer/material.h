#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <OpenEXR/ImathMatrix.h>
#include "mathfwd.h"
#include "vertex_buffer.h"
#include "log.h"

namespace wyc
{

#define DECLARE_UNIFORM_MAP(cls_type) using cls = cls_type;\
	static std::unordered_map<std::string, CUniform> ls_uniform_map = 

#define UNIFORM_MAP ls_uniform_map

#define MAKE_UNIFORM(type, name) { #name, { typeid(type),\
	[](const CMaterial* self, void* val) {\
		*((type*)(val)) = ((cls*)(self))->##name;\
	},\
	[](CMaterial *self, const void *val) {\
		((cls*)(self))->##name = *((const type*)(val));\
	}\
} }

	class CMaterial;

	struct AttribSlot
	{
		EAttribUsage usage;
		unsigned char component;
	};

	struct AttribSlotList
	{
		const AttribSlot *data;
		unsigned size;
		inline const AttribSlot& operator[] (size_t idx) const {
			return data[idx];
		}
	};

	struct AttribDefine
	{
		const AttribSlot *in_attribs;
		unsigned in_count;
		unsigned in_stride;
		const AttribSlot *out_attribs;
		unsigned out_count;
		unsigned out_stride;
	};

	class CUniform
	{
	public:
		typedef void(*uniform_getter) (const CMaterial*, void*);
		typedef void(*uniform_setter) (CMaterial*, const void*);
		const std::type_info &tid;
		uniform_getter get;
		uniform_setter set;
		CUniform()
			: tid(typeid(nullptr))
			, get(nullptr)
			, set(nullptr)
		{}

		CUniform(const std::type_info &t, uniform_getter g, uniform_setter s)
			: tid(t)
			, get(g)
			, set(s)
		{}
	};

	class CMaterial
	{
	public:
		CMaterial();
		virtual ~CMaterial();
		virtual const AttribDefine& get_attrib_define() const = 0;
		virtual void vertex_shader(const void *vertex_in, void *vertex_out) const = 0;
		virtual bool fragment_shader(const void *vertex_out, Color4f &frag_color) const = 0;
		// access uniform properties
		virtual const std::unordered_map<std::string, CUniform>& get_uniform_define() const {
			DECLARE_UNIFORM_MAP(CMaterial) {
			};
			return UNIFORM_MAP;
		}
		template<typename T>
		bool set_uniform(const std::string &name, const T &val);
		template<typename T>
		bool get_uniform(const std::string &name, T &val) const;
		const CUniform* find_uniform(const std::string &name) const;
	};

	typedef std::shared_ptr<CMaterial> material_ptr;

	template<typename T>
	bool CMaterial::set_uniform(const std::string &name, const T &val)
	{
		const CUniform *u = find_uniform(name);
		if (!u) {
			warn("Uniform not found: %s", name.c_str());
			return false;
		}
		if (u->tid != typeid(T)) {
			warn("Uniform type error: %s (%s)", name.c_str(), u->tid.name());
			return false;
		}
		u->set(this, &val);
		return true;
	}

	template<typename T>
	bool CMaterial::get_uniform(const std::string &name, T &val) const
	{
		const CUniform *u = find_uniform(name);
		if (!u) {
			warn("Uniform not found: %s", name.c_str());
			return false;
		}
		if (u->tid != typeid(T)) {
			warn("Uniform type error: %s (%s)", name.c_str(), u->tid.name());
			return false;
		}
		u->get(this, &val);
		return true;
	}

} // namespace wyc