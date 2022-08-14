#include "game_framework_pch.h"
#include "game_framework.h"
#include "renderer_d3d12.h"
#include <chrono>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <dxgidebug.h>
// D3D12 extension library.
#include <d3dx12.h>

#include "assertion_macros.h"
#include "log_macros.h"
#include "windows_window.h"
#include "d3d_helper.h"
#include "memory.h"


namespace wyc
{
	RendererD3D12::RendererD3D12()
		: m_device_state(ERenderDeviceState::DEVICE_EMPTY)
		, m_frame_buffer_count(3)
		, m_frame_buffer_index(0)
		, m_sample_count(1)
		, m_sample_quality()
		, m_frame_index()
		, m_descriptor_size{}
		, m_color_format()
		, m_depth_format()
		, m_window_handle(nullptr)
		, m_gpu_info()
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
		, m_command_list(nullptr)
		, m_command_allocators(nullptr)
		, m_frame_fence(nullptr)
	{
	}

	RendererD3D12::~RendererD3D12()
	{
		release();
	}

	bool RendererD3D12::initialize(IGameWindow* gameWindow, const RendererConfig& config)
	{
		if (m_device_state >= ERenderDeviceState::DEVICE_INITIALIZED)
		{
			return true;
		}
		unsigned width, height;
		WindowsWindow* window = dynamic_cast<WindowsWindow*>(gameWindow);
		HWND hWnd = window->GetWindowHandle();
		gameWindow->get_window_size(width, height);
		if(!CreateDevice(hWnd, width, height, config))
		{
			return false;
		}
		m_frame_buffer_count = config.frame_buffer_count;
		// check MSAA setting
		if(config.sample_count > 4 && m_gpu_info.msaa8_quality_level > 0)
		{
			m_sample_count = 8;
			m_sample_quality = m_gpu_info.msaa8_quality_level - 1;
		}
		else if(config.sample_count > 1 && m_gpu_info.msaa4_quality_level > 0)
		{
			m_sample_count = 4;
			m_sample_quality = m_gpu_info.msaa4_quality_level - 1;
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

		// create command queue
		D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
		commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		commandQueueDesc.Priority = 0;
		commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		commandQueueDesc.NodeMask = 0;
		if(FAILED(m_device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&m_command_queue))))
		{
			LogError("Fail to create default command queue.");
			return false;
		}

