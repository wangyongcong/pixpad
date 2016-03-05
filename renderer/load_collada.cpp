#include "scene.h"
#include <COLLADAFW.h>
#include <COLLADASaxFWLLoader.h>
#include <COLLADAFWIWriter.h>
#include <common/util.h>
#include <common/log.h>

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

class CColladaSceneWriter : public COLLADAFW::IWriter
{
	// member declarations
public:
	CColladaSceneWriter(CScene *scn) 
		: IWriter()
		, m_scene(scn)
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
		error("cancel loading with error:\n%s", errorMessage.c_str());
	}

	/** This is the method called. The writer hast to prepare to receive data.*/
	virtual void start() {
		debug("start loading");
	}

	/** This method is called after the last write* method. No other methods will be called after this.*/
	virtual void finish() {
		debug("finish loading");
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
		debug("\tvisual scene: %s", visualScene->getName().c_str());
		auto &node_array = visualScene->getRootNodes();
		for (size_t i = 0; i < node_array.getCount(); ++i)
		{
			auto node = node_array[i];
			debug("\t\tobj: %s (%s)", node->getName().c_str(), str(node->getUniqueId()));
			auto &inst_cam = node->getInstanceCameras();
			if (!inst_cam.empty())
			{
				for (size_t j = 0; j < inst_cam.getCount(); ++j)
					debug("\t\t\tinstance camera %s", str(inst_cam[j]->getInstanciatedObjectId()));
			}
			auto &inst_geo = node->getInstanceGeometries();
			if (!inst_geo.empty())
			{
				for (size_t j = 0; j < inst_geo.getCount(); ++j)
					debug("\t\t\tinstance geometry %s", str(inst_geo[j]->getInstanciatedObjectId()));
			}
		}
		return true;
	}

	/** When this method is called, the writer must handle all nodes contained in the
	library nodes.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	virtual bool writeLibraryNodes(const COLLADAFW::LibraryNodes* libraryNodes)
	{
		auto &nodes = libraryNodes->getNodes();
		debug("\tlibrary nodes: %d", nodes.getCount());
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
			debug("\tgeometry: %s (%s)", geometry->getName().c_str(), str(geometry->getUniqueId()));
		}
		return true;
	}

	/** When this method is called, the writer must write the material.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	virtual bool writeMaterial(const COLLADAFW::Material* material)
	{
		debug("\tmateiral: %s (%s)", str(material->getName()), str(material->getUniqueId()));
		return true;
	}

	/** When this method is called, the writer must write the effect.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	virtual bool writeEffect(const COLLADAFW::Effect* effect)
	{
		return true;
	}

	/** When this method is called, the writer must write the camera.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	virtual bool writeCamera(const COLLADAFW::Camera* camera)
	{
		debug("\tcamera: %s (%s)", str(camera->getName()), str(camera->getUniqueId()));
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
		debug("\tlight: %s (%s)", str(light->getName()), str(light->getUniqueId()));
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
	
	void writeMesh(const COLLADAFW::Mesh *colla_mesh)
	{
		std::string unique_name = colla_mesh->getUniqueId().toAscii();
		debug("\tmesh: %s (%s)", colla_mesh->getName().c_str(), unique_name.c_str());
		CMesh *scn_mesh = m_scene->create_mesh(unique_name);
		auto &prim_array = colla_mesh->getMeshPrimitives();
		size_t face_count = 0;
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
				error("%s : not support primitive type: %d", __FUNCTION__, prim_type);
				break;
			case COLLADAFW::MeshPrimitive::PrimitiveType::POLYLIST:
				writePolyList(scn_mesh, colla_mesh, prim);
				break;
			default:
				error("%s : undefined primitive type: %d", __FUNCTION__, prim_type);
				break;
			}
			face_count += prim->getFaceCount();
		}
	}

	void writePolyList(CMesh *scn_mesh, const COLLADAFW::Mesh *colla_mesh, const COLLADAFW::MeshPrimitive *primitive)
	{
		const COLLADAFW::MeshVertexData &pos_data = colla_mesh->getPositions();
		if (pos_data.empty())
		{
			warn("Empty mesh");
			return;
		}
		const COLLADAFW::FloatArray *pos_array = pos_data.getFloatValues();
		const float* pos_iter = pos_array->getData();

		size_t face_cnt = primitive->getFaceCount();
		size_t index_cnt = face_cnt * 3;
		const auto &pos_indices = primitive->getPositionIndices();
		if (pos_indices.getCount() != index_cnt)
		{
			warn("Must be triangular mesh");
			return;
		}

		auto &vb = scn_mesh->vertex_buffer();
		// always {x, y, z}
		vb.set_attribute(EAttribUsage::ATTR_POSITION, 3);
		auto &normal_data = colla_mesh->getNormals();
		if (!normal_data.empty()) {
			// always {x, y, z}
			vb.set_attribute(EAttribUsage::ATTR_NORMAL, 3);
		}
		auto &color_data = colla_mesh->getColors();
		if (!color_data.empty()) {
			// always {x, y, z}
			vb.set_attribute(EAttribUsage::ATTR_COLOR, 3);
		}
		vb.resize(index_cnt);
		
		for (size_t i = 0; i < face_cnt; ++i)
		{
			unsigned pos_i;
			for (int j = 0; j < 3; ++j) {
				pos_i = pos_indices[j];
			}
		}

	}
	
	CScene *m_scene;
};

bool CScene::load_collada(const std::wstring & w_file_path)
{
	std::string path;
	if (!wstr2str(path, w_file_path))
	{
		error("Invalid file path");
		return false;
	}
	COLLADASaxFWL::Loader collada;
	CColladaSceneWriter writer(this);
	COLLADAFW::Root root(&collada, &writer);
	if (!root.loadDocument(path))
	{
		error("Fail to load file: %s", path);
		return false;
	}
	return true;
}

} // namespace wyc
