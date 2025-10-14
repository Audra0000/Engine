#include "Renderer.h"
#include "Application.h"
#include <glad/glad.h>
#include <iostream>

Renderer::Renderer()
{
    std::cout << "Renderer Constructor" << std::endl;
}

Renderer::~Renderer()
{
}

bool Renderer::Start()
{
    std::cout << "Initializing Renderer" << std::endl;

    // Create default shader
    defaultShader = std::make_unique<Shader>();

    if (!defaultShader->Create())
    {
        std::cerr << "Failed to create default shader" << std::endl;
        return false;
    }

    std::cout << "Renderer initialized successfully" << std::endl;
    return true;
}

void Renderer::LoadMesh(Mesh& mesh)
{
    unsigned int VAO, VBO, EBO;

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // VBO for vertices
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh.num_vertices * 3 * sizeof(float), mesh.vertices, GL_STATIC_DRAW);

    // EBO for indices
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.num_indices * sizeof(unsigned int), mesh.indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Save IDs in mesh
    mesh.id_vertex = VBO;
    mesh.id_index = EBO;
    mesh.id_VAO = VAO;

    glBindVertexArray(0);
}

void Renderer::DrawMesh(const Mesh& mesh)
{
    if (mesh.id_VAO == 0)
    {
        std::cerr << "ERROR: Trying to draw mesh without VAO" << std::endl;
        return;
    }

    glBindVertexArray(mesh.id_VAO);
    glDrawElements(GL_TRIANGLES, mesh.num_indices, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void Renderer::UnloadMesh(Mesh& mesh)
{
    if (mesh.id_VAO != 0)
    {
        glDeleteVertexArrays(1, &mesh.id_VAO);
        mesh.id_VAO = 0;
    }

    if (mesh.id_vertex != 0)
    {
        glDeleteBuffers(1, &mesh.id_vertex);
        mesh.id_vertex = 0;
    }

    if (mesh.id_index != 0)
    {
        glDeleteBuffers(1, &mesh.id_index);
        mesh.id_index = 0;
    }
}

bool Renderer::Update()
{
    defaultShader->Use();

    const std::vector<Mesh>& meshes = Application::GetInstance().filesystem->GetMeshes();

    for (const auto& mesh : meshes)
    {
        DrawMesh(mesh);
    }

    return true;
}

bool Renderer::CleanUp()
{
    std::cout << "Cleaning up Renderer" << std::endl;

    if (defaultShader)
    {
        defaultShader->Delete();
    }

    return true;
}