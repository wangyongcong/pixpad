#include "engine_pch.h"
#include "renderer/d3d12/renderer_d3d12.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <dxgidebug.h>
// D3D12 extension library.
#include <d3dx12.h>

#include "common/assertion_macros.h"
#include "common/log_macros.h"
#include "common/memory.h"
#include "common/utility.h"
#include "platform/windows/windows_window.h"
#include "renderer/d3d12/d3d_helper.h"
#include "renderer/d3d12/d3d12_type.h"
#include "renderer/d3d12/resource_loader.h"

namespace wyc
{
#define D3D12_NEW_STRUCT(T, ptr)  T* ptr = wyc_new(T);\
	RENDERER_MAKE_STRUCT(ptr, ERendererName::Direct3D12, T);

#define D3D12_DELETE_STRUCT(ptr) wyc_delete(ptr)

	template<class To, class From>
	To* d3d12_struct_cast(From* ptr)
	{
		return RENDERER_CHECK_STRUCT(ptr, ERendererName::Direct3D12, To) ? (To*)(ptr) : nullptr;
	}

	template<class To, class From>
	const To* d3d12_struct_cast(const From* ptr)
	{
		return RENDERER_CHECK_STRUCT(ptr, ERendererName::Direct3D12, To) ? (const To*)(ptr) : nullptr;
	}

	static D3D12_COMMAND_LIST_TYPE s_d3d12_command_list_type[ECommandType::COMMAND_TYPE_COUNT] = {
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		D3D12_COMMAND_LIST_TYPE_COMPUTE,
		D3D12_COMMAND_LIST_TYPE_COPY
	};

	RendererD3D12::RendererD3D12()
		: m_device_state(ERenderDeviceState::DEVICE_EMPTY)
		, m_frame_buffer_count(3)
		, m_frame_buffer_index(0)
		, m_sample_count(1)
		, m_sample_quality(0)
		, m_frame_index(0)
		, m_descriptor_size{}
		, m_color_format()
		, m_depth_format()
		, m_viewport()
		, m_window_handle(nullptr)
		, m_gpu_info(nullptr)
		, m_debug_layer(nullptr)
		, m_dxgi_factory(nullptr)
		, m_adapter(nullptr)
		, m_device(nullptr)
		, m_device_info_queue(nullptr)
		, m_rtv_heap(nullptr)
		, m_dsv_heap(nullptr)
		, m_swap_chain(nullptr)
		, m_swap_chain_buffers(nullptr)
		, m_depth_buffer(nullptr)
		, m_command_queue(nullptr)
		, m_command_pools(nullptr)
		, m_command_lists(nullptr)
		, m_frame_fence(nullptr)
		, m_resource_loader(nullptr)
	{
	}

	RendererD3D12::~RendererD3D12()
	{
		release_device();
	}

