#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <chrono>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <thread>
#include <unistd.h>
#include <vector>

#include "bicubic_interpolation.h"
#include "color_space_conversion.h"
#include "stb_image.h"
#include "stb_image_write.h"
#include "super_resolution_cnn.h"

#define CONV1_FILTERS 64
#define CONV2_FILTERS 32

bool done = false;

void
notify() {
    std::cout.flush();
    while (not done) {
        for (int i = 0; i < 3; i++) {
            std::cout << ".";
            std::cout.flush();
            sleep(1);
        }
        std::cout << "\b\b\b   \b\b\b";
    }
    std::cout << '\n';
}

int
run(int &argc, char **argv) {
    int height                    = 0;
    int width                     = 0;
    int channels                  = 0;
    int new_height                = 0;
    int new_width                 = 0;

    constexpr int block_size      = 256;
    constexpr int overlap         = 16;
    constexpr double pcnn         = 0.707107;
    constexpr int scaling_factor  = 2;

    constexpr int neural_net_size = (block_size + 2 * (overlap + 6)) * (block_size + 2 * (overlap + 6));

    const auto start_timer        = std::chrono::high_resolution_clock::now();

    const char *source_image_name = argv[1];
    const char *output_image_name = argv[2];

    std::cout << "Loading: " << source_image_name << '\n';

    stbi_uc *input_image = nullptr;
    if (not((input_image = stbi_load(source_image_name, &width, &height, &channels, STBI_rgb_alpha)))) {
        std::cerr << "Could not read the image: " << source_image_name << '\n';
        exit(EXIT_FAILURE);
    }

    new_height = height * scaling_factor;
    new_width  = width * scaling_factor;

    std::cout << "From: " << width << " by " << height << '\n';

    std::vector<unsigned char> image_buffer(height * width * channels);
    int stb_iter   = 0;
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

    std::cout << "To: " << new_width << " by " << new_height << '\n';

    unsigned char *new_image_buffer{nullptr};
    if (not((new_image_buffer
             = static_cast<unsigned char *>(malloc(new_height * new_width * channels * sizeof(unsigned char)))))) {
        const auto mem = new_height * new_width * channels * sizeof(unsigned char);
        std::cerr << "Not enough memory to allocate for the new image.\n"
                  << "Could not allocate " << ((mem >= 1000000) ? mem / 1000000 : mem) << ((mem >= 1000000) ? " mb" : " bytes")
                  << " of memory\n"
                  << "Aborting.\n"
                  << '\n';
        done = true;
        exit(EXIT_FAILURE);
    }
    const auto mem = new_height * new_width * channels * sizeof(unsigned char);

    std::cout << "Allocated " << ((mem >= 1000000) ? mem / 1000000 : mem) << ((mem >= 1000000) ? " mb" : " bytes") << " of memory"
              << '\n';

    Resize_Image_BiCubic_Interpolation(image_buffer, height, width, channels, new_height, new_width, new_image_buffer);

    RGB_to_YCbCr_Convert(new_image_buffer, new_height, new_width, channels);

    std::vector<unsigned char> cnn_block(neural_net_size);
    std::vector<double> cnn_data(neural_net_size * CONV2_FILTERS);

    SRCNN_Block(new_image_buffer, cnn_block, cnn_data, new_height, new_width, channels, block_size, overlap, pcnn);

    YCbCr_to_RGB_Convert(new_image_buffer, new_height, new_width, channels);

    if (not(stbi_write_png(output_image_name, new_width, new_height, channels, new_image_buffer, new_width * channels))) {
        std::cerr << "Could not write image:" << output_image_name << '\n';
        exit(EXIT_FAILURE);
    } else {
        std::cout << "DONE" << '\n';
    }

    stbi_image_free(input_image);
    free(new_image_buffer);

    std::cout << "Saved as: " << output_image_name << '\n';
    done                    = true;

    const auto finish_timer = std::chrono::high_resolution_clock::now();
    const double timing     = std::chrono::duration_cast<std::chrono::duration<double>>(finish_timer - start_timer).count();

    std::cout << std::setprecision(2) << std::fixed;
    std::cout << "Execution time: " << timing << " seconds.\nApproximately " << timing / static_cast<double>(60) << " minutes."
              << '\n';
    return 0;
}

int
main(int argc, char **argv) {
    std::cout << std::setprecision(15);
    if (argc == 2 && (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help")) {
        std::cout << "This is an image super-resolution program.\nIn order to "
                     "use it you have to provide an input and an output image.\nThe "
                     "patter is as follows:\n./image-scaler input.png output.png"
                  << '\n';
        return 1;
    }
    if (argc != 3) {
        std::cout << "Calling the program should follow such a pattern\n"
                     "./image-scaler input.png output.png"
                  << '\n';
        return 1;
    }
    std::cout << "\nPreparing your image." << '\n';
    const unsigned int cnt_threads = std::thread::hardware_concurrency();
    if (cnt_threads > 2) {
        std::cout << "You have " << cnt_threads << " threads. Running in a multi-threaded mode." << '\n';

        std::thread t1(run, std::ref(argc), std::ref(argv));
        std::thread t2(notify);

        t2.join();
        t1.join();
    } else {
        std::cout << "You have " << cnt_threads << "threads. Running in a single-threaded mode." << '\n';
        return run(argc, argv);
    }
    return 0;
}
