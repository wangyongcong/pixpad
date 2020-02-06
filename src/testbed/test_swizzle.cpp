#include <stdlib.h>
#include "stb_image.h"
#include "stb_image_write.h"
#include "test.h"
#include "spw_tile.h"
#include "stb_log.h"
#include "metric.h"

using namespace wyc;

class CTestSwizzle : public CTest
{
public:
	virtual void run() override
	{
		const char *path = "assets/texture/lenna.png";
		int image_w, image_h, channels;
		auto data = stbi_load(path, &image_w, &image_h, &channels, 4);
		if(!data) {
			log_error("fail to load image: %s", path);
			return;
		}
		log_info("image size: %d x %d", image_w, image_h);

		size_t image_size = image_w * image_h * sizeof(uint32_t);
		uint32_t *image = (uint32_t*)aligned_alloc(64, image_size);
		memcpy(image, data, image_size);
		stbi_image_free(data);
		
		uint32_t texture_w = align_up(image_w, 64);
		uint32_t texture_h = align_up(image_h, 64);
		size_t texture_size = texture_w * texture_h * sizeof(uint32_t);
		log_info("aligned texture size: %d x %d", texture_w, texture_h);
		uint32_t *dst = (uint32_t*)aligned_alloc(64, texture_size);
		memset(dst, 0xcc, texture_size);
		uint32_t *src = (uint32_t*)aligned_alloc(64, image_size);
		memset(src, 0xcc, image_size);
		
		int ret = 0;
		{
			TIMER(swizzle);
			swizzle_32bpp_fast(dst, texture_w, image, image_w, image_h, image_w);
		}
		ret = stbi_write_png("swizzle-1.png", texture_w, texture_h, 4, dst, 0);
		if (!ret) {
			log_error("fail to save swizzle image: %d", ret);
		}

		{
			TIMER(linearize);
			linearize_32bpp_fast(src, image_w, image_h, image_w, dst, texture_w);
		}
		ret = stbi_write_png("swizzle-2.png", image_w, image_h, 4, src, 0);
		if (!ret) {
			log_error("fail to save linear image: %d", ret);
		}

		ret = memcmp(src, image, image_size);
		if(ret != 0) {
			log_error("image mismatch!");
		}
		
		for(uint32_t *iter=dst, *end=dst + texture_w * texture_h; iter < end; iter += 16) {
			float r = 0, g = 0, b = 0;
			for(auto i=0; i<16; ++i) {
				auto v = iter[i];
				r += (v & 0xFF) / 255.0f;
				g += ((v & 0xFF00) >> 8) / 255.0f;
				b += ((v & 0xFF0000) >> 16) / 255.0f;
			}
			r /= 16;
			g /= 16;
			b /= 16;
			uint32_t v = (int(r * 255) & 0xFF) + ((int(g * 255) & 0xFF) << 8) + ((int(b * 255) & 0xFF) << 16) + 0xFF000000;
			for(auto i=0; i<16; ++i) {
				iter[i] = v;
			}
		}
		linearize_32bpp_fast(src, image_w, image_h, image_w, dst, texture_w);
		ret = stbi_write_png("swizzle-3.png", image_w, image_h, 4, src, 0);
		if (!ret) {
			log_error("fail to save average image: %d", ret);
		}

		constexpr uint32_t n_runs = 1000;
		log_info("swizzle random subrects (%d runs)", n_runs);
		int error = 0;
		for (uint32_t run = 0; run < n_runs; run++) {
			memset(dst, 0xcc, texture_size);
			memset(src, 0xcc, image_size);

			// determine random target rectangle
			uint32_t x0 = rand() % image_w;
			uint32_t y0 = rand() % image_h;
			uint32_t x1 = rand() % image_w;
			uint32_t y1 = rand() % image_h;

			if (x0 > x1) std::swap(x0, x1);
			if (y0 > y1) std::swap(y0, y1);
			
			// determine random destination texture size that fits target rect
			uint32_t dw = x1;
			uint32_t dh = y1;
			if (dw < image_w) dw += rand() % (image_w - dw);
			if (dh < image_h) dh += rand() % (image_h - dh);

			// swizzle two ways
			wyc::swizzle_32bpp(dst, x0, y0, dw, dh, image, x1 - x0, y1 - y0, image_w);
			wyc::linearize_32bpp(src, x1 - x0, y1 - y0, image_w, dst, x0, y0, dw, dh);

			for(int y = 0; y < y1 - y0; ++y)
			{
				auto ref_line = image + y * image_w;
				auto src_line = src + y * image_w;
				if(memcmp(src_line, ref_line, (x1 - x0) * sizeof(uint32_t)) != 0)
				{
					log_error("mismatch: run=%d, line=%d, rect=(%d,%d)-(%d,%d), dw=%d dh=%d", run, y, x0, y0, x1, y1, dw, dh);
					error = 1;
					break;
				}
			}
			if(error)
				break;
		}
		if(!error) {
			log_info("all test pass");
		}
		
		aligned_free(image);
		aligned_free(dst);
		aligned_free(src);
	}
};

REGISTER_TEST(CTestSwizzle)
