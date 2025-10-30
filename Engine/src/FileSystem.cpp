#include "FileSystem.h"
#include <assimp/scene.h>
#include <assimp/postprocess.h> 
#include <assimp/cimport.h>
#include <windows.h>
#include "Application.h"
#include "GameObject.h"
#include "Transform.h"
#include "ComponentMesh.h"
#include "ComponentMaterial.h"

FileSystem::FileSystem() : Module() {}
FileSystem::~FileSystem() {}

bool FileSystem::Awake()
{
    return true;
}

bool FileSystem::Start()
{

    LOG_DEBUG("Initializing FileSystem module");
    LOG_CONSOLE("FileSystem initialized");

    // Get directory of executable
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string execPath(buffer);
    size_t pos = execPath.find_last_of("\\/");
    std::string execDir = execPath.substr(0, pos);

    // Move up two levels: from build/ to Engine/, then to root
    pos = execDir.find_last_of("\\/");
    std::string parentDir = execDir.substr(0, pos);
    pos = parentDir.find_last_of("\\/");
    std::string rootDir = parentDir.substr(0, pos);

    std::string housePath = rootDir + "\\Assets\\BakerHouse.fbx";

    LOG_DEBUG("Attempting to load default model: %s", housePath.c_str());
    LOG_CONSOLE("Loading default scene...");

    GameObject* houseModel = LoadFBXAsGameObject(housePath);

    if (houseModel != nullptr)
    {
        //FORCE correct position and scale
        Transform* t = static_cast<Transform*>(houseModel->GetComponent(ComponentType::TRANSFORM));
        if (t != nullptr)
        {
            t->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
            t->SetScale(glm::vec3(0.01f, 0.01f, 0.01f));  // <-- IMPORTANTE REVISAR: escala pequeña
        }

        GameObject* root = Application::GetInstance().scene->GetRoot();
        root->AddChild(houseModel);
        LOG_DEBUG("FBX loaded from: %s", housePath.c_str());
        LOG_CONSOLE("Default model loaded: %s", housePath.c_str());

    }
    else
    {
        LOG_CONSOLE("WARNING: Could't load default model");
        LOG_DEBUG("Failed to load default FBX, creating fallback geometry");

        GameObject* pyramidObject = new GameObject("Pyramid");
        ComponentMesh* meshComp = static_cast<ComponentMesh*>(pyramidObject->CreateComponent(ComponentType::MESH));
        Mesh pyramidMesh = Primitives::CreatePyramid();
        meshComp->SetMesh(pyramidMesh);

        GameObject* root = Application::GetInstance().scene->GetRoot();
        root->AddChild(pyramidObject);

        LOG_DEBUG("Failed to load default FBX, using fallback pyramid. Use drag & drop");
        LOG_CONSOLE("Using fallback geometry");
    }

    return true;
}

bool FileSystem::Update()
{
    if (Application::GetInstance().input->HasDroppedFile())
    {
        std::string filePath = Application::GetInstance().input->GetDroppedFilePath();
        DroppedFileType fileType = Application::GetInstance().input->GetDroppedFileType();
        Application::GetInstance().input->ClearDroppedFile();

        if (fileType == DROPPED_FBX)
        {
            LOG_DEBUG("Dropped FBX file detected: %s", filePath.c_str());
            LOG_CONSOLE("Loading dropped model...");

            GameObject* loadedModel = LoadFBXAsGameObject(filePath);
            if (loadedModel != nullptr)
            {
                GameObject* root = Application::GetInstance().scene->GetRoot();
                root->AddChild(loadedModel);

                LOG_DEBUG("Model loaded successfully");
                LOG_DEBUG("   Root GameObject: %s", loadedModel->GetName().c_str());
                LOG_DEBUG("   Children: %d", loadedModel->GetChildren().size());
                LOG_CONSOLE("Model loaded successfully: %s", loadedModel->GetName().c_str());
            }
            else
            {
                LOG_DEBUG("ERROR: Failed to load dropped FBX file");
                LOG_CONSOLE("Failed to load model");
            }
        }
        else if (fileType == DROPPED_TEXTURE)
        {
            LOG_DEBUG("Dropped texture file detected: %s", filePath.c_str());
            LOG_CONSOLE("Loading texture...");
            Application::GetInstance().renderer->LoadTexture(filePath);
        }
    }

    return true;
}

