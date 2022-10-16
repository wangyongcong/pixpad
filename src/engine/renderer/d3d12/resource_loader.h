#pragma once
#include <functional>
#include "d3d12_type.h"
#include "renderer_d3d12.h"
#include "common/common_macros.h"

namespace wyc
{
	class RendererD3D12;

	enum ETaskType
	{
		TASK_UPLOAD,
	};

	struct ResourceTask
	{
		ETaskType type;
		DeviceResourceD3D12* resource;
		void* data;
		size_t size;
		int error;
		std::function<void(DeviceResourceD3D12*, bool)> callback;
	};

	class ResourceLoaderD3D12
	{
		DISALLOW_COPY_MOVE_AND_ASSIGN(ResourceLoaderD3D12);
	public:
		ResourceLoaderD3D12(RendererD3D12* renderer);
		~ResourceLoaderD3D12();
		bool start();
		void close();
		void wait_all();
		void update();
		void upload(DeviceResourceD3D12* resource, void* data, size_t size, std::function<void(DeviceResourceD3D12*, bool)> callback);

	protected:
		bool initialize();
		void clear();
		void upload_buffer(ResourceTask& task);

	private:
		RendererD3D12* m_renderer;
		// resource loading queue
		CommandQueueD3D12* m_loader_queue;
		CommandPoolD3D12* m_loader_pool;
		CommandListD3D12* m_loader_list;
		DeviceFenceD3D12* m_loader_fence;

		using TaskQueue = std::vector<ResourceTask>;
		TaskQueue m_queue1{};
		TaskQueue m_queue2{};
		TaskQueue* m_pending_tasks{};
		TaskQueue* m_commited_tasks{};
		bool m_is_initialized;
		bool m_is_started{};
	};
}
