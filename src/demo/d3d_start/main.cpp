#include "game_application.h"
#include "game_instance.h"
#include "rtm/types.h"

using namespace wyc;
using namespace rtm;

struct Vertex
{
	float3f position;
	float4f color;
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
	
	void initialize() override
	{
	}


	void exit() override
	{
	}


	void tick(float deltaTime) override
	{
	}


	void draw(IRenderer* pRenderer) override
	{
	}

};

APPLICATION_MAIN(D3DStartDemo)
