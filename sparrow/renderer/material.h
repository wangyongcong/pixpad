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

#define UNIFORM_MAP protected: virtual const UniformMap& get_uniform_define() const override {\
	using cls_type = std::remove_const_t<std::remove_reference_t<decltype(*this)>>;\
	static std::unordered_map<std::string, CUniform> ls_uniform_map = {
#define UNIFORM_MAP_END }; return ls_uniform_map; }

#define UNIFORM_SLOT(type, name) { #name, { typeid(type),\
	[](const CMaterial* self, void* val) {\
		*((type*)(val)) = ((const cls_type*)(self))->##name;\
	},\
	[](CMaterial *self, const void *val) {\
		((cls_type*)(self))->##name = *((const type*)(val));\
	}\
} },

#define INPUT_ATTRIBUTE_LIST protected: virtual const AttribSlot* get_attrib_input() const override { \
	static const AttribSlot ls_input_attribs[] = {
#define INPUT_ATTRIBUTE_LIST_END {EAttribUsage(0), 0} }; return  ls_input_attribs; }

#define OUTPUT_ATTRIBUTE_LIST protected: virtual const AttribSlot* get_attrib_ouput() const override { \
	static const AttribSlot ls_output_attribs[] = {
#define OUTPUT_ATTRIBUTE_LIST_END {EAttribUsage(0), 0} }; return ls_output_attribs; }

#define ATTRIBUTE_SLOT(usage, component) { EAttribUsage(usage), uint8_t(component) },

	class CMaterial;

	struct AttribSlot
	{
		EAttribUsage usage;
		unsigned char component;
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
		typedef std::unordered_map<std::string, CUniform> UniformMap;
		CMaterial();
		CMaterial(const char *name);
		virtual ~CMaterial();
		virtual void vertex_shader(const void *vertex_in, void *vertex_out) const = 0;
		virtual bool fragment_shader(const void *vertex_out, Color4f &frag_color) const = 0;
		template<typename T>
		bool set_uniform(const std::string &name, const T &val);
		template<typename T>
		bool get_uniform(const std::string &name, T &val) const;
		const CUniform* find_uniform(const std::string &name) const;
		// get attribute define
		virtual const AttribDefine& get_attrib_define() const;
		// get uniform define
		virtual const UniformMap& get_uniform_define() const;
	protected:
		// define attribute layout
		typedef std::pair<const AttribSlot*, size_t> AttribSlotList;
		virtual const AttribSlot* get_attrib_input() const { return nullptr; }
		virtual const AttribSlot* get_attrib_ouput() const { return nullptr; }
		AttribDefine create_attrib_define() const;
		// members
		std::string m_name;
	};

	typedef std::shared_ptr<CMaterial> material_ptr;

	template<typename T>
	bool CMaterial::set_uniform(const std::string &name, const T &val)
	{
		const CUniform *u = find_uniform(name);
		if (!u) {
			log_warn("Uniform not found: %s", name.c_str());
			return false;
		}
		if (u->tid != typeid(T)) {
			log_warn("Uniform type error: %s (%s)", name.c_str(), u->tid.name());
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
			log_warn("Uniform not found: %s", name.c_str());
			return false;
		}
		if (u->tid != typeid(T)) {
			log_warn("Uniform type error: %s (%s)", name.c_str(), u->tid.name());
			return false;
		}
		u->get(this, &val);
		using t1 = std::remove_const<std::remove_reference_t<decltype(*this)>>::type;
		return true;
	}

	struct MtlLight
	{
		Imath::V3f color;
		Imath::V3f position;
		float intensity;
	};

	struct MtlUniformData
	{
		std::vector<MtlLight> lights;
	};

} // namespace wyc