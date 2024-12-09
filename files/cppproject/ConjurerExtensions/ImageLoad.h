#ifndef IMAGELOAD_H_
#define IMAGELOAD_H_

#include <string>
#include <vector>
extern "C" {
	void stbi_set_flip_vertically_on_load(int flag_true_if_should_flip);
	int stbi_write_png(char const* filename, int x, int y, int comp, const void* data, int stride_bytes);
}

namespace image {
	class stb_image;

	void resize_nearest_neighbor(unsigned char* src, int src_width, int src_height, unsigned char* dst, int dst_width, int dst_height, int channels);
	stb_image FlatAndCroppingImg(const stb_image& SrcImg, int cropping_width, int cropping_height);

	struct rgba {
		union {
			struct {
				unsigned char r;
				unsigned char g;
				unsigned char b;
				unsigned char a;
			};
			unsigned char rgbaArray[4] = { 0,0,0,0 };
		};
		bool Eq(unsigned char _r, unsigned char _g, unsigned char _b = 0, unsigned char _a = 255) const {
			switch (channels) {
			case 1: {
				if (r != _r) {
					return false;
				}
				return true;
			}
			case 3: {
				if (r != _r) {
					return false;
				}
				if (g != _g) {
					return false;
				}
				if (b != _b) {
					return false;
				}
				return true;
			}
			case 4: {
				if (r != _r) {
					return false;
				}
				if (g != _g) {
					return false;
				}
				if (b != _b) {
					return false;
				}
				if (a != _a) {
					return false;
				}
				return true;
			}
			}
			return false;
		}
		bool Eq(unsigned long color) const {
			return color == GetHex();
		}
		unsigned long GetHex() const {//转十六进制
			long result = 0;
			for (int i = 0; i < channels - 1; i++) {
				result |= rgbaArray[i];
				result <<= 8;
			}
			result |= rgbaArray[channels - 1];
			return result;
		}
		int channels = 0;//可以用于确认是哪种通道的
	};

	class stb_image {
	public:
		stb_image(const std::string& FilePath, int req_comp = 0);//req_comp为请求的颜色通道数
		stb_image(const char* FilePath, int req_comp = 0) : stb_image(std::string(FilePath), req_comp) {}
		stb_image(const std::vector<uint8_t>& FileData, int req_comp = 0);

		stb_image(const unsigned char* data, const int src_width, const int src_height, const int channel);
		stb_image(const int src_width, const int src_height, const int channel);//透明图片
		/*
		如果 req_comp 设置为 0，stbi_load 将加载图像的所有颜色通道（例如，RGB 或 RGBA）。
		如果 req_comp 设置为 1，stbi_load 将加载图像的灰度通道（单通道）。
		如果 req_comp 设置为 3，stbi_load 将加载图像的 RGB 通道。
		如果 req_comp 设置为 4，stbi_load 将加载图像的 RGBA 通道。
		*/

		rgba GetPixel(int x, int y) const {//获取像素
			int pixelIndex = (y * width + x) * channels;//计算像素索引
			rgba result;
			result.channels = channels;//设置通道数
			for (int i = 0; i < channels; i++) {//设置像素的rgba颜色
				result.rgbaArray[i] = imageData[pixelIndex + i];
			}
			return result;
		}

		void SetPixel(int x, int y, rgba color) {//获取像素
			int pixelIndex = (y * width + x) * channels;//计算像素索引
			int colorChannel = color.channels;
			for (int i = 0; i < colorChannel; i++) {//设置像素的rgba颜色
				imageData[pixelIndex + i] = color.rgbaArray[i];
			}
		}

		unsigned char* GetImageData() const {
			return imageData;
		}
		int GetWidth() const {
			return width;
		}
		int GetHeight() const {
			return height;
		}
		int GetChannels() const {
			return channels;
		}
		bool GetIsNullptr() const {
			return !static_cast<bool>(imageData);
		}

		bool WritePng(std::string& path) const;
		bool WritePng(const char* path) const;

		virtual ~stb_image();
	private:
		unsigned char* imageData = nullptr;
		int width = 0;//宽
		int height = 0;//高
		int channels = 0;//通道
	};

}

#endif
