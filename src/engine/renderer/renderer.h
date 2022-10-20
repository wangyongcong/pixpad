#pragma once
#include "engine.h"
#include "game/game_window.h"
#include "tinyimageformat/tinyimageformat_base.h"

namespace wyc
{
	enum ERendererName : uint16_t
	{
		UnknownRenderer,
		Direct3D12,
		Metal,
	};

	struct RenderStructHead
	{
		ERendererName type;
		size_t size;
	};

#define RENDERER_STRUCT(stype) RenderStructHead intrinsic_header
#define RENDERER_MAKE_STRUCT(ptr, rtype, stype) (ptr)->intrinsic_header = {(rtype), sizeof(stype)}
#define RENDERER_CHECK_STRUCT(ptr, rtype, stype) (((ptr)->intrinsic_header.type == (rtype)) && ((ptr)->intrinsic_header.size == sizeof(stype)))
#define RENDERER_STRUCT_TYPE(ptr) (ptr)->intrinsic_header.type

	struct RendererConfig
	{
		uint8_t frame_buffer_count;
		uint8_t sample_count;
		TinyImageFormat color_format;
		TinyImageFormat depth_stencil_format;
		bool enable_debug;
	};

#define GPU_VENDOR_NAME_SIZE 64

	struct GpuInfo
	{
		wchar_t vendor_name[GPU_VENDOR_NAME_SIZE];
		unsigned vendor_id;
		unsigned device_id;
		unsigned revision;
		size_t video_memory;
		size_t shared_memory;
		int msaa4_quality_level;
		int msaa8_quality_level;
	};

	enum EQueuePriority
	{
		QUEUE_PRIORITY_NORMAL = 0,
		QUEUE_PRIORITY_HIGH,
		MAX_QUEUE_PRIORITY
	};

	enum ECommandType
	{
		COMMAND_TYPE_DRAW,
		COMMAND_TYPE_COMPUTE,
		COMMAND_TYPE_COPY,
		// count of command list type
		COMMAND_TYPE_COUNT
	};

	enum EDeviceResourceType
	{
		VERTEX_BUFFER,
		INDEX_BUFFER,
		TEXTURE_1D,
		TEXTURE_2D,
	};

	struct DeviceFence
	{
		RENDERER_STRUCT(DeviceFence);
	};

	struct CommandQueueDesc
	{
		ECommandType type;
		EQueuePriority priority;
		unsigned node;
	};

	struct CommandQueue
	{
		RENDERER_STRUCT(CommandQueue);
		ECommandType type;
		EQueuePriority priority;
		unsigned node;
	};

	struct CommandPool
	{
		RENDERER_STRUCT(CommandPool);
		CommandQueue* queue;
	};

	struct CommandList
	{
		RENDERER_STRUCT(CommandList);
		CommandPool* pool;
	};

	struct DeviceResource
	{
		RENDERER_STRUCT(DeviceResource);
		EDeviceResourceType type;
		size_t size;
	};

	struct Shader
	{
		RENDERER_STRUCT(Shader);
	};

	class WYCAPI IRenderer
	{
	public:
		virtual ~IRenderer() = default;
		virtual bool initialize(IGameWindow* game_window, const RendererConfig& config) = 0;
		virtual void release() = 0;
		virtual void begin_frame() = 0;
		virtual void end_frame() = 0;
		virtual void present() = 0;
		virtual void resize() = 0;
		virtual void close() = 0;
		virtual const GpuInfo& get_gpu_info(int index = 0) = 0;

		virtual CommandQueue* create_queue(const CommandQueueDesc& desc) = 0;
		virtual void release_queue(CommandQueue* queue) = 0;
		virtual CommandPool* create_command_pool(CommandQueue* queue) = 0;
		virtual void release_command_pool(CommandPool* pool) = 0;
		virtual CommandList* create_command_list(CommandPool* pool) = 0;
		virtual void release_command_list(CommandList* cmd_list) = 0;
		virtual DeviceResource* create_resource(EDeviceResourceType type, size_t size) = 0;
		virtual void release_resource(DeviceResource* res) = 0;
		virtual void upload_resource(DeviceResource* res, void* data, size_t size) = 0;
		
		virtual DeviceFence* create_fence(unsigned value_count=1) = 0;
		virtual void release_fence(DeviceFence* fence) = 0;
		virtual bool is_fence_completed(DeviceFence* fence, unsigned index = 0) = 0;
		virtual void wait_for_fence(DeviceFence* fence, unsigned index=0) = 0;
	};
} // namespace wyc