	bool RendererD3D12::initialize(IGameWindow* game_window, const RendererConfig& config)
	{
		if (m_device_state >= ERenderDeviceState::DEVICE_INITIALIZED)
		{
			return true;
		}
		unsigned width, height;
		WindowsWindow* window = dynamic_cast<WindowsWindow*>(game_window);
		HWND hwnd = window->GetWindowHandle();
		game_window->get_window_size(width, height);
		if(!create_device(hwnd, width, height, config))
		{
			return false;
		}
		m_frame_buffer_count = config.frame_buffer_count;
		// check MSAA setting
		if(config.sample_count > 4 && m_gpu_info->msaa8_quality_level > 0)
		{
			m_sample_count = 8;
			m_sample_quality = m_gpu_info->msaa8_quality_level - 1;
		}
		else if(config.sample_count > 1 && m_gpu_info->msaa4_quality_level > 0)
		{
			m_sample_count = 4;
			m_sample_quality = m_gpu_info->msaa4_quality_level - 1;
		}
		else
		{
			m_sample_count = 1;
			m_sample_quality = 0;
		}

		// check render target color format
		switch(config.color_format)
		{
		case TinyImageFormat_R8G8B8A8_UNORM:
		case TinyImageFormat_R8G8B8A8_SRGB:
			m_color_format = DXGI_FORMAT_R8G8B8A8_UNORM;
			break;
		case TinyImageFormat_B8G8R8A8_UNORM:
		case TinyImageFormat_B8G8R8A8_SRGB:
			m_color_format = DXGI_FORMAT_B8G8R8A8_UNORM;
			break;
		default:
			m_color_format = DXGI_FORMAT_UNKNOWN;
			break;
		}

		// check depth/stencil buffer format
		switch(config.depth_stencil_format)
		{
		case TinyImageFormat_D16_UNORM:
			m_depth_format = DXGI_FORMAT_D16_UNORM;
			break;
		case TinyImageFormat_D24_UNORM_S8_UINT:
			m_depth_format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			break;
		case TinyImageFormat_D32_SFLOAT:
			m_depth_format = DXGI_FORMAT_D32_FLOAT;
			break;
		default:
			m_depth_format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			break;
		}

		m_viewport = {
			0, 0,
			(float)width, (float)height,
			0, 1.0f
		};

		// create command queue
		CommandQueueDesc queue_desc = {
			COMMAND_TYPE_DRAW, QUEUE_PRIORITY_NORMAL, 0
		};
		m_command_queue = (CommandQueueD3D12*)create_queue(queue_desc);
		if(!m_command_queue)
		{
			return false;
		}

		// create default command list
		m_command_pools = (CommandPoolD3D12**)wyc_calloc(m_frame_buffer_count, sizeof(void*));
		memset(m_command_pools, 0, sizeof(void*) * m_frame_buffer_count);

		m_command_lists = (CommandListD3D12**)wyc_calloc(m_frame_buffer_count, sizeof(void*));
		memset(m_command_lists, 0, sizeof(void*) * m_frame_buffer_count);

		for(uint8_t i = 0; i < m_frame_buffer_count; ++i)
		{
			CommandPoolD3D12* pool = (CommandPoolD3D12*)create_command_pool(m_command_queue);
			if(!pool)
			{
				LogError("Fail to create command pool %d", i);
				return false;
			}
			m_command_pools[i] = pool;
			CommandListD3D12* command_list = (CommandListD3D12*)create_command_list(pool);
			if(!command_list)
			{
				LogError("Fail to create command list %d", i);
				return false;
			}
			m_command_lists[i] = command_list;
		}

		m_frame_fence = (DeviceFenceD3D12*)create_fence(m_frame_buffer_count);
		if(!m_frame_fence)
		{
			LogError("Fail to crate frame fence");
			return false;
		}

		// create swap chain
		DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {
			width, height, m_color_format, FALSE,
			{ m_sample_count, m_sample_quality },
			DXGI_USAGE_RENDER_TARGET_OUTPUT,
			m_frame_buffer_count,
			DXGI_SCALING_STRETCH,
			DXGI_SWAP_EFFECT_FLIP_DISCARD,
			DXGI_ALPHA_MODE_UNSPECIFIED,
			DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH,
		};
		IDXGISwapChain1* swapChain1;
		if(FAILED(m_dxgi_factory->CreateSwapChainForHwnd(
			m_command_queue->queue,
			hwnd,
			&swap_chain_desc,
			nullptr,
			nullptr,
			&swapChain1
		)))
		{
			LogError("Fail to create swap chain.");
			return false;
		}
		if(FAILED((swapChain1->QueryInterface(IID_PPV_ARGS(&m_swap_chain)))))
		{
			LogError("Fail to query interface IDXGISwapChain4.");
			swapChain1->Release();
			return false;
		}
		swapChain1->Release();

		if(FAILED(m_dxgi_factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER)))
		{
			LogWarning("Fail to MakeWindowAssociation.");
		}

		// create RTV/DSV heap for swap chain
		D3D12_DESCRIPTOR_HEAP_DESC rtv_heap = {
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
			m_frame_buffer_count,
			D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
			0,
		};
		D3D12_DESCRIPTOR_HEAP_DESC dsv_heap = {
			D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
			1,
			D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
			0,
		};
		if(FAILED(m_device->CreateDescriptorHeap(&rtv_heap, IID_PPV_ARGS(&m_rtv_heap))))
		{
			LogError("Fail to create RTV heap");
			return false;
		}
		if(FAILED(m_device->CreateDescriptorHeap(&dsv_heap, IID_PPV_ARGS(&m_dsv_heap))))
		{
			LogError("Fail to create DSV heap");
			return false;
		}

		// create render target
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(m_rtv_heap->GetCPUDescriptorHandleForHeapStart());
		const unsigned descriptor_size = m_descriptor_size[D3D12_DESCRIPTOR_HEAP_TYPE_RTV];
		ID3D12Resource* buffer = nullptr;
		m_swap_chain_buffers = (ID3D12Resource**)wyc_calloc(m_frame_buffer_count, sizeof(ID3D12Resource*));
		for (int i = 0; i < m_frame_buffer_count; ++i)
		{
			m_swap_chain->GetBuffer(i, IID_PPV_ARGS(&buffer));
			m_device->CreateRenderTargetView(buffer, nullptr, rtv_handle);
			m_swap_chain_buffers[i] = buffer;
			rtv_handle.Offset(descriptor_size);
		}

