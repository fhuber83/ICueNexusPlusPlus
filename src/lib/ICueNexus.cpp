#include <cstdint>

#include "ICueNexus.h"

#include <cstring>


ICueNexus::ICueNexus()
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



ICueNexus::~ICueNexus()
{
    if (device_ != nullptr)
    {
        hid_close(device_);
        device_ = nullptr;
    }

    hid_exit();
}



void ICueNexus::SetBrightness(uint8_t brightness)
{
    // Cap brightness to maximum of 100
    brightness = std::min(static_cast<uint8_t>(100), brightness);

    SendReport(3, 1, brightness);
}



void ICueNexus::PlayAnimation(uint8_t animation, bool loop)
{
    animation = std::clamp<uint8_t>(animation, 1, 3);

    SendReport(3, 13, animation, loop ? 1 : 0);
}



void ICueNexus::StopAnimation()
{
    SendReport(3, 15);
}



void ICueNexus::BlankScreen()
{
    SendReport(3, 4);
}



void ICueNexus::ShowImage(const uint8_t * image)
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
    constexpr uint32_t MAX_PAYLOAD_PER_PACKET = sizeof(buffer) - sizeof(ReportHeader);

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

        std::memcpy(buffer + sizeof(ReportHeader), image + image_data_offset, payload_bytes_in_this_packet);

        hid_write(device_, buffer, sizeof(buffer));

        header->block_number += 1;

        image_data_offset += payload_bytes_in_this_packet;
        payload_bytes_remaining -= payload_bytes_in_this_packet;
    }
}
