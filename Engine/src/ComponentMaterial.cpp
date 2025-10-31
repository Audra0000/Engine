#include "ComponentMaterial.h"
#include "GameObject.h"
#include "Texture.h"
#include <iostream>
#include "Log.h"

ComponentMaterial::ComponentMaterial(GameObject* owner)
    : Component(owner, ComponentType::MATERIAL),
    texture(nullptr),
    texturePath(""),
    originalTexturePath(""),
    hasOriginalTexture(false)
{
    CreateCheckerboardTexture();
}

ComponentMaterial::~ComponentMaterial()
{

}

void ComponentMaterial::Update()
{

}

void ComponentMaterial::OnEditor()
{

}

bool ComponentMaterial::LoadTexture(const std::string& path)
{
    LOG_DEBUG("ComponentMaterial: Loading texture from %s", path.c_str());

    auto newTexture = std::make_unique<Texture>();

    if (newTexture->LoadFromFile(path))
    {
        texture = std::move(newTexture);
        texturePath = path;

        originalTexturePath = path;
        hasOriginalTexture = true;

        LOG_DEBUG("ComponentMaterial: Texture loaded");
        LOG_CONSOLE("Texture loaded: %s", path.c_str());

        return true;
    }
    else
    {
        LOG_DEBUG("ComponentMaterial: Failed to load texture: %s", path.c_str());
        LOG_CONSOLE("Failed to load texture");

        return false;
    }
}

void ComponentMaterial::CreateCheckerboardTexture()
{

    LOG_DEBUG("ComponentMaterial: Checkerboard texture created");

    texture = std::make_unique<Texture>();

    texture->CreateCheckerboard();

    texturePath = "[Checkerboard Pattern]";
}

void ComponentMaterial::Use()
{
    if (texture)
    {
        texture->Bind();
    }
}

void ComponentMaterial::Unbind()
{
    if (texture)
    {
        texture->Unbind();
    }
}

int ComponentMaterial::GetTextureWidth() const
{
    if (texture)
    {
        return texture->GetWidth();
    }
    return 0;
}

int ComponentMaterial::GetTextureHeight() const
{
    if (texture)
    {
        return texture->GetHeight();
    }
    return 0;
}

void ComponentMaterial::RestoreOriginalTexture()
{
    if (hasOriginalTexture && !originalTexturePath.empty())
    {
        auto newTexture = std::make_unique<Texture>();

        if (newTexture->LoadFromFile(originalTexturePath))
        {
			texture = std::move(newTexture); // Move is used here to transfer ownership
            texturePath = originalTexturePath;

            LOG_DEBUG("ComponentMaterial: Original texture restored");
            LOG_CONSOLE("Original texture restored: %s", originalTexturePath.c_str());
        }
        else
        {
            LOG_DEBUG("ComponentMaterial: Failed to restore original texture");
            LOG_CONSOLE("Failed to restore original texture");

            CreateCheckerboardTexture();
        }
    }
    else
    {
        LOG_DEBUG("ComponentMaterial: No original texture to restore");
        LOG_CONSOLE("No original texture available to restore");
    }
}