#pragma once

#include <glad/glad.h>

class Texture
{
public:
    Texture();
    ~Texture();

    // Crear textura checkerboard
    void CreateCheckerboard();

    // Bind/Unbind
    void Bind();
    void Unbind();

    GLuint GetID() const { return textureID; }

private:
    GLuint textureID;
};