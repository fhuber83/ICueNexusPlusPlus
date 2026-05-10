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
