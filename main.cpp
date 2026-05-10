#include <iostream>
#include <array>
#include <iomanip>
#include <chrono>

#include "ICueNexus.h"


/**
 * Creates a test image that is filled with one color
 *
 * @param red   Red component of the color (0-255)
 * @param green Green component of the color (0-255)
 * @param blue  Blue component of the color (0-255)
 *
 * @return Raw byte array containing the image data (no headers)
 */
constexpr std::array<std::uint8_t, ICueNexus::SCREEN_WIDTH * ICueNexus::SCREEN_HEIGHT * 4> CreateTestImage(uint8_t red, uint8_t green, uint8_t blue)
{
    std::array<std::uint8_t, ICueNexus::SCREEN_WIDTH * ICueNexus::SCREEN_HEIGHT * 4> image{};

    for (size_t i = 0; i < image.size(); i += 4)
    {
        image[i + 0] = blue;
        image[i + 1] = green;
        image[i + 2] = red;
        image[i + 3] = 255;
    }

    return image;
}



int main()
{
    ICueNexus nexus;

    nexus.SetBrightness(100);
    nexus.StopAnimation();
    nexus.BlankScreen();

    const auto t1 = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 256; i++)
    {
        auto test_image = CreateTestImage(0, 0, i); // Red image
        nexus.ShowImage(test_image.data());
    }

    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();

    const auto time_per_frame_us = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

    const double frame_s = static_cast<double>(time_per_frame_us) * 1E-6;
    const double fps = 256.0 / frame_s;

    std::cout << "256 images sent in " << std::setprecision(3) << std::fixed << frame_s << "s (" << std::setprecision(1) << fps << " fps)" << std::endl;
}