#include "Window.h"
#include <iostream>
#include <glad/glad.h>

Window::Window() : window(nullptr), width(1280), height(720), scale(1)
{
    std::cout << "Window Constructor" << std::endl;
}

Window::~Window()
{
}

bool Window::Start()
{
    std::cout << "Init SDL3 Window" << std::endl;

    // Initialize SDL3
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL_Init failed! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Set OpenGL version to 4.6
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);

    // Use the core OpenGL profile (modern functions only)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Enable double buffering to prevent flickering
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // Set depth buffer to 24 bits for proper 3D rendering
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // Create window WITH OpenGL flag
    window = SDL_CreateWindow(
        "SDL3 OpenGL Window",
        width,
        height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );

    if (window == nullptr)
    {
        std::cerr << "Window creation failed! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}

bool Window::Update()
{
    int newWidth, newHeight;
    SDL_GetWindowSize(window, &newWidth, &newHeight);

    if (newWidth != width || newHeight != height)
    {
        width = newWidth;
        height = newHeight;
        glViewport(0, 0, width, height);
    }

    return true;
}

bool Window::PostUpdate()
{
    Render();
    return true;
}

void Window::Render()
{
    SDL_GL_SwapWindow(window);
}

bool Window::CleanUp()
{
    std::cout << "Destroying SDL Window" << std::endl;
    if (window != nullptr)
    {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    SDL_Quit();
    return true;
}

void Window::GetWindowSize(int& width, int& height) const
{
    SDL_GetWindowSize(window, &width, &height);
}

int Window::GetScale() const
{
    return scale;
}