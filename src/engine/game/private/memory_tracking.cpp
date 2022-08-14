/*
 * Copyright (c) 2018-2021 The Forge Interactive Inc.
 *
 * This file is part of The-Forge
 * (see https://github.com/ConfettiFX/The-Forge).
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
*/

#include <stdlib.h>
#include <memory.h>

#include "platform.h"

#define MEM_MAX(a, b) ((a) > (b) ? (a) : (b))

#define ALIGN_TO(size, alignment) (((size) + (alignment) - 1) & ~((alignment) - 1))
#define MIN_ALLOC_ALIGNMENT MEM_MAX(VECTORMATH_MIN_ALIGN, PLATFORM_MIN_MALLOC_ALIGNMENT)

#if defined(USE_MEMORY_TRACKING)

#define _CRT_SECURE_NO_WARNINGS 1

// Just include the cpp here so we don't have to add it to the all projects
#include "MemoryManager/mmgr.h"

void* tf_memalign_internal(size_t align, size_t size, const char *f, int l, const char *sf)
{
	void* pMemAlign = mmgrAllocator(f, l, sf, m_alloc_malloc, align, size);

	// Return handle to allocated memory.
	return pMemAlign;
}

void* tf_calloc_memalign_internal(size_t count, size_t align, size_t size, const char *f, int l, const char *sf)
{
	size = ALIGN_TO(size, align);

	void* pMemAlign = mmgrAllocator(f, l, sf, m_alloc_calloc, align, size * count);

	// Return handle to allocated memory.
	return pMemAlign;
}

void* tf_malloc_internal(size_t size, const char* f, int l, const char* sf)
{
	return tf_memalign_internal(MIN_ALLOC_ALIGNMENT, size, f, l, sf);
}

void* tf_calloc_internal(size_t count, size_t size, const char* f, int l, const char* sf)
{
	return tf_calloc_memalign_internal(count, MIN_ALLOC_ALIGNMENT, size, f, l, sf);
}

void* tf_realloc_internal(void* ptr, size_t size, const char *f, int l, const char *sf) 
{
	void* pRealloc = mmgrReallocator(f, l, sf, m_alloc_realloc, size, ptr);

	// Return handle to reallocated memory.
	return pRealloc;
}

void tf_free_internal(void* ptr, const char *f, int l, const char *sf)
{
	mmgrDeallocator(f, l, sf, m_alloc_free, ptr);
}

#else // defined(USE_MEMORY_TRACKING)

bool MemAllocInit()
{
	// No op but this is where you would initialize your memory allocator and bookkeeping data in a real world scenario
	return true;
}

void MemAllocExit()
{
	// Return all allocated memory to the OS. Analyze memory usage, dump memory leaks, ...
}

void* tf_malloc(size_t size)
{
#ifdef _MSC_VER
	void* ptr = _aligned_malloc(size, MIN_ALLOC_ALIGNMENT);
#else
	void* ptr = malloc(size);
#endif

	return ptr;
}

void* tf_calloc(size_t count, size_t size)
{
#ifdef _MSC_VER
	size_t sz = count * size;
	void* ptr = tf_malloc(sz);
	memset(ptr, 0, sz); //-V575
#else
	void* ptr = calloc(count, size);
#endif

	return ptr;
}

void* tf_memalign(size_t alignment, size_t size)
{
#ifdef _MSC_VER
	void* ptr = _aligned_malloc(size, alignment);
#else
	void* ptr;
	alignment = alignment > sizeof(void*) ? alignment : sizeof(void*);
	if (posix_memalign(&ptr, alignment, size))
	{
		ptr = NULL;
	}
#endif

	return ptr;
}

void* tf_calloc_memalign(size_t count, size_t alignment, size_t size)
{
	size_t alignedArrayElementSize = ALIGN_TO(size, alignment);
	size_t totalBytes = count * alignedArrayElementSize;

	void* ptr = tf_memalign(alignment, totalBytes);

	memset(ptr, 0, totalBytes); //-V575
	return ptr;
}

void* tf_realloc(void* ptr, size_t size)
{
#ifdef _MSC_VER
	void* reallocPtr = _aligned_realloc(ptr, size, MIN_ALLOC_ALIGNMENT);
#else
	void* reallocPtr = realloc(ptr, size);
#endif

	return reallocPtr;
}

void tf_free(void* ptr)
{
#ifdef _MSC_VER
	_aligned_free(ptr);
#else
	free(ptr);
#endif
}

void* tf_malloc_internal(size_t size, const char *f, int l, const char *sf) { return tf_malloc(size); }

void* tf_memalign_internal(size_t align, size_t size, const char *f, int l, const char *sf) { return tf_memalign(align, size); }

void* tf_calloc_internal(size_t count, size_t size, const char *f, int l, const char *sf) { return tf_calloc(count, size); }

void* tf_calloc_memalign_internal(size_t count, size_t align, size_t size, const char *f, int l, const char *sf) { return tf_calloc_memalign(count, align, size); }

void* tf_realloc_internal(void* ptr, size_t size, const char *f, int l, const char *sf) { return tf_realloc(ptr, size); }

void tf_free_internal(void* ptr, const char *f, int l, const char *sf) { tf_free(ptr); }

#endif // defined(USE_MEMORY_TRACKING)
