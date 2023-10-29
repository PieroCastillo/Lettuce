#include <iostream>
#include <vector>
#include <SDL.h>
#include <SDL_syswm.h>
#include "Lettuce.Core.h"

void initWindow();
void endWindow();
void drawFrame();

const int width = 800;
const int height = 600;

SDL_Window* window;
void* windowHandle;
void* platformHandle;
std::vector<LettuceCore::GPU> gpus;
LettuceCore::Factory factory;
LettuceCore::Device device;


int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    initWindow();
    //startLettuce();

    bool shouldClose = false;

    while (!shouldClose)
    {
        SDL_Event evt;
        while (SDL_PollEvent(&evt))
        {
            if (evt.type == SDL_QUIT)
            {
                shouldClose = true;
            }
        }
        drawFrame();
    }
    //finishLettuce();
    endWindow();
    SDL_Quit();
    return 0;
}

void drawFrame(){
}

void startLettuce() {
    factory.Create("Triangle Sample", false);
    gpus = factory.GetGPUs();
    device.Create(factory, gpus[0], windowHandle, platformHandle);
    //device.SetSize(width, height);
    //device.LoadView();
}

void finishLettuce(){
    device.Destroy();
    factory.Destroy();
    gpus = factory.GetGPUs();
}

void initWindow() {
    window = SDL_CreateWindow(
        "Triangle Sample",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        0
    );
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    SDL_GetWindowWMInfo(window, &info);
    windowHandle = info.info.win.window;
    platformHandle = info.info.win.hinstance;
}

void endWindow() {
    SDL_DestroyWindow(window);
}