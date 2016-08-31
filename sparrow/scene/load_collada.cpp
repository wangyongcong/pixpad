#include "scene.h"
#include <COLLADAFW.h>
#include <COLLADASaxFWLLoader.h>
#include <COLLADAFWIWriter.h>
#include "util.h"
#include "log.h"
#include "static_mesh.h"
#include "mtl_collada.h"
#include "light.h"

#pragma comment(lib, "GeneratedSaxParser.lib")
#pragma comment(lib, "OpenCOLLADABaseUtils.lib")
#pragma comment(lib, "OpenCOLLADAFramework.lib")
#pragma comment(lib, "OpenCOLLADASaxFrameworkLoader.lib")
#pragma comment(lib, "MathMLSolver.lib")
#pragma comment(lib, "pcre.lib")
#pragma comment(lib, "utf.lib")
#pragma comment(lib, "xml.lib")

namespace wyc
{
	inline const char* str(const COLLADAFW::UniqueId &uid)
	{
		static std::string ls_tmp_uid;
		ls_tmp_uid = uid.toAscii();
		return ls_tmp_uid.c_str();
	}

	inline const char* str(const COLLADAFW::String &s)
	{
		return s.c_str();
	}

	inline void to_transform(wyc::Matrix44f &lft, const COLLADABU::Math::Matrix4 &rht)
	{
		float *dst = lft.getValue();
		for (int i = 0; i < 16; ++i)
		{
			dst[i] = (float)rht.getElement(i);
		}
	}

	inline Imath::C4f to_color(const COLLADAFW::Color &c)
	{
		return Imath::C4f(float(c.getRed()), float(c.getGreen()), float(c.getBlue()), float(c.getAlpha()));
	}

class CColladaSceneWriter : public COLLADAFW::IWriter
{
	// member declarations
public:
	CColladaSceneWriter(CScene *scn) 
		: IWriter()
		, m_scene(scn)
		, m_def_mtl("Collada")
	{
		assert(scn && "CColladaSceneWriter: Need a valid scene to write to");
	}
	virtual ~CColladaSceneWriter() {}
	/** Disable default copy ctor. */
	CColladaSceneWriter(const CColladaSceneWriter& pre) = delete;
	/** Disable default assignment operator. */
	const CColladaSceneWriter& operator= (const CColladaSceneWriter& pre) = delete;

	/** This method will be called if an error in the loading process occurred and the loader cannot
	continue to to load. The writer should undo all operations that have been performed.
	@param errorMessage A message containing informations about the error that occurred.
	*/
	virtual void cancel(const COLLADAFW::String& errorMessage) {
		log_error("cancel loading with error:\n%s", errorMessage.c_str());
	}

	/** This is the method called. The writer hast to prepare to receive data.*/
	virtual void start() {
		log_debug("start loading");
	}

	/** This method is called after the last write* method. No other methods will be called after this.*/
	virtual void finish() {
		log_debug("finish loading");
	}

