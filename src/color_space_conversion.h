#pragma once
#include <iostream>

inline void YCbCr_to_RGB_Convert(unsigned char* src, const int& height,
				 const int& width, const int& channels) {
    int chan {0};

    if (channels < 3) {
	std::cout << "IMAGE HAS LESS THAN 3 CHANNELS." << std::endl;
	exit(1);
    }

    for (int i = 0; i < height; i++) {
	for (int j = 0; j < width; j++) {
	    const int Y = src[chan + 0];
	    const int cb = src[chan + 1];
	    const int cr = src[chan + 2];
	    const int r = Y + 1.402 * (cr - 128);
	    const int g = Y - 0.344136 * (cb - 128) - 0.714136 * (cr - 128);
	    const int b = Y + 1.772 * (cb - 128);
	    src[chan + 0] = (r < 0) ? 0 : (r < 255) ? r : 255;
	    src[chan + 1] = (g < 0) ? 0 : (g < 255) ? g : 255;
	    src[chan + 2] = (b < 0) ? 0 : (b < 255) ? b : 255;
	    chan += channels;
	}
    }
}

inline void RGB_to_YCbCr_Convert(unsigned char* src, const int& height,
				 const int& width, const int& channels) {
    int chan = 0;

    if (channels < 3) {
	std::cout << "IMAGE HAS LESS THAN 3 CHANNELS." << std::endl;
	exit(1);
    }

    for (int i = 0; i < height; i++) {
	for (int j = 0; j < width; j++) {
	    const int r = src[chan + 0];
	    const int g = src[chan + 1];
	    const int b = src[chan + 2];
	    const int Y = 0.299 * r + 0.587 * g + 0.114 * b;
	    const int cb = 128.0 - 0.168736 * r - 0.331264 * g + 0.5 * b;
	    const int cr = 128.0 + 0.5 * r - 0.418688 * g - 0.081312 * b;
	    src[chan + 0] = (Y < 0) ? 0 : (Y < 255) ? Y : 255;
	    src[chan + 1] = (cb < 0) ? 0 : (cb < 255) ? cb : 255;
	    src[chan + 2] = (cr < 0) ? 0 : (cr < 255) ? cr : 255;
	    chan += channels;
	}
    }
}
