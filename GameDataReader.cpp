// GameDataReader.cpp : Defines the entry point for the application.

#include "GameDataReader.h"

using namespace std;
#include <windows.h>
#include <vector>
#include <string>
#include <SDL.h>
#undef main
#include <iostream>
#include <SDL_video.h>

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    std::vector<std::string>* windowTitles = reinterpret_cast<std::vector<std::string>*>(lParam);
    char buffer[256];
    int length = GetWindowTextA(hwnd, buffer, sizeof(buffer));
    if (length > 0)
    {
        windowTitles->push_back(std::string(buffer, length));
    }
    return TRUE;
}


bool record_window_video(SDL_Window* window, const char* filename, int fps, int duration_s)
{
    SDL_Init(SDL_INIT_VIDEO);

    // Get the size of the window
    int width, height;
    SDL_GetWindowSize(window, &width, &height);

    // Create a renderer for the window
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        std::cerr << "Failed to create renderer" << std::endl;
        return false;
    }

    // Create a texture to capture the window
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (!texture)
    {
        std::cerr << "Failed to create texture" << std::endl;
        return false;
    }

    // Initialize the video writer
    SDL_RWops* rwops = SDL_RWFromFile(filename, "wb");
    if (!rwops)
    {
        std::cerr << "Failed to open file for writing" << std::endl;
        return false;
    }

    // Start recording
    Uint32 start_time = SDL_GetTicks();
    while (SDL_GetTicks() - start_time < duration_s * 1000)
    {
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        // Get the texture data
        void* pixels;
        int pitch;
        SDL_LockTexture(texture, NULL, &pixels, &pitch);

        // Write the frame to the video file
        if (SDL_RWwrite(rwops, pixels, 1, pitch * height) != pitch * height)
        {
            std::cerr << "Failed to write frame" << std::endl;
            return false;
        }

        SDL_UnlockTexture(texture);

        // Sleep to maintain the desired frame rate
        SDL_Delay(1000 / fps);
    }

    // Clean up
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_RWclose(rwops);
    return true;
}

int main(int argc, char* argv[])
{
    std::vector<std::string> windowTitles;
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&windowTitles));

    if (windowTitles.empty())
    {
        std::cerr << "No windows found" << std::endl;
        return 1;
    }

    std::cout << "Available windows:" << std::endl;
    for (std::size_t i = 0; i < windowTitles.size(); i++)
    {
        std::cout << "  " << (i + 1) << ") " << windowTitles[i] << std::endl;
    }
    std::cout << "Enter the number of the window to capture: ";
    std::size_t index;
    std::cin >> index;
    if (index < 1 || index > windowTitles.size())
    {
        std::cerr << "Invalid index" << std::endl;
        return 1;
    }

    std::string windowTitle = windowTitles[index - 1];
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL could not initialize. SDL Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Window Title", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        640, 480, SDL_WINDOW_SHOWN);
    if (window == nullptr)
    {
        std::cerr << "Window could not be created. SDL Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    if (!record_window_video(window, "file1", 20, 20)) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}


