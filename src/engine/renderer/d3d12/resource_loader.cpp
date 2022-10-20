#include "engine_pch.h"
#include "renderer/d3d12/resource_loader.h"
#include "common/memory.h"
#include "stb/stb_log.h"

namespace wyc
{
	ResourceLoaderD3D12::ResourceLoaderD3D12(RendererD3D12* renderer)
		: m_renderer(renderer)
		, m_loader_queue(nullptr)
		, m_loader_pool(nullptr)
		, m_loader_list(nullptr)
		, m_loader_fence(nullptr)
		, m_is_initialized(false)
		, m_is_started(false)
	{
		m_pending_tasks = &m_queue1;
		m_commited_tasks = &m_queue2;
		initialize();
	}

	ResourceLoaderD3D12::~ResourceLoaderD3D12()
	{
		clear();
	}

	bool ResourceLoaderD3D12::start()
	{
		if(!m_is_initialized)
		{
			return false;
		}
		if(!m_is_started)
		{
			m_is_started = true;
		}
		return true;
	}

	void ResourceLoaderD3D12::close()
	{
		if(m_is_started)
		{
			wait_all();
			m_is_started = false;
		}
	}

	void ResourceLoaderD3D12::wait_all()
	{
		if(!m_loader_fence) return;
		m_renderer->wait_for_fence(m_loader_fence, 0);
	}

	void ResourceLoaderD3D12::update()
	{
		if(!m_is_started) return;
		auto wait_value = m_loader_fence->value[0];
		if(!m_renderer->is_fence_completed(m_loader_fence))
		{
			return;	
		}

		if(!m_commited_tasks->empty())
		{
			// notify task finished
			for(auto &task : *m_commited_tasks)
			{
				log_debug("Resource is uploaded: %p", task.resource);
				task.callback(task.resource, task.error == 0);
			}
			m_commited_tasks->clear();
		}

		if(m_pending_tasks->empty())
		{
			return;
		}

		// begin collect commands
		auto cmd_list = m_loader_list->command_list;
		m_loader_pool->allocator->Reset();
		cmd_list->Reset(m_loader_pool->allocator, nullptr);

		for (auto &task : *m_pending_tasks)
		{
			switch (task.type)
			{
			case TASK_UPLOAD:
				upload_buffer(task);
				break;
			default: 
				break;
			}
		}
		cmd_list->Close();

		// commit commands
		auto queue = m_loader_queue->queue;
		ID3D12CommandList* const command_lists[] = { cmd_list };
		queue->ExecuteCommandLists(_countof(command_lists), command_lists);
		wait_value += 1;
		m_loader_fence->value[0] = wait_value;
		queue->Signal(m_loader_fence->fence, wait_value);

		// swap task queue
		std::swap(m_commited_tasks, m_pending_tasks);
	}

	void ResourceLoaderD3D12::upload(DeviceResourceD3D12* resource, void* data, size_t size, std::function<void(DeviceResourceD3D12*, bool)> callback)
	{
		if(m_is_started)
		{
			m_pending_tasks->emplace_back(ResourceTask{TASK_UPLOAD, resource, data, size, 0, callback});
		}
	}

	bool ResourceLoaderD3D12::initialize()
	{
		if(!m_renderer)
		{
			return false;
		}
		if(m_is_initialized)
		{
			return true;
		}
		m_loader_queue = (CommandQueueD3D12*)m_renderer->create_queue(CommandQueueDesc{COMMAND_TYPE_COPY, QUEUE_PRIORITY_NORMAL, 0});
		if(!m_loader_queue)
		{
			log_error("Fail to create loader command queue");
			return false;
		}
		m_loader_pool = (CommandPoolD3D12*)m_renderer->create_command_pool(m_loader_queue);
		if(!m_loader_pool)
		{
			log_error("Fail to create loader command pool");
			return false;
		}
		m_loader_list = (CommandListD3D12*)m_renderer->create_command_list(m_loader_pool);
		if(!m_loader_list)
		{
			log_error("Fail to create loader command list");
			return false;
		}
		m_loader_fence = (DeviceFenceD3D12*)m_renderer->create_fence(1);
		if(!m_loader_fence)
		{
			log_error("Fail to create loader fence");
			return false;
		}
		m_is_initialized = true;
		return true;
	}

	void ResourceLoaderD3D12::clear()
	{
		if(m_loader_fence)
		{
			m_renderer->release_fence(m_loader_fence);
			m_loader_fence = nullptr;
		}
		if(m_loader_list)
		{
			m_renderer->release_command_list(m_loader_list);
			m_loader_list = nullptr;
		}
		if(m_loader_pool)
		{
			m_renderer->release_command_pool(m_loader_pool);
			m_loader_pool = nullptr;
		}
		if(m_loader_queue)
		{
			m_renderer->release_queue(m_loader_queue);
			m_loader_queue = nullptr;
		}
	}

	void ResourceLoaderD3D12::upload_buffer(ResourceTask& task)
	{
		D3D12_RESOURCE_DESC res_desc = CD3DX12_RESOURCE_DESC::Buffer(task.size);
		D3D12_HEAP_PROPERTIES heap_prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		ID3D12Resource* upload;
		auto device = m_renderer->GetDevice();
		if(FAILED(device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE, &res_desc, 
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&upload))))
		{
			log_error("[D3D] Failed to create update buffer");
			task.error = 1;
			return;
		}
		D3D12_SUBRESOURCE_DATA res_data;
		res_data.pData = task.data;
		res_data.RowPitch = task.size;
		res_data.SlicePitch = task.size;
		auto d3d_res = task.resource->resource;
		auto cmd_list = m_loader_list->command_list;

		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(d3d_res, 
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
		cmd_list->ResourceBarrier(1, &barrier);

		UpdateSubresources<1>(cmd_list, d3d_res, upload, 0, 0, 1, &res_data);

		barrier = CD3DX12_RESOURCE_BARRIER::Transition(d3d_res,
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
		cmd_list->ResourceBarrier(1, &barrier);

		auto client_callback = task.callback;
		task.callback =[upload, client_callback](DeviceResourceD3D12* arg1, bool arg2)
		{
			upload->Release();
			client_callback(arg1, arg2);
		};
	}
}
