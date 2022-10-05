#include "engine.h"
#include "common/log_macros.h"
#include "common/memory.h"
#include "game/game_application.h"
#include "game/game_instance.h"
// #include "rtm/types.h"
#include "renderer/mesh.h"
#include <sstream>

using namespace wyc;
// using namespace rtm;

struct Vertex
{
	vec3f position;
	vec4f color;
};

void GenerateTriangleVertex(float r)
{
	const float sin30 = 0.5f, cos30 = 0.866f;
	Vertex vertices[] = {
		{
			{ 0, r, 0 },
			{ 1.0f, 0, 0, 1.0f },
		},
		{
			{ -r * cos30, -r * sin30, 0 },
			{ 0, 1.0f, 0, 1.0f },
		},
		{
			{ r * cos30, -r * sin30, 0 },
			{ 0, 0, 1.0f, 1.0f },
		},
	};
}

class D3DStartDemo : public IGameInstance
{
public:
	D3DStartDemo()
		: m_mesh(nullptr)
	{
		
	}

	void initialize() override
	{
		m_mesh = wyc_new(CMesh);
		if(!m_mesh->load("assets/sofa.ply"))
		{
			log_error("Can't load mesh");
		}
	}


	void exit() override
	{
		wyc_safe_delete(m_mesh);
	}


	void tick(float deltaTime) override
	{
	}


	void draw(IRenderer* pRenderer) override
	{
	}

private:
	CMesh *m_mesh;
};

APPLICATION_MAIN(D3DStartDemo)
