extern "C" {
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
}

#include "ImageLoad.h"

namespace image {
	void resize_nearest_neighbor(unsigned char* src, int src_width, int src_height, unsigned char* dst, int dst_width, int dst_height, int channels) {
		for (int y = 0; y < dst_height; y++) {
			for (int x = 0; x < dst_width; x++) {
				// 将目标像素映射回源图像
				int src_x = (x * src_width) / dst_width;
				int src_y = (y * src_height) / dst_height;

				// 获取源图像中最近邻的像素
				for (int c = 0; c < channels; c++) {
					dst[(y * dst_width + x) * channels + c] = src[(src_y * src_width + src_x) * channels + c];
				}
			}
		}
	}

	stb_image FlatAndCroppingImg(const stb_image& SrcImg, int cropping_width, int cropping_height) {
		stb_image result(cropping_width, cropping_height, SrcImg.GetChannels());
		for (int x = 0; x < cropping_width; x++) {
			for (int y = 0; y < cropping_height; y++) {
				int src_x = x % SrcImg.GetWidth();
				int src_y = y % SrcImg.GetHeight();
				result.SetPixel(x, y, SrcImg.GetPixel(src_x, src_y));
			}
		}
		return result;
	}

	stb_image::stb_image(const std::string& FilePath, int req_comp) {//req_comp为请求的颜色通道数
		imageData = stbi_load(FilePath.c_str(), &width, &height, &channels, req_comp);
	}

	stb_image::stb_image(const std::vector<uint8_t>& FileData, int req_comp) {
		imageData = stbi_load_from_memory(FileData.data(), FileData.size(), &width, &height, &channels, req_comp);
	}

	stb_image::stb_image(const unsigned char* data, const int src_width, const int src_height, const int channel) {
		channels = channel;
		height = src_height;
		width = src_width;
		size_t size = src_width * src_height * channel;
		imageData = (unsigned char*)malloc(size);
		memcpy(imageData, data, size);
	}

	stb_image::stb_image(const int src_width, const int src_height, const int channel) {
		channels = channel;
		height = src_height;
		width = src_width;
		size_t size = width * height * channel;
		imageData = (unsigned char*)malloc(size);
		memset(imageData, 0, size);
	}

	stb_image::stb_image(const stb_image& src) {
		width = src.width;
		height = src.height;
		channels = src.channels;
		size_t size = width * height * channels;
		imageData = (unsigned char*)malloc(size);
		memcpy(imageData, src.imageData, size);
	}

	stb_image::stb_image(stb_image&& src) noexcept {
		width = src.width;
		height = src.height;
		channels = src.channels;
		imageData = src.imageData;
		src.imageData = 0;
	}

	stb_image& stb_image::operator=(stb_image&& src) noexcept {
		width = src.width;
		height = src.height;
		channels = src.channels;
		imageData = src.imageData;
		src.imageData = 0;
		return *this;
	}

	stb_image::~stb_image() {
		if (imageData != 0) {
			stbi_image_free(imageData);
			imageData = 0;
		}
	}

	bool stb_image::WritePng(const std::string& path) const {
		return static_cast<bool>(stbi_write_png(path.c_str(), width, height, channels, imageData, width * channels));
	}

	bool stb_image::WritePng(const char* path) const {
		return static_cast<bool>(stbi_write_png(path, width, height, channels, imageData, width * channels));
	}
}
