#pragma once

#include <d3d12.h>
#include "renderer.h"


namespace wyc
{
	class RendererD3D12;

	struct D3D12GpuInfo : GpuInfo
	{
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL(0);
		D3D12_FEATURE_DATA_D3D12_OPTIONS featureData;
		D3D12_FEATURE_DATA_D3D12_OPTIONS1 featureData1;
	};

	enum class ERenderDeviceState : uint8_t
	{
		DEVICE_EMPTY = 0,
		DEVICE_INITIALIZED,
		DEVICE_CLOSED,
		DEVICE_RELEASED,
	};

	class RendererD3D12 : public IRenderer
	{
	public:
		RendererD3D12();
		~RendererD3D12() override;

		// Implement IRenderer
		bool initialize(IGameWindow* gameWindow, const RendererConfig& config) override;
		void release() override;
		void begin_frame() override;
		void end_frame() override;
		void present() override;
		void resize() override;
		void close() override;
		const GpuInfo& get_gpu_info(int index) override;
		// IRenderer

	protected:
		void EnableDebugLayer();
		bool CreateDevice(HWND hWnd, uint32_t width, uint32_t height, const RendererConfig& config);
		void ReportLiveObjects(const wchar_t* prompt=nullptr);
		void WaitForComplete(uint64_t frameIndex);

		ERenderDeviceState m_device_state;
		uint8_t m_frame_buffer_count;
		uint8_t m_frame_buffer_index;
		uint8_t m_sample_count;
		uint8_t m_sample_quality;
		uint64_t m_frame_index;
		unsigned m_descriptor_size[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
		DXGI_FORMAT m_color_format;
		DXGI_FORMAT m_depth_format;

		HWND m_window_handle;
		D3D12GpuInfo m_gpu_info;
		ID3D12Debug* m_debug_layer;
		IDXGIFactory6* m_dxgi_factory;
		IDXGIAdapter4* m_adapter;
		ID3D12Device2* m_device;
		ID3D12InfoQueue* m_device_info_queue;
		ID3D12DescriptorHeap* m_rtv_heap;
		ID3D12DescriptorHeap* m_dsv_heap;
		IDXGISwapChain4* m_swap_chain;
		ID3D12Resource** m_swap_chain_buffers;
		ID3D12Resource* m_depth_buffer;

		ID3D12CommandQueue* m_command_queue;
		ID3D12GraphicsCommandList* m_command_list;
		ID3D12CommandAllocator** m_command_allocators;

		struct DeviceFence
		{
			ID3D12Fence* fence;
			HANDLE wait_event;
			uint64_t value[1];
		};
		DeviceFence* m_frame_fence;
	};

} // namespace wyc
