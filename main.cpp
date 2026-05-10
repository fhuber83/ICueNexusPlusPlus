// C/C++ standard headers
#include <iostream>
#include <chrono>
#include <thread>
#include <format>
#include <csignal>

// OS-dependant headers
#include <unistd.h>

// 3rd party headers
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

// Project headers
#include <cmath>

#include "ICueNexus.h"


// RAII deleters
struct SDLSurfaceDeleter { void operator()(SDL_Surface* s) const { SDL_DestroySurface(s); } };
struct TTFFontDeleter    { void operator()(TTF_Font*    f) const { TTF_CloseFont(f); }      };

using SurfacePtr = std::unique_ptr<SDL_Surface, SDLSurfaceDeleter>;
using FontPtr    = std::unique_ptr<TTF_Font,    TTFFontDeleter>;


// Helper function to create an SDL render surface
[[nodiscard]] SurfacePtr makeSurface(int w, int h, SDL_PixelFormat fmt)
{
    SurfacePtr s{ SDL_CreateSurface(w, h, fmt) };

    if (!s)
        throw std::runtime_error(SDL_GetError());

    return s;
}


// Helper function to open a font file
[[nodiscard]] FontPtr openFont(const char* path, float ptsize)
{
    FontPtr f{ TTF_OpenFont(path, ptsize) };

    if (!f)
        throw std::runtime_error(SDL_GetError());

    return f;
}

volatile bool run = true;

void SIGINT_Handler(int signum)
{
    run = false;
}


int main()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        throw std::runtime_error(SDL_GetError());
    }

    if (!TTF_Init())
    {
        SDL_Quit();
        throw std::runtime_error(SDL_GetError());
    }

    try
    {
        // Create a surface with the exact dimensions and pixel format as the iCUE Nexus
        auto canvas = makeSurface(ICueNexus::SCREEN_WIDTH, ICueNexus::SCREEN_HEIGHT, SDL_PIXELFORMAT_BGRA32);

        // Clear screen (fill with a dark blue color)
        const SDL_PixelFormatDetails * fmtDetails = SDL_GetPixelFormatDetails(canvas->format);

        const Uint32 backgroundColor = SDL_MapRGB(fmtDetails, nullptr, 15, 15, 40);

        const auto font = openFont("font.ttf", 28.0f);

        constexpr SDL_Color foregroundColor{ 220, 220, 255, 255 };

        // Install signal handler for CTRL-C
        std::signal(SIGINT, SIGINT_Handler);

        while (run)
        {
            SDL_FillSurfaceRect(canvas.get(), nullptr, backgroundColor);

            auto truncated = std::chrono::zoned_time{
                std::chrono::current_zone(),
                std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now())
            };

            auto foo = std::format("{:%H:%M:%S}", truncated);

            const SurfacePtr textSurface{TTF_RenderText_Blended(font.get(), foo.c_str(), 0, foregroundColor)};

            if (!textSurface)
                throw std::runtime_error(SDL_GetError());

            // Center text on screen
            const SDL_Rect fontCenteredRect
            {
                (ICueNexus::SCREEN_WIDTH - textSurface->w) / 2,
                (ICueNexus::SCREEN_HEIGHT - textSurface->h) / 2,
                textSurface->w,
                textSurface->h
            };

            // Copy ("blit") rendered text to our main surface
            SDL_BlitSurface(textSurface.get(), nullptr, canvas.get(), &fontCenteredRect);

            // Display image on iCUE Nexus
            ICueNexus nexus;
            nexus.ShowImage(static_cast<uint8_t *>(canvas.get()->pixels));

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Error: " << ex.what() << '\n';
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    TTF_Quit();
    SDL_Quit();
    return 0;
}
