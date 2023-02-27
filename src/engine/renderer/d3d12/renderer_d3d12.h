#pragma once
#include <d3d12.h>
#include "renderer/renderer.h"
#include "common/common_macros.h"


namespace wyc
{
	struct DeviceFenceD3D12;
	struct CommandQueueD3D12;
	struct CommandPoolD3D12;
	struct CommandListD3D12;
	struct GpuInfoD3D12;

	class ResourceLoaderD3D12;

	enum class ERenderDeviceState : uint8_t
	{
		DEVICE_EMPTY = 0,
		DEVICE_INITIALIZED,
		DEVICE_CLOSED,
		DEVICE_RELEASED,
	};

	class RendererD3D12 : public IRenderer
	{
		DISALLOW_COPY_MOVE_AND_ASSIGN(RendererD3D12);
	public:
		RendererD3D12();
		~RendererD3D12() override;

		// --------------------------------------------------------------------
		// Implement IRenderer
		// --------------------------------------------------------------------

		bool initialize(IGameWindow* game_window, const RendererConfig& config) override;
		void release() override;
		void begin_frame() override;
		void end_frame() override;
		void present() override;
		void resize() override;
		void close() override;
		const GpuInfo& get_gpu_info(int index) override;
		CommandQueue* create_queue(const CommandQueueDesc& desc) override;
		void release_queue(CommandQueue* queue) override;
		CommandPool* create_command_pool(CommandQueue* queue) override;
		void release_command_pool(CommandPool* pool) override;
		CommandList* create_command_list(CommandPool* pool) override;
		void release_command_list(CommandList* cmd_list) override;
		DeviceResource* create_resource(EDeviceResourceType type, size_t size) override;
		void release_resource(DeviceResource* res) override;
		void upload_resource(DeviceResource* res, size_t offset, void* data, size_t size) override;
		DeviceFence* create_fence(unsigned value_count=1) override;
		void release_fence(DeviceFence* fence) override;
		bool is_fence_completed(DeviceFence* fence, unsigned index = 0) override;
		void wait_for_fence(DeviceFence* fence, unsigned index=0) override;
		
		// --------------------------------------------------------------------
		// End of IRenderer
		// --------------------------------------------------------------------

		ID3D12Device2* GetDevice() const
		{
			return m_device;
		}
	protected:
		void enable_debug_layer();
		bool create_device(HWND hwnd, uint32_t width, uint32_t height, const RendererConfig& config);
		void release_device();
		void report_live_objects(const wchar_t* prompt=nullptr);
		void wait_for_complete(uint64_t frame_index);

		ERenderDeviceState m_device_state;
		uint8_t m_frame_buffer_count;
		uint8_t m_frame_buffer_index;
		uint8_t m_sample_count;
		uint8_t m_sample_quality;
		uint64_t m_frame_index;
		unsigned m_descriptor_size[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
		DXGI_FORMAT m_color_format;
		DXGI_FORMAT m_depth_format;
		D3D12_VIEWPORT m_viewport;

		HWND m_window_handle;
		GpuInfoD3D12* m_gpu_info;
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

		// frame graphic command queue
		CommandQueueD3D12* m_command_queue;
		CommandPoolD3D12** m_command_pools;
		CommandListD3D12** m_command_lists;
		DeviceFenceD3D12* m_frame_fence;

		ResourceLoaderD3D12* m_resource_loader;
	};
} // namespace wyc
