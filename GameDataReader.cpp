// GameDataReader.cpp : Defines the entry point for the application.

    
using namespace std;
#include <windows.h>
#include <vector>
#include <string>
#include <SDL.h>
#undef main
#include <iostream>
#include <SDL_video.h>
#include <algorithm>
#include <SDL_syswm.h>
#ifdef _WIN32  // for Windows systems
#include <Windows.h>
#include <Winuser.h>
#include <psapi.h>
#else  // for Linux systems
#include <X11/Xlib.h>
#endif
#include <map>


std::vector <std::pair<std::string, HWND>> get_application_windows() {
    std::vector <std::pair<std::string, HWND>> windows;

#ifdef _WIN32  // for Windows systems
    HWND hwnd;
    char title[256];
    DWORD pid, tid;
    HWND shell = GetShellWindow();
    HWND hwndForeground = GetForegroundWindow();
    DWORD foregroundThreadId = GetWindowThreadProcessId(hwndForeground, &pid);
    HMODULE hMods[1024];
    HANDLE hProcess;
    DWORD cbNeeded;

    // Get the list of all top-level windows
    hwnd = GetTopWindow(nullptr);
    while (hwnd != nullptr) {
        if (IsWindowVisible(hwnd) && (GetParent(hwnd) == nullptr)) {
            GetWindowThreadProcessId(hwnd, &pid);
            tid = GetWindowThreadProcessId(hwnd, nullptr);
            if (pid != 0 && pid != 4 && hwnd != shell) { // filter out system processes
                hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
                if (hProcess != nullptr) {
                    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
                        if (GetWindowTextA(hwnd, title, sizeof(title)) > 0) { // filter out windows without a title
                            windows.push_back({title, hwnd});

                        }
                    }
                    CloseHandle(hProcess);
                }
            }
        }
        hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
    }
#else  // for Linux systems
    Display* display = XOpenDisplay(nullptr);
    Window root = DefaultRootWindow(display);
    Atom prop = XInternAtom(display, "_NET_CLIENT_LIST_STACKING", False), type;
    int format;
    unsigned long nitems, bytes_after;
    unsigned char* data;
    XGetWindowProperty(display, root, prop, 0, LONG_MAX, False, XA_WINDOW, &type, &format, &nitems, &bytes_after, &data);
    for (unsigned long i = 0; i < nitems; i++) {
        Window win = ((Window*)data)[i];
        XTextProperty text_prop;
        if (XGetWMName(display, win, &text_prop) != 0) {
            if (text_prop.value != nullptr && text_prop.nitems > 0) { // filter out windows without a title
                char** list;
                int count;
                if (XmbTextPropertyToTextList(display, &text_prop, &list, &count) == Success) {
                    windows.push_back(list[0]);
                    XFreeStringList(list);
                }
            }
            XFree(text_prop.value);
        }
    }
    XFree(data);
    XCloseDisplay(display);
#endif

    return windows;
}





// Function to display a list of options and get user input
int get_user_input(const std::vector <std::pair<std::string, HWND>>& options)
{
    std::cout << "Please select a window title:" << std::endl;
    for (int i = 0; i < options.size(); ++i)
    {
        std::cout << i + 1 << ". " << options[i].first << std::endl;
    }

    int choice = 0;
    while (choice < 1 || choice > options.size())
    {
        cin >> choice;
    }
    return choice;
}


std::vector<bool> check_keys_pressed(std::vector<int> keys)
{


    // Initialize key state array
    const Uint8* keystate = SDL_GetKeyboardState(NULL);
    std::vector<bool> keys_pressed;
    for (auto key : keys)
    {
        keys_pressed.push_back(false);
    }

    // Check if keys are pressed
    for (int i = 0; i < keys.size(); i++)
    {
        if (keystate[keys[i]])
        {
            keys_pressed[i] = true;
        }
        else
        {
            keys_pressed[i] = false;
        }
    }

    // Cleanup SDL
    SDL_Quit();

    return keys_pressed;
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
        check_keys_pressed({ SDL_SCANCODE_A, SDL_SCANCODE_B, SDL_SCANCODE_C });
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
    std::vector <std::pair<std::string, HWND>> window_titles = get_application_windows();

    if (window_titles.empty())
    {
        std::cerr << "No windows found" << std::endl;
        return 1;
    }

    int choice = get_user_input(window_titles);
    std::string selected_title = window_titles[choice].first;

    std::cout << "You selected window title: " << selected_title << std::endl;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL could not initialize. SDL Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    SDL_Window* window = SDL_CreateWindowFrom(window_titles[choice].second);
    if (window == nullptr)
    {
        std::cerr << "Window could not be created. SDL Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    if (!record_window_video(window, "file1.mp4", 20, 5)) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}


