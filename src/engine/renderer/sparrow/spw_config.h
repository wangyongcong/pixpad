#pragma once

// max hardwares to be used, 0 means "as much as possible"
#define MAX_CORE_NUM 0

// hardward cache line size
#define CACHE_LINE_SIZE 64
#define CACHE_LINE_ALIGN alignas(CACHE_LINE_SIZE)

// primitive queue size
#define PRIMITIVE_QUEUE_SIZE 64

#define SPW_TILE_W 32
#define SPW_TILE_H 32

#ifdef SPW_USE_DOUBLE
using spw_float = double;
#else
using spw_float = float;
#endif