bool FileSystem::CleanUp()
{
    aiDetachAllLogStreams();
    LOG_CONSOLE("FileSystem cleaned up");
    return true;
}

GameObject* FileSystem::LoadFBXAsGameObject(const std::string& file_path)
{
    LOG_DEBUG("=== Loading FBX with ASSIMP ===");
    LOG_DEBUG("File: %s", file_path.c_str());
    LOG_CONSOLE("Loading model with ASSIMP...");

    unsigned int importFlags = aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded;

    LOG_DEBUG("ASSIMP import flags: TargetRealtime_MaxQuality | ConvertToLeftHanded");

    const aiScene* scene = aiImportFile(file_path.c_str(), importFlags);

    if (scene == nullptr)
    {
        LOG_DEBUG("ERROR: ASSIMP failed to load file");
        LOG_DEBUG("ASSIMP Error: %s", aiGetErrorString());
        LOG_CONSOLE("ERROR: Failed to load model - %s", aiGetErrorString());
        return nullptr;
    }

    if (!scene->HasMeshes())
    {
        LOG_DEBUG("ERROR: No meshes found in scene");
        LOG_CONSOLE("ERROR: No geometry found in model");

        aiReleaseImport(scene);
        return nullptr;
    }

    // Extraer directorio del archivo para texturas
    std::string directory = file_path.substr(0, file_path.find_last_of("/\\"));

    LOG_DEBUG("=== ASSIMP Scene Information ===");
    LOG_DEBUG("  Meshes: %d", scene->mNumMeshes);
    LOG_DEBUG("  Materials: %d", scene->mNumMaterials);
    LOG_DEBUG("  Nodes: %d", CountNodes(scene->mRootNode));
    LOG_CONSOLE("ASSIMP: Found %d meshes, %d materials, %d nodes", scene->mNumMeshes, scene->mNumMaterials, CountNodes(scene->mRootNode));

    GameObject* rootObject = ProcessNode(scene->mRootNode, scene, directory);

    aiReleaseImport(scene);

    LOG_DEBUG("=== FBX Loading Complete ===");
    LOG_DEBUG("GameObject hierarchy created successfully");
    LOG_CONSOLE("Model loaded successfully");

    return rootObject;
}

int FileSystem::CountNodes(aiNode* node)
{
    int count = 1;
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        count += CountNodes(node->mChildren[i]);
    }
    return count;
}

