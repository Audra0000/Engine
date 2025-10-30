#include "Texture.h"
#include <iostream>
#include <windows.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <fstream>
#include "Log.h"

#define CHECKERS_WIDTH 64
#define CHECKERS_HEIGHT 64

Texture::Texture() : textureID(0), width(0), height(0), nrChannels(0)
{
}

Texture::~Texture()
{
    if (textureID != 0)
        glDeleteTextures(1, &textureID);
}

void Texture::CreateCheckerboard()
{
    LOG_DEBUG("Creating checkerboard pattern texture");
    // patron checkerboard
    static GLubyte checkerImage[CHECKERS_HEIGHT][CHECKERS_WIDTH][4];

    for (int i = 0; i < CHECKERS_HEIGHT; i++) {
        for (int j = 0; j < CHECKERS_WIDTH; j++) {
            int c = ((((i & 0x8) == 0) ^ ((j & 0x8) == 0))) * 255;
            checkerImage[i][j][0] = (GLubyte)c;
            checkerImage[i][j][1] = (GLubyte)c;
            checkerImage[i][j][2] = (GLubyte)c;
            checkerImage[i][j][3] = (GLubyte)255;
        }
    }

    // Generate texture in OpenGL
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, CHECKERS_WIDTH, CHECKERS_HEIGHT,
        0, GL_RGBA, GL_UNSIGNED_BYTE, checkerImage);

    glBindTexture(GL_TEXTURE_2D, 0);

    width = CHECKERS_WIDTH;
    height = CHECKERS_HEIGHT;
    nrChannels = 4;

    LOG_DEBUG("Checkerboard texture created - Size: %dx%d, ID: %d", width, height, textureID);
    LOG_CONSOLE("Default checkerboard texture ready");
}

void Texture::InitDevIL()
{
    static bool initialized = false;
    if (!initialized)
    {
        LOG_DEBUG("Initializing DevIL library");

        ilInit();
        iluInit();
        initialized = true;

        ILint devilVersion = ilGetInteger(IL_VERSION_NUM);
        int devilMajor = devilVersion / 100;
        int devilMinor = (devilVersion / 10) % 10;
        int devilPatch = devilVersion % 10;
        LOG_DEBUG("DevIL initialized successfully - Version: %d.%d.%d", devilMajor, devilMinor, devilPatch);
        LOG_CONSOLE("DevIL library initialized - Version: %d.%d.%d", devilMajor, devilMinor, devilPatch);
    }
}

// Helper function to normalize path separators
std::string NormalizePath(const std::string& path)
{
    std::string normalized = path;
    for (char& c : normalized)
    {
        if (c == '\\')
            c = '/';
    }
    return normalized;
}

// Helper function to check if path is absolute
bool IsAbsolutePath(const std::string& path)
{
    // Windows: Check for drive letter (C:, D:, etc.) or UNC path (\\)
    if (path.length() >= 2)
    {
        if (path[1] == ':' || (path[0] == '\\' && path[1] == '\\'))
            return true;
    }
    // Unix: Check for root (/)
    if (!path.empty() && path[0] == '/')
        return true;

    return false;
}

// Helper function to check if file exists
bool FileExists(const std::string& path)
{
    std::ifstream file(path);
    return file.good();
}

bool Texture::LoadFromFile(const std::string& path, bool flipVertically)
{
 
    InitDevIL();

    std::string fullPath;

    // If the path is absolute (dropped file), use it directly
    if (IsAbsolutePath(path))
    {
        fullPath = path;
    }
    else
    {
        // If relative, build path from executable
        char buffer[MAX_PATH];
        GetModuleFileNameA(NULL, buffer, MAX_PATH);
        std::string execPath(buffer);
        size_t pos = execPath.find_last_of("\\/");
        std::string execDir = execPath.substr(0, pos);

        // Go up two levels: from build/ to Engine/, then to root
        pos = execDir.find_last_of("\\/");
        std::string parentDir = execDir.substr(0, pos);
        pos = parentDir.find_last_of("\\/");
        std::string rootDir = parentDir.substr(0, pos);

        // Build full path to texture
        fullPath = rootDir + "\\" + path;
    }

    // Normalize the path (convert backslashes to forward slashes)
    fullPath = NormalizePath(fullPath);

    LOG_DEBUG("=== DevIL Texture Loading ===");
    LOG_DEBUG("Path: %s", fullPath.c_str());

    // Verify that the file exists before attempting to load it
    if (!FileExists(fullPath))
    {
        LOG_DEBUG("ERROR: Texture file does not exist");
        LOG_CONSOLE("ERROR: Texture file not found");
        return false;
    }

    // Generate an image ID in DevIL
    ILuint imageID;
    ilGenImages(1, &imageID);
    ilBindImage(imageID);

    // Load the image
    if (!ilLoadImage(fullPath.c_str()))
    {
        ILenum error = ilGetError();
        LOG_DEBUG("ERROR: DevIL failed to load image");
        LOG_DEBUG("DevIL Error: %s", iluErrorString(error));
        LOG_CONSOLE("ERROR: DevIL failed to load texture");
        ilDeleteImages(1, &imageID);
        return false;
    }

    // Convert the image to RGBA (standard format)
    if (!ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE))
    {
        LOG_DEBUG("ERROR: Failed to convert image to RGBA format");
        LOG_CONSOLE("ERROR: Failed to convert texture format");
        ilDeleteImages(1, &imageID);
        return false;
    }

    // Get image information
    width = ilGetInteger(IL_IMAGE_WIDTH);
    height = ilGetInteger(IL_IMAGE_HEIGHT);
    nrChannels = ilGetInteger(IL_IMAGE_CHANNELS);
    ILubyte* data = ilGetData();

    LOG_DEBUG("Image properties:");
    LOG_DEBUG("  Width: %d pixels", width);
    LOG_DEBUG("  Height: %d pixels", height);
    LOG_DEBUG("  Channels: %d", nrChannels);
    LOG_DEBUG("  Size: %.2f KB", (width * height * 4) / 1024.0f);
    LOG_CONSOLE("DevIL: Texture loaded - %dx%d, %d channels, %.2f KB", width, height, nrChannels, (width * height * 4) / 1024.0f);

    if (!data)
    {
        LOG_DEBUG("ERROR: Failed to get image data from DevIL");
        LOG_CONSOLE("ERROR: Could not extract texture data");
        ilDeleteImages(1, &imageID);
        return false;
    }

    LOG_DEBUG("Creating OpenGL texture object");

    // Generate and configure the texture in OpenGL
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Configure wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load image in OpenGL 
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    LOG_DEBUG("OpenGL texture created - ID: %d", textureID);

    ilDeleteImages(1, &imageID);

    return true;
}

void Texture::Bind()
{
    glBindTexture(GL_TEXTURE_2D, textureID);
}

void Texture::Unbind()
{
    glBindTexture(GL_TEXTURE_2D, 0);
}