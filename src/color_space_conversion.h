#pragma once
#include <cstdlib>
#include <iostream>

inline void YCbCr_to_RGB_Convert(unsigned char* src, int& height, int& width,
				 int& channels) {
    int Y, cb, cr;
    int r, g, b;
    int chan = 0;

    if (channels < 3) {
	std::cout << "IMAGE HAS LESS THAN 3 CHANNELS." << std::endl;
	exit(1);
    }

    for (int i = 0; i < height; i++) {
	for (int j = 0; j < width; j++) {
	    Y = (int) src[chan + 0];
	    cb = (int) src[chan + 1];
	    cr = (int) src[chan + 2];
	    r = Y + 1.402f * (cr - 128);
	    g = Y - 0.344136f * (cb - 128) - 0.714136f * (cr - 128);
	    b = Y + 1.772f * (cb - 128);
	    src[chan + 0] = (r < 0) ? 0 : (r < 255) ? r : 255;
	    src[chan + 1] = (g < 0) ? 0 : (g < 255) ? g : 255;
	    src[chan + 2] = (b < 0) ? 0 : (b < 255) ? b : 255;
	    chan += channels;
	}
    }
}

inline void RGB_to_YCbCr_Convert(unsigned char* src, int& height, int& width,
				 int& channels) {
    int Y, cb, cr;
    int r, g, b;
    int chan = 0;

    if (channels < 3) {
	std::cout << "IMAGE HAS LESS THAN 3 CHANNELS." << std::endl;
	exit(1);
    }

    for (int i = 0; i < height; i++) {
	for (int j = 0; j < width; j++) {
	    r = src[chan + 0];
	    g = src[chan + 1];
	    b = src[chan + 2];
	    Y = 0.299f * r + 0.587f * g + 0.114f * b;
	    cb = 128.0f - 0.168736f * r - 0.331264f * g + 0.5f * b;
	    cr = 128.0f + 0.5f * r - 0.418688f * g - 0.081312f * b;
	    src[chan + 0] = (Y < 0) ? 0 : (Y < 255) ? Y : 255;
	    src[chan + 1] = (cb < 0) ? 0 : (cb < 255) ? cb : 255;
	    src[chan + 2] = (cr < 0) ? 0 : (cr < 255) ? cr : 255;
	    chan += channels;
	}
    }
}
