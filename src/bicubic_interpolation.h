#pragma once
#include <vector>

[[__nodiscard__]] inline double BiCubic_Weight_Coeff(const double x,
						     const double a) {
    if (x <= 1.0f) {
	return (1.0f - ((a + 3.0f) - (a + 2.0f) * x) * x * x);
    }
    if (x < 2.0f) {
	return ((-4.0f + (8.0f - (5.0f - x) * x) * x) * a);
    }
    return 0.0f;
}

inline void BiCubic_Coeff_4x4(const double y, const double x, std::vector<double> &coeff) {
    const double u = y - static_cast<int>(y) + 1;
    const double v = x - static_cast<int>(x) + 1;

    int k = 0;
    for (int i = 0; i < 4; i++) {
	const double du = (u > i) ? (u - i) : (i - u);
	for (int j = 0; j < 4; j++) {
	    constexpr double a = -0.5f;
	    const double dv = (v > j) ? (v - j) : (j - v);
	    coeff[k] =
		BiCubic_Weight_Coeff(du, a) * BiCubic_Weight_Coeff(dv, a);
	    k++;
	}
    }
}

inline void BiCubic_Filter(std::vector<unsigned char> &pix,
			   const std::vector<unsigned char> &src,
			   const int height,
			   const int width, const int channels, const double y_double,
			   const double x_double) {
    std::vector<double> coeff(16);
    std::vector<double> sum(8, 0);

    const int y0 = static_cast<int>(y_double) - 1;
    const int x0 = static_cast<int>(x_double) - 1;
    BiCubic_Coeff_4x4(y_double, x_double, coeff);

    int k = 0;
    int l = 0;
    for (int i = 0; i < 4; i++) {
	const int yf = ((y0 + i) < 0)	       ? 0
		 : ((y0 + i) < height) ? (y0 + i)
				       : (height - 1);
	for (int j = 0; j < 4; j++) {
	    const int xf = ((x0 + j) < 0)	  ? 0
		     : ((x0 + j) < width) ? (x0 + j)
					  : (width - 1);
	    l = (yf * width + xf) * channels;
	    for (int d = 0; d < channels; d++) {
		sum[d] += coeff[k] * static_cast<double>(src[l + d]);
	    }
	    k++;
	}
    }
    for (int i = 0; i < channels; i++) {
	pix[i] = static_cast<unsigned char>((sum[i] < 0.0f)	? 0
					    : (sum[i] < 255.0f) ? sum[i]
								: 255);
    }
}

inline void Resize_Image_BiCubic_Interpolation(std::vector<unsigned char> &src,
					       const int height, const int width,
    const int channels, const int resize_height, const int resize_width,
					       unsigned char *res) {
    const double ratio_height =
	static_cast<double>(resize_height) / static_cast<double>(height);
    const double ratio_width = static_cast<double>(resize_width) / static_cast<double>(width);
    std::vector<unsigned char> pix(100);

    int k = 0;
    for (int i = 0; i < resize_height; i++) {
	const double src_y = (static_cast<double>(i) + 0.5f) / ratio_height - 0.5f;
	for (int j = 0; j < resize_width; j++) {
	    const double src_x = (static_cast<double>(j) + 0.5f) / ratio_width - 0.5f;
	    BiCubic_Filter(pix, src, height, width, channels, src_y, src_x);
	    for (int d = 0; d < channels; d++) {
		res[k] = pix[d];
		k++;
	    }
	}
    }
}
