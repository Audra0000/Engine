#pragma once

#include "Component.h"
#include <string>
#include <memory>

class Texture;

class ComponentMaterial : public Component {
public:
    ComponentMaterial(GameObject* owner);
    ~ComponentMaterial();

    void Update() override;
    void OnEditor() override;

    bool LoadTexture(const std::string& path);
    void CreateCheckerboardTexture();
	void RestoreOriginalTexture(); // for module editor
    void Use();
    void Unbind();
    bool HasTexture() const { return texture != nullptr; }
	bool HasOriginalTexture() const { return hasOriginalTexture; } // for module editor

    const std::string& GetTexturePath() const { return texturePath; }
	const std::string& GetOriginalTexturePath() const { return originalTexturePath; } // for module editor
    int GetTextureWidth() const;
    int GetTextureHeight() const;

private:
    std::unique_ptr<Texture> texture;
    std::string texturePath;

    std::string originalTexturePath; 
    bool hasOriginalTexture = false;
};