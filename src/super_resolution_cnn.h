#pragma once
#include <vector>

#include "pretrained_weights_biases.h"

#define SCALING_FACTOR 2

inline void Convolution_99x11x55_offset(std::vector<unsigned char>& src,
					std::vector<double>& cnndst,
					const int& height, const int& width,
					const int& offset,
					const double& partcnn) {
    std::vector<double> temp_buffer(CONV_LAYER1_FILTERS);

    int rowf, colf;
    int li;
    int ld;

    for (int row = offset - 2; row < height + 2 - offset; row++) {
	for (int col = offset - 2; col < width + 2 - offset; col++) {
	    for (int k = 0; k < CONV_LAYER1_FILTERS; k++) {
		temp_buffer[k] = 0.0f;
		for (int i = 0; i < 9; i++) {
		    rowf = row - 4 + i;
		    for (int j = 0; j < 9; j++) {
			colf = col - 4 + j;
			li = rowf * width + colf;
			temp_buffer[k] += weights_conv1_data[k][i][j] * src[li];
		    }
		}
		temp_buffer[k] += biases_conv1[k];
		temp_buffer[k] =
		    (temp_buffer[k] < 0.0f) ? 0.0f : temp_buffer[k];
	    }
	    for (int k = 0; k < CONV_LAYER2_FILTERS; k++) {
		double result = 0.0f;
		for (int i = 0; i < CONV_LAYER1_FILTERS; i++) {
		    result += temp_buffer[i] * weights_conv2_data[k][i];
		}
		result += biases_conv2[k];
		result = (result < 0.0f) ? 0.0f : result;
		ld = (k * height + row) * width + col;
		cnndst[ld] = result;
	    }
	}
    }

    for (int row = offset; row < height - offset; row++) {
	for (int col = offset; col < width - offset; col++) {
	    double t = 0.0f;
	    for (int i = 0; i < CONV_LAYER2_FILTERS; i++) {
		double tp = 0.0f;
		for (int m = 0; m < 5; m++) {
		    rowf = row - 2 + m;
		    for (int n = 0; n < 5; n++) {
			colf = col - 2 + n;
			ld = (i * height + rowf) * width + colf;
			tp += weights_conv3_data[i][m][n] * cnndst[ld];
		    }
		}
		t += tp;
	    }
	    li = row * width + col;
	    t += biases_conv3;
	    t *= partcnn;
	    t += ((1.0 - partcnn) * static_cast<double>(src[li]));
	    t += 0.5;
	    t = (t < 0.0) ? 0.0 : (t < 255.0) ? t : 255.0;
	    src[li] = static_cast<unsigned char>(t);
	}
    }
}

inline void SRCNN_Block(unsigned char* src, std::vector<unsigned char>& block,
			std::vector<double>& cnndst, const int& height,
			const int& width,
			const int& channels, const int& block_size,
			const int& overlap, const double& partcnn) {
    int xf, yf;
    int ki {0};
    int k  {0};
    constexpr int offset = 6;

    const int bs2 = block_size;
    const int bm = (height + bs2 - 1) / bs2;
    const int bn = (width + bs2 - 1) / bs2;

    const int bb = block_size + 2 * (offset + overlap);

    for (int i = 0; i < bm; i++) {
	const int y0 = i * bs2 - overlap;
	for (int j = 0; j < bn; j++) {
	    const int x0 = j * bs2 - overlap;
	    k = 0;
	    for (int y = 0; y < bb; y++) {
		yf = y0 + y - offset;
		yf = (yf < 0) ? 0 : (yf < height) ? yf : (height - 1);
		for (int x = 0; x < bb; x++) {
		    xf = x0 + x - offset;
		    xf = (xf < 0) ? 0 : (xf < width) ? xf : (width - 1);
		    ki = (yf * width + xf) * channels;
		    block[k] = src[ki];
		    k++;
		}
	    }

	    Convolution_99x11x55_offset(block, cnndst, bb, bb, offset, partcnn);

	    for (int y = 0; y < block_size; y++) {
		yf = y0 + y + overlap;
		if (yf < height) {
		    for (int x = 0; x < block_size; x++) {
			xf = x0 + x + overlap;
			if (xf < width) {
			    ki = (yf * width + xf) * channels;
			    k = ((y + overlap + offset) * bb + x + offset +
				 overlap);
			    src[ki] = block[k];
			}
		    }
		}
	    }
	}
    }
}
