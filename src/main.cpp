#define STB_IMAGE_IMPLEMENTATION       // important for stb_image
#define STB_IMAGE_WRITE_IMPLEMENTATION // important for stb_image_write
#include <unistd.h>		       // sleep

#include <chrono>     // timer
#include <cstdlib>    // malloc , free
#include <functional> // ref
#include <iomanip>    // timer
#include <iostream>   // console output
#include <ostream>
#include <thread> // threads
#include <vector> // vector container

#include "bicubic_interpolation.h"
#include "color_space_conversion.h"
#include "stb_image.h"
#include "stb_image_write.h"
#include "super_resolution_cnn.h"
// #include "sys/sysinfo.h"
// #include "sys/types.h"

#define CONV1_FILTERS 64
#define CONV2_FILTERS 32

// struct sysinfo memInfo;

bool done = false;
void notify() {
    std::cout.flush();
    while (!done) {
	for (int i = 0; i < 3; i++) {
	    std::cout << ".";
	    std::cout.flush();
	    sleep(1);
	}
	std::cout << "\b\b\b   \b\b\b";
    }
    std::cout << std::endl;
}

int run(int &argc, char **argv) {
    //   sysinfo(&memInfo);
    //   long long physMemUsed = memInfo.totalram - memInfo.freeram;
    //   physMemUsed *= memInfo.mem_unit;
    std::cout << std::setprecision(15) << std::endl;
    int height = 0;
    int width = 0;
    int channels = 0;
    int new_height = 0;
    int new_width = 0;

    constexpr int block_size = 256;
    constexpr int overlap = 16;
    constexpr double pcnn = 0.707107;
    constexpr int scaling_factor = 2;

    constexpr int neural_net_size =
	(block_size + 2 * (overlap + 6)) * (block_size + 2 * (overlap + 6));

    // begin the timer
    /*   std::chrono::time_point<*/
    /*std::chrono::system_clock,*/
    /*std::chrono::duration<long, std::ratio<1, 1000000000>>>*/
    const auto start_timer = std::chrono::high_resolution_clock::now();

    // store input arguments
    const char *source_image_name = argv[1];
    const char *output_image_name = argv[2];

    std::cout << "Loading: " << source_image_name << std::endl;

    // load the input image using stb_image
    stbi_uc *input_image = nullptr;
    if (!((input_image = stbi_load(source_image_name, &width, &height,
				   &channels, STBI_rgb_alpha)))) {
	std::cerr << "Could not read the image: " << source_image_name
		  << std::endl;
	exit(EXIT_FAILURE);
    }

    // assign dimensions of the scaled image
    new_height = height * scaling_factor;
    new_width = width * scaling_factor;

    std::cout << "From: " << width << " by " << height << std::endl;

    // move image to a vector of chars
    std::vector<unsigned char> image_buffer(height * width * channels);
    int stb_iter = 0;
    int image_iter = 0;
    for (int y = 0; y < height; y++) {
	for (int x = 0; x < width; x++) {
	    for (int d = 0; d < channels; d++) {
		image_buffer[image_iter + d] = input_image[stb_iter + d];
	    }
	    stb_iter += STBI_rgb_alpha;
	    image_iter += channels;
	}
    }

    std::cout << "To: " << new_width << " by " << new_height << std::endl;

    // cannot use std::vector here , because of stb_image
    unsigned char *new_image_buffer{nullptr};
    if (!((new_image_buffer = static_cast<unsigned char *>(malloc(
	       new_height * new_width * channels * sizeof(unsigned char)))))) {
	const auto mem =
	    new_height * new_width * channels * sizeof(unsigned char);
	std::cerr << "Not enough memory to allocate for the new image.\n"
		  << "Could not allocate "
		  << ((mem >= 1000000) ? mem / 1000000 : mem)
		  << ((mem >= 1000000) ? " mb" : " bytes") << " of memory\n"
		  << "Aborting.\n"
		  << std::endl;
	done = true;
	exit(EXIT_FAILURE);
    }
    const auto mem = new_height * new_width * channels * sizeof(unsigned char);

    std::cout << "Allocated " << ((mem >= 1000000) ? mem / 1000000 : mem)
	      << ((mem >= 1000000) ? " mb" : " bytes") << " of memory"
	      << std::endl;

    // preprocessing
    // stretch the image to the desired resolution
    Resize_Image_BiCubic_Interpolation(image_buffer, height, width, channels,
				       new_height, new_width, new_image_buffer);

    // convert from RGB to YCBCR color space
    RGB_to_YCbCr_Convert(new_image_buffer, new_height, new_width, channels);

    std::vector<unsigned char> cnn_block(neural_net_size);
    std::vector<double> cnn_data(neural_net_size * CONV2_FILTERS);

    SRCNN_Block(new_image_buffer, cnn_block, cnn_data, new_height, new_width,
		channels, block_size, overlap, pcnn);

    // convert from YCBCR color space to RGB
    YCbCr_to_RGB_Convert(new_image_buffer, new_height, new_width, channels);

    if (!(stbi_write_png(output_image_name, new_width, new_height, channels,
			 new_image_buffer, new_width * channels))) {
	std::cerr << "Could not write image:" << output_image_name << std::endl;
	exit(EXIT_FAILURE);
    } else {
	std::cout << "DONE" << std::endl;
    }

    // release the input image buffer
    stbi_image_free(input_image);
    // release the output image buffer
    free(new_image_buffer);

    std::cout << "Saved as: " << output_image_name << std::endl;
    done = true;

    // stop the timer
    /*   std::chrono::time_point<*/
    /*std::chrono::system_clock,*/
    /*std::chrono::duration<long, std::ratio<1, 1000000000>>>*/
    const auto finish_timer = std::chrono::high_resolution_clock::now();
    const double timing =
	std::chrono::duration_cast<std::chrono::duration<double>>(finish_timer -
								  start_timer)
	    .count();

    std::cout << std::setprecision(2) << std::fixed;
    std::cout << "Execution time: " << timing << " seconds.\nApproximately "
	      << timing / static_cast<double>(60) << " minutes." << std::endl;
    return 0;
}

int main(int argc, char **argv) {
    if (argc == 2 &&
	(std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help")) {
	std::cout
	    << "This is an image super-resolution program.\nIn order to "
	       "use it you have to provide an input and an output image.\nThe "
	       "patter is as follows:\n./image-scaler input.png output.png"
	    << std::endl;
	return 1;
    }
    if (argc != 3) {
	std::cout << "Calling the program should follow such a pattern\n"
		     "./image-scaler input.png output.png"
		  << std::endl;
	return 1;
    }
    std::cout << "\nPreparing your image." << std::endl;
    const unsigned int cnt_threads = std::thread::hardware_concurrency();
    if (cnt_threads > 2) {
	std::cout << "You have " << cnt_threads
		  << " threads. Running in a multi-threaded mode." << std::endl;

	std::thread t1(run, std::ref(argc), std::ref(argv));
	std::thread t2(notify);

	t2.join();
	t1.join();
    } else {
	std::cout << "You have " << cnt_threads
		  << "threads. Running in a single-threaded mode." << std::endl;
	return run(argc, argv);
    }
    return 0;
}