		// create depth/stencil buffer
		D3D12_RESOURCE_DESC depth_desc = {
			D3D12_RESOURCE_DIMENSION_TEXTURE2D,
			0,
			width, height,
			1, 1,
			m_depth_format,
			{m_sample_count, m_sample_quality},
			D3D12_TEXTURE_LAYOUT_UNKNOWN,
			D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,
		};
		D3D12_CLEAR_VALUE clearDepth;
		clearDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		clearDepth.DepthStencil.Depth = 1.0f;
		clearDepth.DepthStencil.Stencil = 0;
		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
		if(FAILED(m_device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE, 
			&depth_desc, 
			D3D12_RESOURCE_STATE_COMMON, 
			&clearDepth, 
			IID_PPV_ARGS(&m_depth_buffer)
		)))
		{
			LogError("Fail to create depth/stencil buffer");
			return false;
		}

		D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc = {
			m_depth_format,
			D3D12_DSV_DIMENSION_TEXTURE2D,
			D3D12_DSV_FLAG_NONE,
		};
		dsv_desc.Texture2D.MipSlice = 0;
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsv_heap->GetCPUDescriptorHandleForHeapStart());
		m_device->CreateDepthStencilView(m_depth_buffer, &dsv_desc, dsvHandle);

		// initialize resource queue
		m_resource_loader = wyc_new(ResourceLoaderD3D12, this);
		m_resource_loader->start();

		// execute init commands
		auto cmd_list = m_command_lists[0]->command_list;
		cmd_list->Reset(m_command_pools[0]->allocator, NULL);
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_depth_buffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		cmd_list->ResourceBarrier(1, &barrier);
		cmd_list->Close();

		auto cmd_queue = m_command_queue->queue;
		ID3D12CommandList* const command_lists[] = { cmd_list };
		cmd_queue->ExecuteCommandLists(1, command_lists);
		m_frame_index += 1;
		m_frame_buffer_index = m_swap_chain->GetCurrentBackBufferIndex();
		cmd_queue->Signal(m_frame_fence->fence, m_frame_index);
		m_frame_fence->value[m_frame_buffer_index] = m_frame_index;

		m_device_state = ERenderDeviceState::DEVICE_INITIALIZED;
		return true;
	}

	void RendererD3D12::release()
	{
		release_device();
	}

	void RendererD3D12::close()
	{
		if(m_device_state == ERenderDeviceState::DEVICE_INITIALIZED)
		{
			wait_for_complete(m_frame_index);
			if(m_resource_loader)
			{
				m_resource_loader->close();
			}
			m_device_state = ERenderDeviceState::DEVICE_CLOSED;
		}
	}

	const GpuInfo& RendererD3D12::get_gpu_info(int index)
	{
		return *m_gpu_info;
	}

	DeviceResource* RendererD3D12::create_resource(EDeviceResourceType type, size_t size)
	{
		ID3D12Resource* resource;
		void* mapped_buffer = nullptr;
		if(type == CONSTANT_BUFFER)
		{
			// Constant buffers must be a multiple of the minimum hardware
			size = ALIGN_CONSTANT_BUFFER_SIZE(size);
			if(FAILED(m_device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), 
				D3D12_HEAP_FLAG_NONE, 
				&CD3DX12_RESOURCE_DESC::Buffer(size), 
				D3D12_RESOURCE_STATE_COMMON, 
				nullptr, 
				IID_PPV_ARGS(&resource))))
			{
				return nullptr;
			}
			if(FAILED(resource->Map(0, nullptr, &mapped_buffer)))
			{
				log_error("Can't map constant buffer");
			}
		}
		else
		{
			if(FAILED(m_device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), 
				D3D12_HEAP_FLAG_NONE, 
				&CD3DX12_RESOURCE_DESC::Buffer(size), 
				D3D12_RESOURCE_STATE_COMMON, 
				nullptr, 
				IID_PPV_ARGS(&resource))))
			{
				return nullptr;
			}
		}
		D3D12_NEW_STRUCT(DeviceResourceD3D12, d3d_res);
		d3d_res->size = size;
		d3d_res->type = type;
		d3d_res->resource = resource;
		if(type == CONSTANT_BUFFER)
		{
			d3d_res->mapped_buffer = (uint8_t*)mapped_buffer;
			d3d_res->mapped_size = size;
		}
		else
		{
			d3d_res->mapped_buffer = nullptr;
			d3d_res->mapped_size = 0;
		}
		return d3d_res;
	}

	void RendererD3D12::release_resource(DeviceResource* res)
	{
		DeviceResourceD3D12* d3d_res = d3d12_struct_cast<DeviceResourceD3D12>(res);
		if(!d3d_res)
		{
			log_error("[D3D] Expect D3D12 resource: type is %d.", RENDERER_STRUCT_TYPE(res));
			return;
		}
		if(d3d_res->mapped_buffer)
		{
			assert(d3d_res->resource);
			d3d_res->resource->Unmap(0, nullptr);
			d3d_res->mapped_buffer = nullptr;
		}
		SAFE_RELEASE(d3d_res->resource);
		D3D12_DELETE_STRUCT(d3d_res);
	}

	void RendererD3D12::upload_resource(DeviceResource* res, size_t offset, void* data, size_t size)
	{
		DeviceResourceD3D12* d3d_res = d3d12_struct_cast<DeviceResourceD3D12>(res);
		if(!d3d_res)
		{
			log_error("[D3D] Expect D3D12 resource: type is %d.", RENDERER_STRUCT_TYPE(res));
			return;
		}
		if(!d3d_res->resource)
		{
			log_error("[D3D] Resource is not valid.");
			return;
		}
		if(size > d3d_res->size)
		{
			log_error("[D3D] Resource buffer is not enough.");
			return;
		}

		if(d3d_res->type == CONSTANT_BUFFER)
		{
			if(d3d_res->mapped_buffer)
			{
				offset = ALIGN_CONSTANT_BUFFER_SIZE(offset);
				size = ALIGN_CONSTANT_BUFFER_SIZE(size);
				if(offset + size > d3d_res->mapped_size)
				{
					log_warning("constant buffer upload size is overflow");
					size = d3d_res->mapped_size - offset;
				}
				memcpy(d3d_res->mapped_buffer + offset, data, size);
			}
		}
		else
		{
			m_resource_loader->upload(d3d_res, offset, data, size, [this](DeviceResource* resource, bool is_succeed)
			{
				DeviceResourceD3D12* d3d12_res = (DeviceResourceD3D12*)resource;
				ID3D12GraphicsCommandList* cmd_list = m_command_lists[m_frame_buffer_index]->command_list;
				auto finish_barrier = CD3DX12_RESOURCE_BARRIER::Transition(d3d12_res->resource, 
				D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
				cmd_list->ResourceBarrier(1, &finish_barrier);
			});
		}
	}

	void RendererD3D12::begin_frame()
	{
		m_frame_index += 1;
		auto frame_fence_value = m_frame_fence->value[m_frame_buffer_index];
		wait_for_complete(frame_fence_value);

		ID3D12CommandAllocator* allocator = m_command_pools[m_frame_buffer_index]->allocator;
		ID3D12GraphicsCommandList* cmd_list = m_command_lists[m_frame_buffer_index]->command_list;
		// reset memory allocator
		allocator->Reset();
		// reset command list and start to record commands
		cmd_list->Reset(allocator, nullptr);

		// handle resource loading
		m_resource_loader->update();

		// init viewport
		cmd_list->RSSetViewports(1, &m_viewport);

		// wait until back buffer is ready
		auto frame_buffer = m_swap_chain_buffers[m_frame_buffer_index];
		D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			frame_buffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		cmd_list->ResourceBarrier(1, &barrier);

		// clear frame buffer
		float clear_color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(m_rtv_heap->GetCPUDescriptorHandleForHeapStart(), m_frame_buffer_index, m_descriptor_size[D3D12_DESCRIPTOR_HEAP_TYPE_RTV]);
		cmd_list->ClearRenderTargetView(rtv, clear_color, 0, nullptr);

		CD3DX12_CPU_DESCRIPTOR_HANDLE dsv(m_dsv_heap->GetCPUDescriptorHandleForHeapStart());
		cmd_list->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		// set render target
		cmd_list->OMSetRenderTargets(1, &rtv, true, &dsv);
	}

	void RendererD3D12::end_frame()
	{
	}

	void RendererD3D12::present()
	{
		ID3D12GraphicsCommandList* cmd_list = m_command_lists[m_frame_buffer_index]->command_list;
		auto frame_buffer = m_swap_chain_buffers[m_frame_buffer_index];
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			frame_buffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		cmd_list->ResourceBarrier(1, &barrier);
		cmd_list->Close();

		auto queue = m_command_queue->queue;
		ID3D12CommandList* const command_lists[] = { cmd_list };
		queue->ExecuteCommandLists(_countof(command_lists), command_lists);
		m_frame_fence->value[m_frame_buffer_index] = m_frame_index;
		EnsureHResult(queue->Signal(m_frame_fence->fence, m_frame_index));
		EnsureHResult(m_swap_chain->Present(0, 0));
		m_frame_buffer_index = (m_frame_buffer_index + 1) % m_frame_buffer_count;
	}

	void RendererD3D12::resize()
	{

	}

	bool RendererD3D12::create_device(HWND hwnd, uint32_t width, uint32_t height, const RendererConfig& config)
	{
		if(m_device)
		{
			return true;
		}

		m_window_handle = hwnd;

		if(config.enable_debug)
		{
			enable_debug_layer();
		}

		D3D_FEATURE_LEVEL featureLevels[4] =
		{
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
		};
		
		// query for hardware adapter
		UINT create_factory_flags = 0;
		if(config.enable_debug)
		{
			create_factory_flags = DXGI_CREATE_FACTORY_DEBUG;
		}
		EnsureHResult(CreateDXGIFactory2(create_factory_flags, IID_PPV_ARGS(&m_dxgi_factory)));

		int gpuCount = 0;
		IDXGIAdapter4* adapter;
		for (UINT i = 0; m_dxgi_factory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND; ++i)
		{
			DXGI_ADAPTER_DESC3 adapter_desc;
			adapter->GetDesc3(&adapter_desc);

			if(adapter_desc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)
			{
				adapter->Release();
				continue;
			}

			for (const D3D_FEATURE_LEVEL level: featureLevels)
			{
				if(SUCCEEDED(D3D12CreateDevice(adapter, level, __uuidof(ID3D12Device), nullptr)))
				{
					IDXGIAdapter4* gpuInterface;
					if(SUCCEEDED(adapter->QueryInterface(IID_PPV_ARGS(&gpuInterface))))
					{
						gpuCount += 1;
						SAFE_RELEASE(gpuInterface);
						break;
					}
				}
			}
			adapter->Release();
		}

		if(gpuCount < 1)
		{
			LogError("Can't find Direct3D12 device.");
			return false;
		}

		GpuInfoD3D12* gpu_info_list = (GpuInfoD3D12*)alloca(sizeof(GpuInfoD3D12) * gpuCount);
		memset(gpu_info_list, 0, sizeof(GpuInfoD3D12) * gpuCount);

		struct GpuHandle
		{
			IDXGIAdapter4* adpater;
			ID3D12Device2* device;
		};
		GpuHandle* gpu_list = (GpuHandle*)alloca(sizeof(GpuHandle) * gpuCount);
		memset(gpu_list, 0, sizeof(GpuHandle) * gpuCount);

		gpuCount = 0;
		for (UINT i = 0; m_dxgi_factory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND; ++i)
		{
			DXGI_ADAPTER_DESC3 adapter_desc;
			adapter->GetDesc3(&adapter_desc);

			if(adapter_desc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)
			{
				adapter->Release();
				continue;
			}
			for (const D3D_FEATURE_LEVEL level: featureLevels)
			{
				if(FAILED(D3D12CreateDevice(adapter, level, __uuidof(ID3D12Device), nullptr)))
				{
					continue;
				}

				if(FAILED(adapter->QueryInterface(IID_PPV_ARGS(&gpu_list[gpuCount].adpater))))
				{
					continue;
				}

				ID3D12Device2* device;
				D3D12CreateDevice(adapter, level, IID_PPV_ARGS(&device));
				gpu_list[gpuCount].device = device;
				auto &gpu = gpu_info_list[gpuCount];
				gpuCount += 1;

				gpu.feature_level = level;
				wcscpy_s(gpu.vendor_name, GPU_VENDOR_NAME_SIZE, adapter_desc.Description);
				gpu.vendor_id = adapter_desc.VendorId;
				gpu.device_id = adapter_desc.DeviceId;
				gpu.revision = adapter_desc.Revision;
				gpu.video_memory = adapter_desc.DedicatedVideoMemory;
				gpu.shared_memory = adapter_desc.SharedSystemMemory;
				device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &gpu.feature_data, sizeof(gpu.feature_data));
				device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS1, &gpu.feature_data1, sizeof(gpu.feature_data1));
				break;
			}
			adapter->Release();
		}

		// select the best one
		int gpu_index = 0;
		for(int i=1; i<gpuCount; ++i)
		{
			auto &gpu1 = gpu_info_list[gpu_index];
			auto &gpu2 = gpu_info_list[i];
			if(gpu2.feature_data1.WaveOps != gpu1.feature_data1.WaveOps)
			{
				if(gpu2.feature_data1.WaveOps)
				{
					gpu_index = i;
				}
				continue;
			}
			if(gpu2.feature_level != gpu1.feature_level)
			{
				if(gpu2.feature_level > gpu1.feature_level)
				{
					gpu_index = i;
				}
				continue;
			}
			if(gpu2.video_memory > gpu1.video_memory)
			{
				gpu_index = i;
			}
		}

		m_adapter = gpu_list[gpu_index].adpater;
		m_device = gpu_list[gpu_index].device;
		m_gpu_info = wyc_new(GpuInfoD3D12);
		memcpy(m_gpu_info, &gpu_info_list[gpu_index], sizeof(GpuInfo));

		for(int i=0; i<gpuCount; ++i)
		{
			if(i != gpu_index)
			{
				SAFE_RELEASE(gpu_list[i].adpater);
				SAFE_RELEASE(gpu_list[i].device);
			}
		}

		// check MSAA support
		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaa_quality_levels;
		msaa_quality_levels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		msaa_quality_levels.SampleCount = 4;
		msaa_quality_levels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
		msaa_quality_levels.NumQualityLevels = 0;
		if(SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaa_quality_levels, sizeof(msaa_quality_levels))))
		{
			m_gpu_info->msaa4_quality_level = (int) msaa_quality_levels.NumQualityLevels;
		}
		msaa_quality_levels.SampleCount = 8;
		msaa_quality_levels.NumQualityLevels = 0;
		if(SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaa_quality_levels, sizeof(msaa_quality_levels))))
		{
			m_gpu_info->msaa8_quality_level = (int) msaa_quality_levels.NumQualityLevels;
		}

		LogInfo("Device: %ls", m_gpu_info->vendor_name);
		LogInfo("Vendor ID: %d", m_gpu_info->vendor_id);
		LogInfo("Revision ID: %d", m_gpu_info->revision);
		float mem_size;
		const char* unit = format_memory_size(m_gpu_info->video_memory, mem_size);
		LogInfo("Video Memory: %.1f %s", mem_size, unit);
		unit = format_memory_size(m_gpu_info->shared_memory, mem_size);
		LogInfo("Shared Memory: %.1f %s", mem_size, unit);
		LogInfo("MSAA 4x: %d", m_gpu_info->msaa4_quality_level);
		LogInfo("MSAA 8x: %d", m_gpu_info->msaa8_quality_level);

		for(int i=0; i<D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
		{
			m_descriptor_size[i] = m_device->GetDescriptorHandleIncrementSize((D3D12_DESCRIPTOR_HEAP_TYPE)i);
		}

		if(config.enable_debug)
		{
			if (SUCCEEDED(m_device->QueryInterface(IID_PPV_ARGS(&m_device_info_queue))))
			{
				m_device_info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
				m_device_info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
				m_device_info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
			}

			// Suppress whole categories of messages
			// D3D12_MESSAGE_CATEGORY Categories[] = 
			// {
			// };

			// Suppress messages based on their severity level
			D3D12_MESSAGE_SEVERITY severities[] =
			{
				D3D12_MESSAGE_SEVERITY_INFO
			};

			// Suppress individual messages by their ID
			D3D12_MESSAGE_ID deny_ids[] = {
				D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
				D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
				D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
			};

			D3D12_INFO_QUEUE_FILTER info_filter = {};
			// 		NewinfoFilterFilter.DenyList.NumCategories = _countof(Categories);
			// 		infoFilter.DenyList.pCategoryList = Categories;
			info_filter.DenyList.NumSeverities = _countof(severities);
			info_filter.DenyList.pSeverityList = severities;
			info_filter.DenyList.NumIDs = _countof(deny_ids);
			info_filter.DenyList.pIDList = deny_ids;

			CheckAndReturnFalse(m_device_info_queue->PushStorageFilter(&info_filter));
		} // enable debug checks
		
		return true;
	}

	void RendererD3D12::release_device()
	{
		if(m_device_state >= ERenderDeviceState::DEVICE_RELEASED || m_device_state < ERenderDeviceState::DEVICE_INITIALIZED)
		{
			return;
		}
		m_device_state = ERenderDeviceState::DEVICE_RELEASED;
		if(m_resource_loader)
		{
			wyc_delete(m_resource_loader);
			m_resource_loader = nullptr;
		}
		if(m_command_pools)
		{
			for (uint8_t i = 0; i < m_frame_buffer_count; ++i)
			{
				if(auto pool = m_command_pools[i])
				{
					release_command_pool(pool);
				}
				if(auto command_list = m_command_lists[i])
				{
					release_command_list(command_list);
				}
			}
			wyc_free(m_command_pools);
			wyc_free(m_command_lists);
			m_command_pools = nullptr;
			m_command_lists = nullptr;
		}
		if(m_command_queue)
		{
			release_queue(m_command_queue);
			m_command_queue = nullptr;
		}
		if(m_frame_fence)
		{
			release_fence(m_frame_fence);
			m_frame_fence = nullptr;
		}

		if (m_swap_chain_buffers)
		{
			for (int i = 0; i < m_frame_buffer_count; ++i)
			{
				ID3D12Resource* buff = m_swap_chain_buffers[i];
				SAFE_RELEASE(buff);
			}
			wyc_free(m_swap_chain_buffers);
			m_swap_chain_buffers = nullptr;
		}
		SAFE_RELEASE(m_depth_buffer);
		SAFE_RELEASE(m_swap_chain);
		SAFE_RELEASE(m_rtv_heap);
		SAFE_RELEASE(m_dsv_heap);

		SAFE_RELEASE(m_adapter);
		SAFE_RELEASE(m_dxgi_factory);

		if(m_device_info_queue)
		{
			m_device_info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, false);
			m_device_info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, false);
			m_device_info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);
			SAFE_RELEASE(m_device_info_queue);
		}
		if(m_gpu_info)
		{
			wyc_delete(m_gpu_info);
			m_gpu_info = nullptr;
		}
		SAFE_RELEASE(m_debug_layer);
		SAFE_RELEASE(m_device);
		report_live_objects(L"Check leaks on shutdown");
	}

	void RendererD3D12::report_live_objects(const wchar_t* prompt)
	{
#ifdef _DEBUG
		if(prompt)
		{
			OutputDebugStringW(L"[");
			OutputDebugStringW(prompt);
			OutputDebugStringW(L"]\n");
		}
		ComPtr<IDXGIDebug1> dxgi_debug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgi_debug))))
		{
			dxgi_debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
		}