		// create default command list
		m_command_allocators = (ID3D12CommandAllocator**)tf_calloc(m_frame_buffer_count, sizeof(ID3D12CommandAllocator*));
		memset(m_command_allocators, 0, sizeof(ID3D12CommandAllocator*) * m_frame_buffer_count);
		// mpCommandFences = (D3D12Fence*)tf_calloc_memalign(m_frame_buffer_count, alignof(D3D12Fence), sizeof(D3D12Fence));
		for(uint8_t i = 0; i < m_frame_buffer_count; ++i)
		{
			if(FAILED(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_command_allocators[i]))))
			{
				LogError("Fail to create command allocator %d", i);
				return false;
			}
			// Ensure(CreateFence(mpCommandFences[i]));
		}
		if(FAILED(m_device->CreateCommandList(
			0, D3D12_COMMAND_LIST_TYPE_DIRECT, 
			m_command_allocators[0], nullptr, 
			IID_PPV_ARGS(&m_command_list)
		))) 
		{
			LogError("Fail to create default command list");
			return false;
		}

		size_t sz = sizeof(DeviceFence) + sizeof(uint64_t) * (m_frame_buffer_count - 1);
		m_frame_fence = (DeviceFence*) tf_malloc(sz);
		memset(m_frame_fence, 0, sz);

		if(FAILED((m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_frame_fence->fence)))))
		{
			LogError("Fail to crate frame fence");
			return false;
		}
		m_frame_fence->wait_event = CreateEvent(NULL, FALSE, FALSE, NULL);
		if(m_frame_fence->wait_event == NULL)
		{
			LogError("Fail to crate event for fence");
			return false;
		}

		// create swap chain
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {
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
			m_command_queue,
			hWnd,
			&swapChainDesc,
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

		if(FAILED(m_dxgi_factory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER)))
		{
			LogWarning("Fail to MakeWindowAssociation.");
		}

		// create RTV/DSV heap for swap chain
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeap = {
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
			m_frame_buffer_count,
			D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
			0,
		};
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeap = {
			D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
			1,
			D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
			0,
		};
		if(FAILED(m_device->CreateDescriptorHeap(&rtvHeap, IID_PPV_ARGS(&m_rtv_heap))))
		{
			LogError("Fail to create RTV heap");
			return false;
		}
		if(FAILED(m_device->CreateDescriptorHeap(&dsvHeap, IID_PPV_ARGS(&m_dsv_heap))))
		{
			LogError("Fail to create DSV heap");
			return false;
		}

		// create render target
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtv_heap->GetCPUDescriptorHandleForHeapStart());
		const unsigned descriptorSize = m_descriptor_size[D3D12_DESCRIPTOR_HEAP_TYPE_RTV];
		ID3D12Resource* buffer = nullptr;
		m_swap_chain_buffers = (ID3D12Resource**)tf_calloc(m_frame_buffer_count, sizeof(ID3D12Resource*));
		for (int i = 0; i < m_frame_buffer_count; ++i)
		{
			m_swap_chain->GetBuffer(i, IID_PPV_ARGS(&buffer));
			m_device->CreateRenderTargetView(buffer, nullptr, rtvHandle);
			m_swap_chain_buffers[i] = buffer;
			rtvHandle.Offset(descriptorSize);
		}

		// create depth/stencil buffer
		D3D12_RESOURCE_DESC depthDesc = {
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
			&depthDesc, 
			D3D12_RESOURCE_STATE_COMMON, 
			&clearDepth, 
			IID_PPV_ARGS(&m_depth_buffer)
		)))
		{
			LogError("Fail to create depth/stencil buffer");
			return false;
		}

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {
			m_depth_format,
			D3D12_DSV_DIMENSION_TEXTURE2D,
			D3D12_DSV_FLAG_NONE,
		};
		dsvDesc.Texture2D.MipSlice = 0;
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsv_heap->GetCPUDescriptorHandleForHeapStart());
		m_device->CreateDepthStencilView(m_depth_buffer, &dsvDesc, dsvHandle);

		// execute init commands
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_depth_buffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		m_command_list->ResourceBarrier(1, &barrier);
		m_command_list->Close();

		ID3D12CommandList* const commandLists[] = { m_command_list };
		m_command_queue->ExecuteCommandLists(1, commandLists);
		m_frame_index += 1;
		m_frame_buffer_index = m_swap_chain->GetCurrentBackBufferIndex();
		m_command_queue->Signal(m_frame_fence->fence, m_frame_index);
		m_frame_fence->value[m_frame_buffer_index] = m_frame_index;

		m_device_state = ERenderDeviceState::DEVICE_INITIALIZED;
		return true;
	}

	void RendererD3D12::release()
	{
		if(m_device_state >= ERenderDeviceState::DEVICE_RELEASED || m_device_state < ERenderDeviceState::DEVICE_INITIALIZED)
		{
			return;
		}
		m_device_state = ERenderDeviceState::DEVICE_RELEASED;

		if (m_command_allocators)
		{
			for (uint8_t i = 0; i < m_frame_buffer_count; ++i)
			{
				m_command_allocators[i]->Release();
			}
			tf_free(m_command_allocators);
			m_command_allocators = nullptr;
		}

		if (m_swap_chain_buffers)
		{
			for (int i = 0; i < m_frame_buffer_count; ++i)
			{
				ID3D12Resource* buff = m_swap_chain_buffers[i];
				SAFE_RELEASE(buff);
			}
			tf_free(m_swap_chain_buffers);
			m_swap_chain_buffers = nullptr;
		}
		SAFE_RELEASE(m_depth_buffer);
		SAFE_RELEASE(m_swap_chain);
		SAFE_RELEASE(m_rtv_heap);
		SAFE_RELEASE(m_dsv_heap);

		SAFE_RELEASE(m_command_queue);
		SAFE_RELEASE(m_command_list);
		if(m_frame_fence)
		{
			SAFE_RELEASE(m_frame_fence->fence);
			SAFE_CLOSE_HANDLE(m_frame_fence->wait_event);
			tf_free(m_frame_fence);
			m_frame_fence = nullptr;
		}

		SAFE_RELEASE(m_adapter);
		SAFE_RELEASE(m_dxgi_factory);

		if(m_device_info_queue)
		{
			m_device_info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, false);
			m_device_info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, false);
			m_device_info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);
			SAFE_RELEASE(m_device_info_queue);
		}
		SAFE_RELEASE(m_debug_layer);
		SAFE_RELEASE(m_device);

		ReportLiveObjects(L"Check leaks on shutdown");
	}

	void RendererD3D12::close()
	{
		if(m_device_state == ERenderDeviceState::DEVICE_INITIALIZED)
		{
			WaitForComplete(m_frame_index);
			m_device_state = ERenderDeviceState::DEVICE_CLOSED;
		}
	}

	const GpuInfo& RendererD3D12::get_gpu_info(int index)
	{
		return m_gpu_info;
	}

	void RendererD3D12::begin_frame()
	{
		m_frame_index += 1;
		auto frameFenceValue = m_frame_fence->value[m_frame_buffer_index];
		WaitForComplete(frameFenceValue);

		ID3D12CommandAllocator* pAllocator = m_command_allocators[m_frame_buffer_index];
		pAllocator->Reset();
		m_command_list->Reset(pAllocator, nullptr);

		auto frameBuffer = m_swap_chain_buffers[m_frame_buffer_index];
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			frameBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		m_command_list->ResourceBarrier(1, &barrier);
		
		float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(m_rtv_heap->GetCPUDescriptorHandleForHeapStart(), m_frame_buffer_index, m_descriptor_size[D3D12_DESCRIPTOR_HEAP_TYPE_RTV]);
		m_command_list->ClearRenderTargetView(rtv, clearColor, 0, nullptr);

		CD3DX12_CPU_DESCRIPTOR_HANDLE dsv(m_dsv_heap->GetCPUDescriptorHandleForHeapStart());
		m_command_list->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	}

	void RendererD3D12::end_frame()
	{
	}

	void RendererD3D12::present()
	{
		auto frameBuffer = m_swap_chain_buffers[m_frame_buffer_index];
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			frameBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		m_command_list->ResourceBarrier(1, &barrier);
		m_command_list->Close();

		ID3D12CommandList* const commandLists[] = { m_command_list };
		m_command_queue->ExecuteCommandLists(_countof(commandLists), commandLists);
		m_frame_fence->value[m_frame_buffer_index] = m_frame_index;
		EnsureHResult(m_command_queue->Signal(m_frame_fence->fence, m_frame_index));
		EnsureHResult(m_swap_chain->Present(0, 0));
		m_frame_buffer_index = (m_frame_buffer_index + 1) % m_frame_buffer_count;
	}

	void RendererD3D12::resize()
	{

	}

	bool RendererD3D12::CreateDevice(HWND hWnd, uint32_t width, uint32_t height, const RendererConfig& config)
	{
		if(m_device)
		{
			return true;
		}

		m_window_handle = hWnd;

		if(config.enable_debug)
		{
			EnableDebugLayer();
		}

		D3D_FEATURE_LEVEL featureLevels[4] =
		{
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
		};
		
		// query for hardware adapter
		UINT createFactoryFlags = 0;
		if(config.enable_debug)
		{
			createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
		}
		EnsureHResult(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&m_dxgi_factory)));

		int gpuCount = 0;
		IDXGIAdapter4* adapter;
		for (UINT i = 0; m_dxgi_factory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND; ++i)
		{
			DXGI_ADAPTER_DESC3 adapterDesc;
			adapter->GetDesc3(&adapterDesc);

			if(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)
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

		D3D12GpuInfo* gpuInfoList = (D3D12GpuInfo*)alloca(sizeof(D3D12GpuInfo) * gpuCount);
		memset(gpuInfoList, 0, sizeof(D3D12GpuInfo) * gpuCount);

		struct GpuHandle
		{
			IDXGIAdapter4* adpater;
			ID3D12Device2* device;
		};
		GpuHandle* gpuList = (GpuHandle*)alloca(sizeof(GpuHandle) * gpuCount);
		memset(gpuList, 0, sizeof(GpuHandle) * gpuCount);

		gpuCount = 0;
		for (UINT i = 0; m_dxgi_factory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND; ++i)
		{
			DXGI_ADAPTER_DESC3 adapterDesc;
			adapter->GetDesc3(&adapterDesc);

			if(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)
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

				if(FAILED(adapter->QueryInterface(IID_PPV_ARGS(&gpuList[gpuCount].adpater))))
				{
					continue;
				}

				ID3D12Device2* pDevice;
				D3D12CreateDevice(adapter, level, IID_PPV_ARGS(&pDevice));
				gpuList[gpuCount].device = pDevice;
				auto &gpu = gpuInfoList[gpuCount];
				gpuCount += 1;

				gpu.featureLevel = level;
				wcscpy_s(gpu.vendor_name, GPU_VENDOR_NAME_SIZE, adapterDesc.Description);
				gpu.vendor_id = adapterDesc.VendorId;
				gpu.device_id = adapterDesc.DeviceId;
				gpu.revision = adapterDesc.Revision;
				gpu.video_memory = adapterDesc.DedicatedVideoMemory;
				gpu.shared_memory = adapterDesc.SharedSystemMemory;

				pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &gpu.featureData, sizeof(gpu.featureData));
				pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS1, &gpu.featureData1, sizeof(gpu.featureData1));
				break;
			}
			adapter->Release();
		}

		// select the best one
		int gpuIndex = 0;
		for(int i=1; i<gpuCount; ++i)
		{
			auto &gpu1 = gpuInfoList[gpuIndex];
			auto &gpu2 = gpuInfoList[i];
			if(gpu2.featureData1.WaveOps != gpu1.featureData1.WaveOps)
			{
				if(gpu2.featureData1.WaveOps)
				{
					gpuIndex = i;
				}
				continue;
			}
			if(gpu2.featureLevel != gpu1.featureLevel)
			{
				if(gpu2.featureLevel > gpu1.featureLevel)
				{
					gpuIndex = i;
				}
				continue;
			}
			if(gpu2.video_memory > gpu1.video_memory)
			{
				gpuIndex = i;
			}
		}

		m_adapter = gpuList[gpuIndex].adpater;
		m_device = gpuList[gpuIndex].device;
		memcpy(&m_gpu_info, &gpuInfoList[gpuIndex], sizeof(GpuInfo));

		for(int i=0; i<gpuCount; ++i)
		{
			if(i != gpuIndex)
			{
				SAFE_RELEASE(gpuList[i].adpater);
				SAFE_RELEASE(gpuList[i].device);
			}
		}

		// check MSAA support
		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaQualityLevels;
		msaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		msaaQualityLevels.SampleCount = 4;
		msaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
		msaaQualityLevels.NumQualityLevels = 0;
		if(SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaaQualityLevels, sizeof(msaaQualityLevels))))
		{
			m_gpu_info.msaa4_quality_level = (int) msaaQualityLevels.NumQualityLevels;
		}
		msaaQualityLevels.SampleCount = 8;
		msaaQualityLevels.NumQualityLevels = 0;
		if(SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaaQualityLevels, sizeof(msaaQualityLevels))))
		{
			m_gpu_info.msaa8_quality_level = (int) msaaQualityLevels.NumQualityLevels;
		}

		LogInfo("Device: %ls", m_gpu_info.vendor_name);
		LogInfo("Vendor ID: %d", m_gpu_info.vendor_id);
		LogInfo("Revision ID: %d", m_gpu_info.revision);
		LogInfo("Video Memory: %zu", m_gpu_info.video_memory);
		LogInfo("MSAA 4x: %d", m_gpu_info.msaa4_quality_level);
		LogInfo("MSAA 8x: %d", m_gpu_info.msaa8_quality_level);

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
			D3D12_MESSAGE_SEVERITY Severities[] =
			{
				D3D12_MESSAGE_SEVERITY_INFO
			};

			// Suppress individual messages by their ID
			D3D12_MESSAGE_ID DenyIds[] = {
				D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
				D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
				D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
			};

			D3D12_INFO_QUEUE_FILTER infoFilter = {};
			// 		NewinfoFilterFilter.DenyList.NumCategories = _countof(Categories);
			// 		infoFilter.DenyList.pCategoryList = Categories;
			infoFilter.DenyList.NumSeverities = _countof(Severities);
			infoFilter.DenyList.pSeverityList = Severities;
			infoFilter.DenyList.NumIDs = _countof(DenyIds);
			infoFilter.DenyList.pIDList = DenyIds;

			CheckAndReturnFalse(m_device_info_queue->PushStorageFilter(&infoFilter));
		} // enable debug checks
		
		return true;
	}

	void RendererD3D12::ReportLiveObjects(const wchar_t* prompt)
	{
#ifdef _DEBUG
		if(prompt)
		{
			OutputDebugStringW(L"[");
			OutputDebugStringW(prompt);
			OutputDebugStringW(L"]\n");
		}
		ComPtr<IDXGIDebug1> dxgiDebug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
		{
			dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
		}
#endif
	}

	void RendererD3D12::WaitForComplete(uint64_t frameIndex)
	{
		if(m_frame_fence->fence->GetCompletedValue() < frameIndex)
		{
			m_frame_fence->fence->SetEventOnCompletion(frameIndex, m_frame_fence->wait_event);
			WaitForSingleObject(m_frame_fence->wait_event, INFINITE);
		}
	}

	static D3D12_COMMAND_LIST_TYPE D3D12CommandListTypeConverter[ECommandType::MaxCount] = {
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		D3D12_COMMAND_LIST_TYPE_COMPUTE,
		D3D12_COMMAND_LIST_TYPE_COPY
	};

	void RendererD3D12::EnableDebugLayer()
	{
		if(SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debug_layer))))
		{
			m_debug_layer->EnableDebugLayer();
			ID3D12Debug1* pDebug1 = NULL;
			if (SUCCEEDED(m_debug_layer->QueryInterface(IID_PPV_ARGS(&pDebug1))))
			{
				pDebug1->SetEnableGPUBasedValidation(TRUE);
				pDebug1->Release();
			}
		}
	}
} // namespace wyc
