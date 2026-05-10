#pragma once

#include <array>
#include <memory>

#include <cstdint>

#include <hidapi/hidapi.h>

class ICueNexus
{
    /// USB Vendor ID ("Corsair")
    static constexpr std::uint16_t  VENDOR_ID    = 0x1b1c;

    /// USB Device ID ("CORSAIR iCUE NEXUS")
    static constexpr std::uint16_t  PRODUCT_ID   = 0x1b8e;

public:

    /// Number of columns that the screen can display (Width)
    static constexpr uint16_t       SCREEN_WIDTH = 640;

    /// Number of rows the screen can display (height)
    static constexpr uint16_t       SCREEN_HEIGHT = 48;

    /// Number of bytes in each full sceen image
    static constexpr uint32_t       IMAGE_SIZE    = SCREEN_WIDTH * SCREEN_HEIGHT * 4;

    /**
     * Constructs an object encapsulating a Corsair iCue Nexus device. Uses libhidapi internally
     *
     * @throws std::runtime_error if the device cannot be found or initialized
     */
    ICueNexus();

    virtual ~ICueNexus();

    /**
     * Set display brightness
     * @param brightness Brightness in the range from 0 to 100
     */
    void SetBrightness(uint8_t brightness);


    /**
     * Play one of the three built-in animations
     * @param animation Animation number, in the range from 1 to 3
     * @param loop Shall the animation be repeated infinitely?
     */
    void PlayAnimation(uint8_t animation, bool loop = true);


    /**
     * Stops playback of built-in animations
     */
    void StopAnimation();


    /**
     * Clears screen contents to all black
     */
    void BlankScreen();


    /**
     * Transfers a new image to the device and displays it automatically once complete
     * @param image Pointer to raw image data in BGRA format (4 bytes per pixel, blue first). The image must be exactly 640x48 pixels in size (122,880 bytes).
     */
    void ShowImage(const uint8_t * image);

private:

    hid_device * device_;

    /**
     * Helper function to send a feature report with an arbitrary number of bytes
     * to the device.
     * @tparam Bytes
     * @param bytes Bytes to send as feature report
     */
    template <typename... Bytes>
    void SendReport(Bytes... bytes)
    {
        if (device_ == nullptr)
        {
            throw std::runtime_error("HID device not initialized");
        }

        constexpr std::size_t N = sizeof...(bytes);
        std::array<std::byte, N> buffer { static_cast<std::byte>(bytes)... };

        hid_send_feature_report(device_, reinterpret_cast<const unsigned char *>(buffer.data()), buffer.size());
    }
};