GameObject* FileSystem::ProcessNode(aiNode* node, const aiScene* scene, const std::string& directory)
{
    // ======================================================== 1 ====================================================
    // Create GameObject 
    std::string nodeName = node->mName.C_Str();
    if (nodeName.empty()) nodeName = "Unnamed";

    GameObject* gameObject = new GameObject(nodeName);

    LOG_DEBUG("Processing node: %s", nodeName.c_str());

    // ======================================================== 2 ====================================================
    // Get Transform component

    Transform* transform = static_cast<Transform*>(gameObject->GetComponent(ComponentType::TRANSFORM));

    if (transform != nullptr)
    {
        aiVector3D position, scaling;
        aiQuaternion rotation;
        node->mTransformation.Decompose(scaling, rotation, position);

        transform->SetPosition(glm::vec3(position.x, position.y, position.z));
        transform->SetScale(glm::vec3(scaling.x, scaling.y, scaling.z));

        // NO aplicar la rotación del FBX
        // transform->SetRotationQuat(glm::quat(rotation.w, rotation.x, rotation.y, rotation.z));
        // En su lugar, dejar rotación por defecto (identidad)
        transform->SetRotationQuat(glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
    }

    // ======================================================== 3 ====================================================
    // Process meshes
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        unsigned int meshIndex = node->mMeshes[i];
        aiMesh* aiMesh = scene->mMeshes[meshIndex];

        LOG_DEBUG("  Processing mesh %d: %s", i, aiMesh->mName.C_Str());

        // Procesar mesh con nueva estructura
        Mesh mesh = ProcessMesh(aiMesh, scene);

        // Crear ComponentMesh y asignar
        ComponentMesh* meshComponent = static_cast<ComponentMesh*>(gameObject->CreateComponent(ComponentType::MESH));
        meshComponent->SetMesh(mesh);

        // ======================================================== 4 ====================================================
        // Material handling
        if (aiMesh->mMaterialIndex >= 0)
        {
            aiMaterial* material = scene->mMaterials[aiMesh->mMaterialIndex];

            if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
            {
                aiString texturePath;
                material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath);

                LOG_DEBUG("    Material has texture: %s", texturePath.C_Str());

                ComponentMaterial* matComponent = static_cast<ComponentMaterial*>(gameObject->GetComponent(ComponentType::MATERIAL));

                if (matComponent == nullptr)
                {
                    matComponent = static_cast<ComponentMaterial*>(gameObject->CreateComponent(ComponentType::MATERIAL));
                }

                std::string textureFile = texturePath.C_Str();
                if (textureFile.find('/') == std::string::npos && textureFile.find('\\') == std::string::npos)
                {
                    textureFile = directory + "\\" + textureFile;
                }

                matComponent->LoadTexture(textureFile);
            }
        }
    }

    // ======================================================== 5 ====================================================
    // Recursive for children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        GameObject* child = ProcessNode(node->mChildren[i], scene, directory);
        if (child != nullptr)
        {
            gameObject->AddChild(child);
        }
    }

    return gameObject;
}

Mesh FileSystem::ProcessMesh(aiMesh* aiMesh, const aiScene* scene)
{
    Mesh mesh;

    // Reserve space for efficiency
    mesh.vertices.reserve(aiMesh->mNumVertices);
    mesh.indices.reserve(aiMesh->mNumFaces * 3);

    // ========================================================
    // Process vertices
    // ========================================================
    for (unsigned int i = 0; i < aiMesh->mNumVertices; i++)
    {
        Vertex vertex;

        // Position
        vertex.position = glm::vec3(
            aiMesh->mVertices[i].x,
            aiMesh->mVertices[i].y,
            aiMesh->mVertices[i].z
        );

        // Normals
        if (aiMesh->HasNormals())
        {
            vertex.normal = glm::vec3(
                aiMesh->mNormals[i].x,
                aiMesh->mNormals[i].y,
                aiMesh->mNormals[i].z
            );
        }
        else
        {
            vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
        }

        // Texture coordinates
        if (aiMesh->HasTextureCoords(0))
        {
            vertex.texCoords = glm::vec2(
                aiMesh->mTextureCoords[0][i].x,
                aiMesh->mTextureCoords[0][i].y
            );
        }
        else
        {
            vertex.texCoords = glm::vec2(0.0f, 0.0f);
        }

        mesh.vertices.push_back(vertex);
    }

    // ========================================================
    // Process indices
    // ========================================================
    for (unsigned int i = 0; i < aiMesh->mNumFaces; i++)
    {
        aiFace face = aiMesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            mesh.indices.push_back(face.mIndices[j]);
        }
    }

    LOG_DEBUG("      Mesh processed: Vertices: %d, Indices: %d, Triangles: %d", mesh.vertices.size(), mesh.indices.size(), mesh.indices.size() / 3);
    LOG_CONSOLE("  Mesh processed: %d vertices, %d triangles", mesh.vertices.size(), mesh.indices.size() / 3);
    return mesh;
}

std::vector<TextureInfo> FileSystem::LoadMaterialTextures(aiMaterial* mat, unsigned int type,
    const std::string& typeName,
    const std::string& directory)
{
    std::vector<TextureInfo> textures;

    for (unsigned int i = 0; i < mat->GetTextureCount((aiTextureType)type); i++)
    {
        aiString str;
        mat->GetTexture((aiTextureType)type, i, &str);

        TextureInfo texture;
        texture.path = directory + "/" + str.C_Str();
        texture.type = typeName;
        // El ID se asignará después en el Renderer
        texture.id = 0;

        textures.push_back(texture);
    }

    return textures;
}