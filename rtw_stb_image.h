#ifndef RTW_STB_IMAGE_H
#define RTW_STB_IMAGE_H

// Disable strict warnings for this header from the Microsoft Visual C++ compiler.
#ifdef _MSC_VER
#pragma warning(push, 0)
#endif

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG

#include "./external/stb_image.h"

#include <iostream>
#include <cstdlib>

class rtw_image
{
public:
	rtw_image() {}

	rtw_image(const char *image_filename)
	{
		// Loads image data from the specified file. If the RTW_IMAGES environment variable is
		// defined, looks only in that directory for the image file. If the image was not found,
		// searches for the specified image file first from the current directory, then in the
		// images/ subdirectory, then the _parent's_ images/ subdirectory, and then _that_
		// parent, on so on, for six levels up. If the image was not loaded successfully,
		// width() and height() will return 0.

		auto filename = std::string(image_filename);
		auto imagedir = getenv("RTW_IMAGES");

		// hunt for the image file in some likely location
		if (imagedir && load(std::string(imagedir) + '/' + filename))
			return;
		if (load(filename))
			return;
		if (load("images/" + filename))
			return;
		if (load("../images/" + filename))
			return;
		if (load("../../images/" + filename))
			return;
		if (load("../../../images/" + filename))
			return;
		if (load("../../../../images/" + filename))
			return;
		if (load("../../../../../images/" + filename))
			return;
		if (load("../../../../../../images/" + filename))
			return;

		std::cerr << "ERROR: Could not load image file '" << image_filename << "'.\n";
	}

	~rtw_image()
	{
		delete[] bdata;
		STBI_FREE(fdata);
	}

	bool load(const std::string &filename)
	{
		// loads the linear (gamma = 1) image data from the given file name. return true if the load succeeded.
		// the resulting data buffer contains the there [0.0,1.0] floating-point values for the first pixel(red,then green,then blue).
		// pixels are contiguous, going left to right for the width of the image, followed by the next row below, for the full height of the image.

		auto n = bytes_per_pixel;
		fdata = stbi_loadf(filename.c_str(), &image_width, &image_height, &n, bytes_per_pixel);
		if (fdata == nullptr)
			return false;

		bytes_per_scanline = image_width * bytes_per_pixel;

		convert_to_bytes();

		return true;
	}

	int width() const { return (fdata == nullptr) ? 0 : image_width; }
	int height() const { return (fdata == nullptr) ? 0 : image_height; }


	const unsigned char *pixel_data(int x, int y) const
	{
		// return the address of the three RGB bytes of the pixel at x,y
		// If there is no image data,return magenta
		static unsigned char magenta[] = {255, 0, 255}; // ��ɫ
		if (bdata == nullptr)
			return magenta;
		x = clamp(x, 0, image_width);
		y = clamp(y, 0, image_height);

		return bdata + y * bytes_per_scanline + x * bytes_per_pixel;
	}

private:
	const int bytes_per_pixel = 3; // r��g��b ��ռ��һ���ֽڣ��������ͨ��
	float *fdata = nullptr;			// ��ͼ�������Ը�������ʽ�洢��fdata������
	unsigned char *bdata = nullptr; // linear 8-bit pixel data
	int image_width = 0;			// loaded image width
	int image_height = 0;			// loaded image height
	int bytes_per_scanline = 0;

	static int clamp(int x, int low, int high)
	{
		if (x < low)
			return low;
		if (x < high)
			return x;
		return high - 1;
	}

	static unsigned char float_to_byte(float value)
	{
		if (value <= 0.0)
			return 0;
		if (value >= 1.0)
			return 255;
		return static_cast<unsigned char>(256.0 * value); // ��float����ת��Ϊ8-bits�ַ�char
	}

	void convert_to_bytes()
	{
		// convert the linear floating point pixel data to bytes,storing the resulting byte data in the 'bdata' member
		int total_bytes = image_width * image_height * bytes_per_pixel;
		// ����һ����̬ unsigned char ������
		bdata = new unsigned char[total_bytes];

		// iterate through all pixel components, converting from [0.0,1.0] float values to unsigned [0,255] byte values
		auto *bptr = bdata;
		auto *fptr = fdata;
		for (auto i = 0; i < total_bytes; i++, fptr++, bptr++)
		{
			*bptr = float_to_byte(*fptr);
		}
	}
};

// restore MSVC compiler warnings

#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif