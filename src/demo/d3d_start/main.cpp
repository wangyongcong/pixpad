#include "engine.h"
#include "common/log_macros.h"
#include "common/memory.h"
#include "common/task_thread.h"
#include "game/game_application.h"
#include "game/game_instance.h"
// #include "rtm/types.h"
#include "renderer/mesh.h"

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
	DISALLOW_COPY_MOVE_AND_ASSIGN(D3DStartDemo);
public:
	D3DStartDemo()
		: m_mesh(nullptr)
		, m_gpu_vb(nullptr)
		, m_task_thread(nullptr)
	{
	}

	void initialize() override
	{
		m_task_thread = wyc_new(TaskThread);
		m_task_thread->start();
		m_frame_count = 0;
		m_mesh = wyc_new(CMesh);
		m_is_mesh_loaded = m_task_thread->enqueue(
		[](CMesh* mesh)->bool {
			return mesh->load("assets/icosahedron.ply");
		}, m_mesh);

	}

	void exit() override
	{
		m_task_thread->stop();
		wyc_delete(m_task_thread);
		m_task_thread = nullptr;

		auto renderer = g_application->get_renderer();
		if(m_gpu_vb)
		{
			renderer->release_resource(m_gpu_vb);
		}
		wyc_safe_delete(m_mesh);
	}


	void tick(float delta_time) override
	{
		m_frame_count += 1;
		if(m_is_mesh_loaded.valid())
		{
			if(std::future_status::ready == m_is_mesh_loaded.wait_for(std::chrono::milliseconds(0)))
			{
				bool is_ready = m_is_mesh_loaded.get();
				if(!is_ready)
				{
					log_error("Fail to load mesh");
				}
				else
				{
					log_debug("Mesh is loaded at frame %lld", m_frame_count);
				}
				const VertexBuffer& vb = m_mesh->vertex_buffer();
				auto renderer = g_application->get_renderer();
				m_gpu_vb = renderer->create_resource(VERTEX_BUFFER, vb.data_size());
				log_debug("Upload resource: %p", m_gpu_vb);
				renderer->upload_resource(m_gpu_vb, (void*)vb.data(), vb.data_size());
			}
		}
	}


	void draw(IRenderer* renderer) override
	{
	}

private:
	CMesh *m_mesh;
	DeviceResource* m_gpu_vb;
	TaskThread* m_task_thread;
	std::future<bool> m_is_mesh_loaded;
	uint64_t m_frame_count;
};

APPLICATION_MAIN(D3DStartDemo)
