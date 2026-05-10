# C++ Wrapper around `libusbhid` for Corsair iCUE Nexus

The [iCUE Nexus](https://www.corsair.com/de/de/p/pc-components-accessories/ch-9910010-na/icue-nexus-companion-touch-screen-ch-9910010-na) is a 640x48 24-bit color touch
display from Corsair that can be attached to their keyboards or be used as stand-alone displays on your desk.

It is designed to integrate into Corsair's iCUE Software ecosystem to display device status, control device profiles, etc.

The iCUE software has several problems though:
- It is Windows only 😒
- It is utterly bloated and a nightmare to use, in my opionion

For that reason, I created a simple C++ wrapper around [`libusb`](https://github.com/libusb/libusb) that should (hopefully) be pretty straightforward to use.

# Examples

## Basic

Here's a simply demo program:

```cpp
#include "ICueNexus.h"

int main()
{
    ICueNexus nexus;

    // Maximum brightness
    nexus.SetBrightness(100);

    // Play bult-in animation #2 in endless loop
    nexus.PlayAnimation(2, true);
}
```

## Rendering with SDL3

To display an `SDL_Surface` on the Nexus, simply create a surface in the correct format and pass a pointer to its pixel data to `ShowImage()`:

```cpp
// Surface dimensions must be 640x48 and pixel format must be SDL_PIXELFORMAT_BGRA32
SDL_Surface canvas = ...;

ICueNexus nexus;
nexus.ShowImage(static_cast<uint8_t *>(canvas.get()->pixels));
```

For a full example, see the included `main.cpp` file.