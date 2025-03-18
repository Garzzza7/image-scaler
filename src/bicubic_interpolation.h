#pragma once
#include <cstddef>
#include <vector>

[[__nodiscard__]] inline double BiCubic_Weight_Coeff(double x, double a) {
    if (x <= 1.0f) {
	return (1.0f - ((a + 3.0f) - (a + 2.0f) * x) * x * x);
    } else if (x < 2.0f) {
	return ((-4.0f + (8.0f - (5.0f - x) * x) * x) * a);
    }
    return 0.0f;
}

inline void BiCubic_Coeff_4x4(double y, double x, std::vector<double> &coeff) {
    const double a = -0.5f;

    double u = y - (int) y + 1;
    double v = x - (int) x + 1;

    int k = 0;
    for (int i = 0; i < 4; i++) {
	double du = (u > i) ? (u - i) : (i - u);
	for (int j = 0; j < 4; j++) {
	    double dv = (v > j) ? (v - j) : (j - v);
	    coeff[k] =
		BiCubic_Weight_Coeff(du, a) * BiCubic_Weight_Coeff(dv, a);
	    k++;
	}
    }
}

inline void BiCubic_Filter(std::vector<unsigned char> &pix,
			   std::vector<unsigned char> &src, int height,
			   int width, int channels, double y_double,
			   double x_double) {
    std::vector<double> coeff(16);
    std::vector<double> sum(8, 0);

    int y0 = (int) y_double - 1;
    int x0 = (int) x_double - 1;
    BiCubic_Coeff_4x4(y_double, x_double, coeff);

    int k = 0;
    int l = 0;
    for (int i = 0; i < 4; i++) {
	int yf = ((y0 + i) < 0)	       ? 0
		 : ((y0 + i) < height) ? (y0 + i)
				       : (height - 1);
	for (int j = 0; j < 4; j++) {
	    int xf = ((x0 + j) < 0)	  ? 0
		     : ((x0 + j) < width) ? (x0 + j)
					  : (width - 1);
	    l = (yf * width + xf) * channels;
	    for (int d = 0; d < channels; d++) {
		sum[d] += coeff[k] * (double) src[l + d];
	    }
	    k++;
	}
    }
    for (int i = 0; i < channels; i++) {
	pix[i] = (unsigned char) ((sum[i] < 0.0f)     ? 0
				  : (sum[i] < 255.0f) ? sum[i]
						      : 255);
    }
}

inline void Resize_Image_BiCubic_Interpolation(std::vector<unsigned char> &src,
					       int height, int width,
					       int channels, int resize_height,
					       int resize_width,
					       unsigned char *res) {
    double ratio_height = (double) ((double) resize_height / (double) height);
    double ratio_width = (double) ((double) resize_width / (double) width);
    std::vector<unsigned char> pix(100);

    int k = 0;
    for (int i = 0; i < resize_height; i++) {
	double src_y = ((double) i + 0.5f) / ratio_height - 0.5f;
	for (int j = 0; j < resize_width; j++) {
	    double src_x = ((double) j + 0.5f) / ratio_width - 0.5f;
	    BiCubic_Filter(pix, src, height, width, channels, src_y, src_x);
	    for (int d = 0; d < channels; d++) {
		res[k] = pix[d];
		k++;
	    }
	}
    }
}
