#define SUCCESS 0
#define INVALID_ARGUMENT_COUNT -100

#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>


void rgb_to_hls(unsigned char red,
		unsigned char green,
		unsigned char blue,
		unsigned char* hue,
		unsigned char* lightness,
		unsigned char* saturation) {
	double _r, _g, _b;
	_r = red / 255.;
	_g = green / 255.;
	_b = blue / 255.;
	double _max, _min;
	_max = _r > (_g > _b ? _g : _b) ? _r : (_g > _b ? _g : _b);
	_min = _r < (_g < _b ? _g : _b) ? _r : (_g < _b ? _g : _b);
	double _h, _l, _s;
	_l = (_min + _max) / 2.0;
	if (_min == _max) {
		_h = 0.0;
		_s = 0.0;
	} else {
		if (_l < 0.5) {
			_s = (_max - _min) / (_max + _min);
		} else {
			_s = (_max - _min) / (2 - _max - _min);
		};
		double _rc = (_max - _r) / (_max - _min);
		double _gc = (_max - _g) / (_max - _min);
		double _bc = (_max - _b) / (_max - _min);
		if (_r == _max) {
			_h = _bc - _gc;
		} else if (_g == _max) {
			_h = 2.0 + _rc - _bc;
		} else {
			_h = 4.0 + _gc - _rc;
		};
		_h = fmod((_h / 6.0), 1.0);
		_h = _h < 0 ? (1. + _h) : _h;
	};
	*hue = _h * 255;
	*lightness = _l * 255;
	*saturation = _s * 255;
};


double _v(double m1, double m2, double hue) {
	hue = fmod(hue, 1.0);
	hue = hue < 0 ? (1. + hue) : hue;
	if (hue < (1 / 6.)) {
		return m1 + (m2 - m1) * hue * 6.;
	} else if (hue < .5) {
		return m2;
	} else if (hue < (2 / 3.)) {
		return m1 + (m2 - m1) * ((2 / 3.) - hue) * 6.;
	} else {
		return m1;
	};
};


void hls_to_rgb(unsigned char hue,
		unsigned char lightness,
		unsigned char saturation,
		unsigned char* red,
		unsigned char* green,
		unsigned char* blue) {
	double _h, _l, _s;
	_h = hue / 255.;
	_l = lightness / 255.;
	_s = saturation / 255.;
	double  _r, _g, _b, _m1, _m2;
	if (_s == 0.0) {
		_r = _l;
		_g = _l;
		_b = _l;
	} else {
		if (_l < 0.5) {
			_m2 = _l * (1. + _s);
		} else {
			_m2 = _l + _s - (_l * _s);
		}
		_m1 = 2.0 * _l - _m2;
		_r = _v(_m1, _m2, _h + (1 / 3.));
		_g = _v(_m1, _m2, _h);
		_b = _v(_m1, _m2, _h - (1 / 3.));
	};
	*red = _r * 255;
	*green = _g * 255;
	*blue = _b * 255;
};


unsigned char _alpha(double alpha,
		     unsigned char a,
		     unsigned char b) {
	double _a = a / 255.;
	double _b = b / 255.;
	return 255 * (alpha * _a + (1 - alpha) * _b);
};


void rgb_color_blend(unsigned char* pixel_1,
		     unsigned char* pixel_2,
		     double alpha,
		     unsigned char* result) {
	unsigned char _r1, _g1, _b1, _r2, _g2, _b2;
	_r1 = pixel_1[0]; _g1 = pixel_1[1]; _b1 = pixel_1[2];
	_r2 = pixel_2[0]; _g2 = pixel_2[1]; _b2 = pixel_2[2];

	unsigned char _h1, _l1, _s1, _h2, _l2, _s2;
	rgb_to_hls(_r1, _g1, _b1, &_h1, &_l1, &_s1);
	rgb_to_hls(_r2, _g2, _b2, &_h2, &_l2, &_s2);

	unsigned char _h, _l, _s;
	_h = _alpha(alpha, _h2, _h1);
	_l = _l1;
	_s = _alpha(alpha, _s2, _s1);
	
	unsigned char _r, _g, _b;
	hls_to_rgb(_h, _l, _s, &_r, &_g, &_b);
	if (_r < 0 || _r > 255 || _g < 0 || _g > 255 || _b < 0 || _b > 255) {
		printf("%d %d %d\n", _r, _g, _b);
	};
	result[0] = _r; result[1] = _g; result[2] = _b;
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
	stbi_write_bmp(_newImageFilename,
		       _image1Width,
		       _image1Height,
		       _image1CountComponents,
		       _newImage);

	/* Clean up memory */
	free(_newImage);

	/* Exit with success */
	return SUCCESS;
}
