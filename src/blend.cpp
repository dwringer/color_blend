#define SUCCESS 0
#define INVALID_ARGUMENT_COUNT -100

#include <iostream>

#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>


void rgb_color_blend(unsigned char* pixel_1,
		     unsigned char* pixel_2,
		     double alpha,
		     unsigned char* result) {
	unsigned char _r1, _g1, _b1, _r2, _g2, _b2;
	_r1 = pixel_1[0]; _g1 = pixel_1[1]; _b1 = pixel_1[2];
	_r2 = pixel_2[0]; _g2 = pixel_2[1]; _b2 = pixel_2[2];
	
	result[0] = alpha * _r1 + (1 - alpha) * _r2;
	result[1] = alpha * _g1 + (1 - alpha) * _g2;
	result[2] = alpha * _b1 + (1 - alpha) * _b2;
};


int main(int argc, char** argv) {
	/* Process command line arguments */
	if ((argc != 4) and (argc != 5)) {
		return INVALID_ARGUMENT_COUNT;
	};
	char* _image1Filename = argv[1];
	char* _image2Filename = argv[2];
	char* _newImageFilename = argv[3];
	double _alpha;
	if (argc == 5) {
		_alpha = atof(argv[4]);
	} else {
		_alpha = 1.0;
	};

	/* Load first image */
	int _image1Width, _image1Height, _image1CountComponents;
	unsigned char* _image1 = stbi_load(_image1Filename,
					   &_image1Width,
					   &_image1Height,
					   &_image1CountComponents,
					   STBI_rgb);
	int _stride = _image1CountComponents * _image1Width;

	/* Load second image */
	int _image2Width, _image2Height, _image2CountComponents;
	unsigned char* _image2 = stbi_load(_image2Filename,
					   &_image2Width,
					   &_image2Height,
					   &_image2CountComponents,
					   STBI_rgb);

	/* Prepare new result image */
	unsigned char* _newImage = (unsigned char*)
		malloc(_image1Width *
		       _image1Height *
		       _image1CountComponents);

	/* Perform pixel-by-pixel conversion */
	for (int i=0; i < _image1Height; i++) {
		for (int j=0; j < _image1Width; j++) {
			unsigned char _image1Pixel[3];
			unsigned char _image2Pixel[3];
			int _baseOffset = ((i * _stride) +
					   (j * _image1CountComponents));
			for (int k = 0; k < _image1CountComponents; k++) {
				_image1Pixel[k] = _image1[_baseOffset + k];
				_image2Pixel[k] = _image2[_baseOffset + k];
			};

			/* Convert pixel */
			unsigned char _newPixel[3];
			//(unsigned char*)
			//				malloc(_image1CountComponents);
			rgb_color_blend(_image1Pixel,
					_image2Pixel,
					_alpha,
					_newPixel);
			
			for (int k = 0; k < _image1CountComponents; k++) {
				_newImage[_baseOffset + k] = _newPixel[k];
			};
		};
	};

	/* Write result image */
	stbi_write_png(_newImageFilename,
		       _image1Width,
		       _image1Height,
		       _image1CountComponents,
		       _newImage,
		       _stride);

	/* Clean up memory */
	free(_newImage);

	/* Exit with success */
	return SUCCESS;
}