	/** When this method is called, the writer must write the global document asset.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	virtual bool writeGlobalAsset(const COLLADAFW::FileInfo* asset)
	{
		return true;
	}

	/** When this method is called, the writer must write the entire visual scene.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	virtual bool writeScene(const COLLADAFW::Scene* scene)
	{
		return true;
	}

	/** When this method is called, the writer must write the entire visual scene.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	virtual bool writeVisualScene(const COLLADAFW::VisualScene* visualScene)
	{
		log_debug("\tvisual scene: %s", visualScene->getName().c_str());
		auto &node_array = visualScene->getRootNodes();
		for (size_t i = 0; i < node_array.getCount(); ++i)
		{
			writeVisualSceneNode(node_array[i], COLLADABU::Math::Matrix4::IDENTITY);
		}
		return true;
	}

	/** When this method is called, the writer must handle all nodes contained in the
	library nodes.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	virtual bool writeLibraryNodes(const COLLADAFW::LibraryNodes* libraryNodes)
	{
		auto &nodes = libraryNodes->getNodes();
		log_debug("\tlibrary nodes: %d", nodes.getCount());
		return true;
	}

	/** When this method is called, the writer must write the geometry.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	virtual bool writeGeometry(const COLLADAFW::Geometry* geometry)
	{
		if (geometry->getType() == COLLADAFW::Geometry::GEO_TYPE_MESH)
		{
			const COLLADAFW::Mesh *mesh = dynamic_cast<const COLLADAFW::Mesh*>(geometry);
			writeMesh(mesh);			
		}
		else
		{
			log_debug("\tgeometry: %s (%s)", geometry->getName().c_str(), str(geometry->getUniqueId()));
		}
		return true;
	}

	/** When this method is called, the writer must write the material.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	virtual bool writeMaterial(const COLLADAFW::Material* material)
	{
		log_debug("\tmateiral: %s (%s)", str(material->getName()), str(material->getUniqueId()));
		std::string unique_name = material->getUniqueId().toAscii();
		auto spw_mtl = std::make_shared<CMaterialCollada>();
		m_materials[unique_name] = spw_mtl;
		std::string effect_name = material->getInstantiatedEffect().toAscii();
		m_effect_to_mtl[effect_name] = unique_name;
		return true;
	}

	/** When this method is called, the writer must write the effect.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	virtual bool writeEffect(const COLLADAFW::Effect* effect)
	{
		log_debug("\teffect: %s (%s)", str(effect->getName()), str(effect->getUniqueId()));
		std::string unique_name = effect->getUniqueId().toAscii();
		auto it_effect = m_effect_to_mtl.find(unique_name);
		if (it_effect == m_effect_to_mtl.end())
			return true;
		auto it_mtl = m_materials.find(it_effect->second);
		if (it_mtl == m_materials.end())
			return true;
		auto spw_mtl = it_mtl->second;
		auto &common_profile = effect->getCommonEffects();
		const COLLADAFW::EffectCommon *common_effect;
		for (unsigned i = 0, cnt = common_profile.getCount(); i < cnt; ++i)
		{
			common_effect = common_profile[i];
			auto &emission = common_effect->getEmission();
			if (emission.isColor()) {
				spw_mtl->set_uniform("emission", to_color(emission.getColor()));
			}
			auto &diffuse = common_effect->getDiffuse();
			if (diffuse.isColor()) {
				spw_mtl->set_uniform("diffuse", to_color(diffuse.getColor()));
			}
			auto &specular = common_effect->getSpecular();
			if (specular.isColor()) {
				spw_mtl->set_uniform("specular", to_color(specular.getColor()));
			}
			auto &shininess = common_effect->getShininess();
			spw_mtl->set_uniform("shininess", shininess.getFloatValue());
		}
		return true;
	}

	/** When this method is called, the writer must write the camera.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	virtual bool writeCamera(const COLLADAFW::Camera* camera)
	{
		std::string unique_name = camera->getUniqueId().toAscii();
		log_debug("\tcamera: %s (%s)", str(camera->getName()), unique_name.c_str());
		auto scn_camera = m_scene->create_camera(unique_name);
		auto camera_type = camera->getCameraType();
		if (COLLADAFW::Camera::ORTHOGRAPHIC == camera_type)
		{
			scn_camera->set_orthographic(float(camera->getXMag()), float(camera->getYMag()), 
				float(camera->getNearClippingPlane()), float(camera->getFarClippingPlane()));
		}
		else if (COLLADAFW::Camera::PERSPECTIVE == camera_type)
		{
			float yfov = float(camera->getXFov() / camera->getAspectRatio());
			scn_camera->set_perspective(yfov, float(camera->getAspectRatio()),
				float(camera->getNearClippingPlane()), float(camera->getFarClippingPlane()));
		}
		return true;
	}

	/** When this method is called, the writer must write the image.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	virtual bool writeImage(const COLLADAFW::Image* image)
	{
		return true;
	}

	/** When this method is called, the writer must write the light.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	virtual bool writeLight(const COLLADAFW::Light* light)
	{
		log_debug("\tlight: %s (%s)", str(light->getName()), str(light->getUniqueId()));
		std::string unique_name = light->getUniqueId().toAscii();
		auto lit = std::make_shared<CLight>();
		lit->set_name(unique_name);
		m_lights[unique_name] = lit;
		return true;
	}

	/** Writes the animation.
	@return True on succeeded, false otherwise.*/
	virtual bool writeAnimation(const COLLADAFW::Animation* animation)
	{
		return true;
	}

	/** When this method is called, the writer must write the AnimationList.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	virtual bool writeAnimationList(const COLLADAFW::AnimationList* animationList)
	{
		return true;
	}

	/** When this method is called, the writer must write the skin controller data.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	virtual bool writeSkinControllerData(const COLLADAFW::SkinControllerData* skinControllerData)
	{
		return true;
	}

	/** When this method is called, the writer must write the controller.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	virtual bool writeController(const COLLADAFW::Controller* controller)
	{
		return true;
	}

	/** When this method is called, the writer must write the formula.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	virtual bool writeFormulas(const COLLADAFW::Formulas* formulas)
	{
		return true;
	}

	/** When this method is called, the writer must write the formula.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	virtual bool writeFormula(const COLLADAFW::Formula* formulas)
	{
		return true;
	}

	/** When this method is called, the writer must write the kinematics scene.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	virtual bool writeKinematicsScene(const COLLADAFW::KinematicsScene* kinematicsScene)
	{
		return true;
	}

	// private function declarations
private:
	
	void writeMesh(const COLLADAFW::Mesh *collada_mesh)
	{
		std::string unique_name = collada_mesh->getUniqueId().toAscii();
		log_debug("\tmesh: %s (%s)", collada_mesh->getName().c_str(), unique_name.c_str());
		const COLLADAFW::MeshVertexData &pos_data = collada_mesh->getPositions();
		if (pos_data.empty())
		{
			log_warn("Empty mesh");
			return;
		}
		if (COLLADAFW::FloatOrDoubleArray::DATA_TYPE_FLOAT != pos_data.getType())
		{
			log_warn("Invalid data type, we only support single precision floating point value");
			return;
		}
		auto scn_mesh = std::make_shared<CMesh>();
		m_mesh[unique_name] = scn_mesh;
		auto &vb = scn_mesh->vertex_buffer();
		// Position: {x, y, z}
		size_t vertex_count = pos_data.getValuesCount() / 3;
		vb.set_attribute(EAttribUsage::ATTR_POSITION, 3);
		// Color: {x, y, z}
		auto &color_data = collada_mesh->getColors();
		if (!color_data.empty()) {
			assert(color_data.getValuesCount() == pos_data.getValuesCount());
			vb.set_attribute(EAttribUsage::ATTR_COLOR, 3);
		}
		// Normal: {x, y, z}
		auto &normal_data = collada_mesh->getNormals();
		if (!normal_data.empty()) {
			// Notice: COLLADA could have face normal, but we only support vertex normal
			if (normal_data.getValuesCount() == pos_data.getValuesCount()) {
				vb.set_attribute(EAttribUsage::ATTR_NORMAL, 3);
			}
			else {
				log_warn("Normal count dosen't match vertex count. It could be face normal which we don't support.");
			}
		}
		// todo: apply other vertex attribute
		// reserve memory for vertex data
		vb.resize(vertex_count);
		auto va = vb.get_attribute(EAttribUsage::ATTR_POSITION);
		Imath::V3f *vec = (Imath::V3f*)pos_data.getFloatValues()->getData();
		for (auto &it : va) {
			it = *vec++;
		}
		if (!color_data.empty())
		{
			vec = (Imath::V3f*)color_data.getFloatValues()->getData();
			va = vb.get_attribute(EAttribUsage::ATTR_COLOR);
			for (auto &it : va) {
				it = *vec++;
			}
		}
		if (!normal_data.empty())
		{
			vec = (Imath::V3f*)normal_data.getFloatValues()->getData();
			va = vb.get_attribute(EAttribUsage::ATTR_NORMAL);
			for (auto &it : va) {
				it = *vec++;
			}
		}
		// setup indices
		auto &prim_array = collada_mesh->getMeshPrimitives();
		for (size_t i = 0; i < prim_array.getCount(); ++i)
		{
			const COLLADAFW::MeshPrimitive * prim = prim_array[i];
			auto prim_type = prim->getPrimitiveType();
			switch (prim_type)
			{
			case COLLADAFW::MeshPrimitive::PrimitiveType::POINTS:
			case COLLADAFW::MeshPrimitive::PrimitiveType::LINES:
			case COLLADAFW::MeshPrimitive::PrimitiveType::LINE_STRIPS:
			case COLLADAFW::MeshPrimitive::PrimitiveType::POLYGONS:
			case COLLADAFW::MeshPrimitive::PrimitiveType::TRIANGLES:
			case COLLADAFW::MeshPrimitive::PrimitiveType::TRIANGLE_FANS:
			case COLLADAFW::MeshPrimitive::PrimitiveType::TRIANGLE_STRIPS:
				log_error("%s : not support primitive type: %d", __FUNCTION__, prim_type);
				break;
			case COLLADAFW::MeshPrimitive::PrimitiveType::POLYLIST:
				writePolyList(scn_mesh.get(), collada_mesh, prim);
				break;
			default:
				log_error("%s : undefined primitive type: %d", __FUNCTION__, prim_type);
				break;
			}
		}
	}

	void writePolyList(CMesh *scn_mesh, const COLLADAFW::Mesh *collada_mesh, const COLLADAFW::MeshPrimitive *primitive)
	{
		size_t face_cnt = primitive->getFaceCount();
		size_t index_cnt = face_cnt * 3;
		const auto &pos_indices = primitive->getPositionIndices();
		if (pos_indices.getCount() != index_cnt)
		{
			log_warn("Must be triangular mesh");
			return;
		}
		size_t vertex_cnt = scn_mesh->vertex_count();
		auto &ib = scn_mesh->index_buffer();
		const unsigned int *index_data = pos_indices.getData();
		//if (vertex_cnt < std::numeric_limits<uint16_t>::max()) {
		//	ib.resize<uint16_t>(index_cnt);
		//	for (auto &i : ib) {
		//		i = (uint16_t)(*index_data++);
		//	}
		//	assert(index_data == pos_indices.getData() + pos_indices.getCount());
		//}
		if (vertex_cnt < std::numeric_limits<uint32_t>::max()) {
			ib.resize<uint32_t>(index_cnt, index_data);
			for (auto &i : ib) {
				i = (uint32_t)(*index_data++);
			}
			assert(index_data == pos_indices.getData() + pos_indices.getCount());
		}
		else {
			assert(0 && "Vertex count overflow");
			return;
		}
	}

	void writeVisualSceneNode(const COLLADAFW::Node *node, const COLLADABU::Math::Matrix4& parent_transform)
	{
		log_debug("\t\tobj: %s (%s)", node->getName().c_str(), str(node->getUniqueId()));
		COLLADABU::Math::Matrix4 colla_transofrm = node->getTransformationMatrix() * parent_transform;
		Matrix44f transform;
		to_transform(transform, colla_transofrm);
		
		std::string unique_name;
		auto &camera_list = node->getInstanceCameras();
		if (!camera_list.empty())
		{
			for (size_t j = 0; j < camera_list.getCount(); ++j) {
				unique_name = camera_list[j]->getInstanciatedObjectId().toAscii();
				auto scn_camera = m_scene->get_camera(unique_name);
				if (!scn_camera) {
					log_debug("\t\t\tinstance camera %s not found", unique_name.c_str());
					continue;
				}
				scn_camera->set_transform(transform);
				m_scene->set_active_camera(unique_name);
				log_debug("\t\t\tinstance camera %s", unique_name.c_str());
			}
		}

		auto &geometry_list = node->getInstanceGeometries();
		if (!geometry_list.empty())
		{
			for (size_t j = 0; j < geometry_list.getCount(); ++j) {
				auto geometry = geometry_list[j];
				unique_name = geometry->getInstanciatedObjectId().toAscii();
				auto mesh = m_mesh[unique_name];
				auto obj = std::make_shared<CStaticMesh>();
				obj->set_name(node->getName().c_str());
				obj->set_mesh(mesh);
				auto &material_array = geometry->getMaterialBindings();
				for (size_t k = 0; k < material_array.getCount(); ++k)
				{
					auto &binding = material_array[k];
					unique_name = binding.getReferencedMaterial().toAscii();
					auto it_mtl = m_materials.find(unique_name);
					if(it_mtl != m_materials.end())
						obj->set_material(it_mtl->second);
				}
				m_scene->add_object(obj);
				log_debug("\t\t\tinstance geometry %s (PID=%d)", unique_name.c_str(), obj->get_pid());
			}
		}

		auto &child_nodes = node->getChildNodes();
		for (size_t i = 0; i < child_nodes.getCount(); ++i)
		{
			writeVisualSceneNode(child_nodes[i], colla_transofrm);
		}
	}

	// private data declarations
private:
	CScene *m_scene;
	std::string m_def_mtl;
	std::unordered_map<std::string, std::shared_ptr<CMesh>> m_mesh;
	std::unordered_map<std::string, std::shared_ptr<CMaterial>> m_materials;
	std::unordered_map<std::string, std::string> m_effect_to_mtl;
	std::unordered_map<std::string, std::shared_ptr<CLight>> m_lights;
};

bool CScene::load_collada(const std::wstring & w_file_path)
{
	std::string path;
	if (!wstr2str(path, w_file_path))
	{
		log_error("Invalid file path");
		return false;
	}
	COLLADASaxFWL::Loader collada;
	CColladaSceneWriter writer(this);
	COLLADAFW::Root root(&collada, &writer);
	if (!root.loadDocument(path))
	{
		log_error("Fail to load file: %s", path);
		return false;
	}
	return true;
}

} // namespace wyc
