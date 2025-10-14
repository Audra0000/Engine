#include "Primitives.h"
#include <cmath>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Mesh Primitives::CreateTriangle()
{
    Mesh mesh;

    mesh.num_vertices = 3;
    mesh.vertices = new float[9] {
        -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
            0.0f, 0.5f, 0.0f
        };

    mesh.num_indices = 3;
    mesh.indices = new unsigned int[3] { 0, 1, 2 };

    return mesh;
}

Mesh Primitives::CreateCube()
{
    Mesh mesh;

    mesh.num_vertices = 8;
    mesh.vertices = new float[24] {
        -0.5f, -0.5f, 0.5f,  // 0
            0.5f, -0.5f, 0.5f,  // 1
            0.5f, 0.5f, 0.5f,  // 2
            -0.5f, 0.5f, 0.5f,  // 3
            -0.5f, -0.5f, -0.5f,  // 4
            0.5f, -0.5f, -0.5f,  // 5
            0.5f, 0.5f, -0.5f,  // 6
            -0.5f, 0.5f, -0.5f   // 7
        };

    mesh.num_indices = 36;
    mesh.indices = new unsigned int[36] {
        0, 1, 2, 2, 3, 0,  // frontal
            5, 4, 7, 7, 6, 5,  // trasera
            4, 0, 3, 3, 7, 4,  // izquierda
            1, 5, 6, 6, 2, 1,  // derecha
            3, 2, 6, 6, 7, 3,  // superior
            4, 5, 1, 1, 0, 4   // inferior
        };

    return mesh;
}

Mesh Primitives::CreatePyramid()
{
    Mesh mesh;

    mesh.num_vertices = 5;
    mesh.vertices = new float[15] {
        -0.5f, -0.5f, 0.5f,  // 0
            0.5f, -0.5f, 0.5f,  // 1
            0.5f, -0.5f, -0.5f,  // 2
            -0.5f, -0.5f, -0.5f,  // 3
            0.0f, 0.5f, 0.0f   // 4 
        };

    mesh.num_indices = 18;
    mesh.indices = new unsigned int[18] {
        0, 1, 4,  // frontal
            1, 2, 4,  // derecha
            2, 3, 4,  // trasera
            3, 0, 4,  // izquierda
            0, 2, 1,  // base
            0, 3, 2   // base
        };

    return mesh;
}

Mesh Primitives::CreatePlane(float width, float height)
{
    Mesh mesh;

    mesh.num_vertices = 4;
    float halfW = width * 0.5f;
    float halfH = height * 0.5f;

    mesh.vertices = new float[12] {
        -halfW, 0.0f, -halfH,  // 0
            halfW, 0.0f, -halfH,  // 1
            halfW, 0.0f, halfH,  // 2
            -halfW, 0.0f, halfH   // 3
        };

    mesh.num_indices = 6;
    mesh.indices = new unsigned int[6] {
        0, 1, 2,
            2, 3, 0
        };

    return mesh;
}

Mesh Primitives::CreateSphere(float radius, unsigned int rings, unsigned int sectors)
{
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    float const R = 1.0f / (float)(rings - 1);
    float const S = 1.0f / (float)(sectors - 1);

    for (unsigned int r = 0; r < rings; r++)
    {
        for (unsigned int s = 0; s < sectors; s++)
        {
            float const y = sin(-M_PI / 2 + M_PI * r * R);
            float const x = cos(2 * M_PI * s * S) * sin(M_PI * r * R);
            float const z = sin(2 * M_PI * s * S) * sin(M_PI * r * R);

            vertices.push_back(x * radius);
            vertices.push_back(y * radius);
            vertices.push_back(z * radius);
        }
    }

    for (unsigned int r = 0; r < rings - 1; r++)
    {
        for (unsigned int s = 0; s < sectors - 1; s++)
        {
            indices.push_back(r * sectors + s);
            indices.push_back(r * sectors + (s + 1));
            indices.push_back((r + 1) * sectors + (s + 1));

            indices.push_back(r * sectors + s);
            indices.push_back((r + 1) * sectors + (s + 1));
            indices.push_back((r + 1) * sectors + s);
        }
    }

    Mesh mesh;
    mesh.num_vertices = vertices.size() / 3;
    mesh.vertices = new float[vertices.size()];
    memcpy(mesh.vertices, vertices.data(), vertices.size() * sizeof(float));

    mesh.num_indices = indices.size();
    mesh.indices = new unsigned int[indices.size()];
    memcpy(mesh.indices, indices.data(), indices.size() * sizeof(unsigned int));

    return mesh;
}

Mesh Primitives::CreateCylinder(float radius, float height, unsigned int segments)
{
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    float halfHeight = height * 0.5f;

    // Top center
    vertices.push_back(0.0f);
    vertices.push_back(halfHeight);
    vertices.push_back(0.0f);

    // Bottom center
    vertices.push_back(0.0f);
    vertices.push_back(-halfHeight);
    vertices.push_back(0.0f);

    // Top and bottom circle vertices
    for (unsigned int i = 0; i <= segments; i++)
    {
        float angle = 2.0f * M_PI * i / segments;
        float x = cos(angle) * radius;
        float z = sin(angle) * radius;

        // Top circle
        vertices.push_back(x);
        vertices.push_back(halfHeight);
        vertices.push_back(z);

        // Bottom circle
        vertices.push_back(x);
        vertices.push_back(-halfHeight);
        vertices.push_back(z);
    }

    // Top cap
    for (unsigned int i = 0; i < segments; i++)
    {
        indices.push_back(0);
        indices.push_back(2 + i * 2);
        indices.push_back(2 + (i + 1) * 2);
    }

    // Bottom cap
    for (unsigned int i = 0; i < segments; i++)
    {
        indices.push_back(1);
        indices.push_back(3 + (i + 1) * 2);
        indices.push_back(3 + i * 2);
    }

    // Side faces
    for (unsigned int i = 0; i < segments; i++)
    {
        unsigned int topCurrent = 2 + i * 2;
        unsigned int bottomCurrent = 3 + i * 2;
        unsigned int topNext = 2 + (i + 1) * 2;
        unsigned int bottomNext = 3 + (i + 1) * 2;

        indices.push_back(topCurrent);
        indices.push_back(bottomCurrent);
        indices.push_back(topNext);

        indices.push_back(topNext);
        indices.push_back(bottomCurrent);
        indices.push_back(bottomNext);
    }

    Mesh mesh;
    mesh.num_vertices = vertices.size() / 3;
    mesh.vertices = new float[vertices.size()];
    memcpy(mesh.vertices, vertices.data(), vertices.size() * sizeof(float));

    mesh.num_indices = indices.size();
    mesh.indices = new unsigned int[indices.size()];
    memcpy(mesh.indices, indices.data(), indices.size() * sizeof(unsigned int));

    return mesh;
}