#endif
	}

	void RendererD3D12::wait_for_complete(uint64_t frame_index)
	{
		if(m_frame_fence->fence->GetCompletedValue() < frame_index)
		{
			m_frame_fence->fence->SetEventOnCompletion(frame_index, m_frame_fence->wait_event);
			WaitForSingleObject(m_frame_fence->wait_event, INFINITE);
		}
	}

	CommandQueue* RendererD3D12::create_queue(const CommandQueueDesc& desc)
	{
		// create command queue
		D3D12_COMMAND_QUEUE_DESC command_queue_desc;
		command_queue_desc.Type = s_d3d12_command_list_type[desc.type];
		command_queue_desc.Priority = (int)desc.priority;
		command_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		command_queue_desc.NodeMask = desc.node;
		ID3D12CommandQueue* d3d_queue;
		if(FAILED(m_device->CreateCommandQueue(&command_queue_desc, IID_PPV_ARGS(&d3d_queue))))
		{
			LogError("Fail to create command queue.");
			return nullptr;
		}
		D3D12_NEW_STRUCT(CommandQueueD3D12, queue);
		queue->type = desc.type;
		queue->priority = desc.priority;
		queue->node = desc.node;
		queue->queue = d3d_queue;
		return queue;
	}

	void RendererD3D12::release_queue(CommandQueue* queue)
	{
		CommandQueueD3D12* d3d_queue = d3d12_struct_cast<CommandQueueD3D12>(queue);
		if(!d3d_queue)
		{
			log_error("Type is not matched (expected D3D12CommandQueue)", RENDERER_STRUCT_TYPE(queue));
			return;
		}
		SAFE_RELEASE(d3d_queue->queue);
		D3D12_DELETE_STRUCT(d3d_queue);
	}

	CommandPool* RendererD3D12::create_command_pool(CommandQueue* queue)
	{
		CommandQueueD3D12* d3d_queue = d3d12_struct_cast<CommandQueueD3D12>(queue);
		if(!d3d_queue)
		{
			log_error("Type is not matched (expected D3D12CommandQueue): %d", RENDERER_STRUCT_TYPE(queue));
			return nullptr;
		}
		ID3D12CommandAllocator* allocator;
		if (FAILED(m_device->CreateCommandAllocator(s_d3d12_command_list_type[d3d_queue->type], IID_PPV_ARGS(&allocator))))
		{
			LogError("Fail to create command allocator");
			return nullptr;
		}
		D3D12_NEW_STRUCT(CommandPoolD3D12, d3d_pool);
		d3d_pool->queue = queue;
		d3d_pool->allocator = allocator;
		return d3d_pool;
	}

	void RendererD3D12::release_command_pool(CommandPool* pool)
	{
		CommandPoolD3D12* d3d_pool = d3d12_struct_cast<CommandPoolD3D12>(pool);
		if(!d3d_pool)
		{
			log_error("Type is not matched (expected D3D12CommandPool): %d", RENDERER_STRUCT_TYPE(pool));
			return;
		}
		SAFE_RELEASE(d3d_pool->allocator);
		D3D12_DELETE_STRUCT(d3d_pool);
	}

	CommandList* RendererD3D12::create_command_list(CommandPool* pool)
	{
		CommandPoolD3D12* d3d_pool = d3d12_struct_cast<CommandPoolD3D12>(pool);
		if(!d3d_pool)
		{
			log_error("Type is not matched (expected D3D12CommandPool): %d", RENDERER_STRUCT_TYPE(pool));
			return nullptr;
		}
		CommandQueueD3D12* d3d_queue = (CommandQueueD3D12*)d3d_pool->queue;
		ID3D12GraphicsCommandList* command_list;
		if (FAILED(m_device->CreateCommandList(d3d_queue->node, s_d3d12_command_list_type[d3d_queue->type], 
			d3d_pool->allocator, nullptr, IID_PPV_ARGS(&command_list)
		)))
		{
			LogError("Fail to create command list");
			return nullptr;
		}
		command_list->Close();
		D3D12_NEW_STRUCT(CommandListD3D12, d3d_commands);
		d3d_commands->pool = pool;
		d3d_commands->command_list = command_list;
		return d3d_commands;
	}

	void RendererD3D12::release_command_list(CommandList* cmd_list)
	{
		CommandListD3D12* d3d_cmds = d3d12_struct_cast<CommandListD3D12>(cmd_list);
		if(!d3d_cmds)
		{
			log_error("Type is not matched (expected D3D12CommandList): %d", RENDERER_STRUCT_TYPE(cmd_list));
			return;
		}
		SAFE_RELEASE(d3d_cmds->command_list);
		D3D12_DELETE_STRUCT(d3d_cmds);
	}

	DeviceFence* RendererD3D12::create_fence(unsigned value_count)
	{
		ID3D12Fence* d3d_fence;
		if(FAILED((m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&d3d_fence)))))
		{
			LogError("Fail to crate D3D12 fence");
			return nullptr;
		}
		HANDLE handle = CreateEvent(NULL, FALSE, FALSE, NULL);
		if(handle == NULL)
		{
			d3d_fence->Release();
			LogError("Fail to crate event for fence");
			return nullptr;
		}
		size_t sz = sizeof(DeviceFenceD3D12) + sizeof(uint64_t) * (value_count - 1);
		DeviceFenceD3D12* fence = (DeviceFenceD3D12*) wyc_malloc(sz);
		RENDERER_MAKE_STRUCT(fence, ERendererName::Direct3D12, DeviceFenceD3D12);
		fence->fence = d3d_fence;
		fence->wait_event = handle;
		fence->count = value_count;
		memset(fence->value, 0, value_count * sizeof(uint64_t));
		return fence;
	}

	void RendererD3D12::release_fence(DeviceFence* fence)
	{
		DeviceFenceD3D12* d3d_fence = d3d12_struct_cast<DeviceFenceD3D12>(fence);
		if(!d3d_fence)
		{
			log_error("Type is not matched (expected DeviceFenceD3D12): %d", RENDERER_STRUCT_TYPE(fence));
			return;
		}
		SAFE_RELEASE(d3d_fence->fence);
		SAFE_CLOSE_HANDLE(d3d_fence->wait_event);
		wyc_free(d3d_fence);
	}

	bool RendererD3D12::is_fence_completed(DeviceFence* fence, unsigned index)
	{
		DeviceFenceD3D12* d3d_fence = d3d12_struct_cast<DeviceFenceD3D12>(fence);
		if(!d3d_fence)
		{
			log_error("Type is not matched (expected DeviceFenceD3D12): %d", RENDERER_STRUCT_TYPE(fence));
			return false;
		}
		if(index >= d3d_fence->count)
		{
			log_error("Invalid fence value index.");
			return false;
		}
		return d3d_fence->fence->GetCompletedValue() >= d3d_fence->value[index];
	}

	void RendererD3D12::wait_for_fence(DeviceFence* fence, unsigned index)
	{
		DeviceFenceD3D12* d3d_fence = d3d12_struct_cast<DeviceFenceD3D12>(fence);
		if(!d3d_fence)
		{
			log_error("Type is not matched (expected DeviceFenceD3D12): %d", RENDERER_STRUCT_TYPE(fence));
			return;
		}
		if(index >= d3d_fence->count)
		{
			log_error("Invalid fence value index.");
			return;
		}
		uint64_t fence_value = d3d_fence->value[index];
		ID3D12Fence* hw_fence = d3d_fence->fence;
		if(hw_fence->GetCompletedValue() < fence_value)
		{
			hw_fence->SetEventOnCompletion(fence_value, d3d_fence->wait_event);
			WaitForSingleObject(d3d_fence->wait_event, INFINITE);
		}
	}

	void RendererD3D12::enable_debug_layer()
	{
		if(SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debug_layer))))
		{
			m_debug_layer->EnableDebugLayer();
			ID3D12Debug1* debug1 = NULL;
			if (SUCCEEDED(m_debug_layer->QueryInterface(IID_PPV_ARGS(&debug1))))
			{
				debug1->SetEnableGPUBasedValidation(TRUE);
				debug1->Release();
			}
		}
	}
} // namespace wyc
