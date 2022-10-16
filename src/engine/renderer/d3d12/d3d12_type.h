#pragma once
#include "renderer/renderer.h"

namespace wyc
{
	struct GpuInfoD3D12 : GpuInfo
	{
		D3D_FEATURE_LEVEL feature_level = (D3D_FEATURE_LEVEL)0;
		D3D12_FEATURE_DATA_D3D12_OPTIONS feature_data = {};
		D3D12_FEATURE_DATA_D3D12_OPTIONS1 feature_data1 = {};
	};

	struct DeviceFenceD3D12 : DeviceFence
	{
		ID3D12Fence* fence;
		HANDLE wait_event;
		unsigned count;
		uint64_t value[1];
	};

	struct CommandQueueD3D12 : CommandQueue
	{
		ID3D12CommandQueue* queue;
	};

	struct CommandPoolD3D12 : CommandPool
	{
		ID3D12CommandAllocator* allocator;
	};

	struct CommandListD3D12 : CommandList
	{
		ID3D12GraphicsCommandList* command_list;
	};
	
	struct DeviceResourceD3D12 : DeviceResource
	{
		ID3D12Resource* resource;
		DeviceFenceD3D12* signal;
	};

}
