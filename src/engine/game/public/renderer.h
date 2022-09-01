#pragma once
#include "game_window.h"
#include "tinyimageformat/tinyimageformat_base.h"

namespace wyc
{
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

	enum EFenceStatus
	{
		COMPLETE,
		INCOMPLETE,
	};

	enum ECommandType
	{
		DRAW,
		COMPUTE,
		COPY,
		// count of command list type
		MAX_COUNT
	};

	class GAME_FRAMEWORK_API IRenderer
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
	};
} // namespace wyc
