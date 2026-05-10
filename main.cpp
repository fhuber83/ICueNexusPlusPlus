#include <hidapi/hidapi.h>
#include <iostream>
#include <array>
#include <algorithm>
#include <cstring>
#include <iomanip>
#include <memory>
#include <chrono>

class ICueNexus
{
    static constexpr std::uint16_t  VENDOR_ID    = 0x1b1c;
    static constexpr std::uint16_t  PRODUCT_ID   = 0x1b8e;
    static constexpr bool           DEBUG_OUTPUT = false;

public:

    static constexpr uint16_t       SCREEN_WIDTH = 640;
    static constexpr uint16_t       SCREEN_HEIGHT = 48;
    static constexpr uint32_t       IMAGE_SIZE    = SCREEN_WIDTH * SCREEN_HEIGHT * 4;

    ICueNexus()
    {
        if (hid_init() != 0)
        {
            throw std::runtime_error("Failed to initialize HID API");
        }

        device_ = hid_open(VENDOR_ID, PRODUCT_ID, nullptr);

        if (!device_)
        {
            hid_exit();
            throw std::runtime_error("Failed to open HID device");
        }

        if (hid_set_nonblocking(device_, 1) != 0)
        {
            hid_close(device_);
            hid_exit();
            throw std::runtime_error("Failed to set non-blocking mode");
        }
    }


    virtual ~ICueNexus()
    {
        if (device_ != nullptr)
        {
            hid_close(device_);
            device_ = nullptr;
        }

        hid_exit();
    }


    /**
     * Set display brightness
     * @param brightness Brightness in the range from 0 to 100
     */
    void SetBrightness(uint8_t brightness)
    {
        // Cap brightness to maximum of 100
        brightness = std::min(static_cast<uint8_t>(100), brightness);

        SendReport(3, 1, brightness);
    }

    void PlayAnimation(uint8_t animation, bool loop = true)
    {
        animation = std::clamp<uint8_t>(animation, 1, 3);

        SendReport(3, 13, animation, loop ? 1 : 0);
    }

    void StopAnimation()
    {
        SendReport(3, 15);
    }

    void BlankScreen()
    {
        SendReport(3, 4);
    }

    void ShowImage(const uint8_t * image)
    {
        uint8_t buffer[1024];

        // An 8-byte block is sent with every chunk
        struct ReportHeader
        {
            uint8_t endpoint;           // 0x02
            uint8_t command;            // 0x05
            uint8_t sub_command;        // ?? Always 0x40
            uint8_t is_last_block;      // 1 if this is the last block, 0 otherwise
            uint16_t block_number;      // Block number, starting from 0
            uint16_t payload_length;    // Size of payload, in bytes
        };

        auto * header = reinterpret_cast<ReportHeader *>(buffer);

        header->endpoint       = 0x02;
        header->command        = 0x05;
        header->sub_command    = 0x40;
        header->is_last_block  = 0;
        header->block_number   = 0;
        header->payload_length = 0;

        // Make sure our header struct isn't padded
        static_assert(std::has_unique_object_representations_v<ReportHeader>, "ReportHeader must have a unique object representation");
        static_assert(sizeof(ReportHeader) == 8, "ReportHeader must be exactly 8 bytes in size");

        uint32_t image_data_offset = 0;

        // Number of payload bytes (image data) yet to be sent
        uint32_t payload_bytes_remaining = IMAGE_SIZE;

        // Maximum number of payload bytes we can send per transfer
        const uint32_t MAX_PAYLOAD_PER_PACKET = sizeof(buffer) - sizeof(ReportHeader);

        // Main transfer loop
        while (payload_bytes_remaining > 0)
        {
            uint32_t payload_bytes_in_this_packet = 0;

            if (payload_bytes_remaining > MAX_PAYLOAD_PER_PACKET)
            {
                payload_bytes_in_this_packet = MAX_PAYLOAD_PER_PACKET;
            }
            else
            {
                payload_bytes_in_this_packet = payload_bytes_remaining;
                header->is_last_block = 1;
            }

            header->payload_length = payload_bytes_in_this_packet;

            // if constexpr(DEBUG_OUTPUT)
            // {
            //     std::cerr << "Sending block " << header->block_number << " with " << payload_bytes_in_this_packet << " bytes of image data ("
            //               << payload_bytes_remaining << " bytes remaining after this block)" << std::endl;
            // }

            memcpy(buffer + sizeof(ReportHeader), image + image_data_offset, payload_bytes_in_this_packet);

            hid_write(device_, buffer, sizeof(buffer));

            header->block_number += 1;

            image_data_offset += payload_bytes_in_this_packet;
            payload_bytes_remaining -= payload_bytes_in_this_packet;
        }
    }

private:

    hid_device * device_;

    template <typename... Bytes>
    void SendReport(Bytes... bytes)
    {
        if (device_ == nullptr)
        {
            throw std::runtime_error("HID device not initialized");
        }

        constexpr std::size_t N = sizeof...(bytes);
        std::array<std::byte, N> buffer { static_cast<std::byte>(bytes)... };

        if constexpr(DEBUG_OUTPUT)
        {
            std::cout << "Sending report bytes : [ ";
            for (const auto & byte : buffer)
            {
                std::cout << std::hex << std::fixed << std::setw(2) << std::setfill('0') << std::uppercase << std::to_integer<int>(byte) << " ";
            }
            std::cout << std::dec << "]" << std::endl;
        }

        hid_send_feature_report(device_, reinterpret_cast<const unsigned char *>(buffer.data()), buffer.size());
    }
};

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