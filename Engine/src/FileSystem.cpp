#include "FileSystem.h"
#include <assimp/scene.h>
#include <assimp/postprocess.h> 
#include <assimp/cimport.h>
#include <iostream>
#include <windows.h>
#include "Application.h"

FileSystem::FileSystem() : Module() {}
FileSystem::~FileSystem() {}

bool FileSystem::Awake()
{
	return true;
}

bool FileSystem::Start()
{
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

	std::string warriorPath = rootDir + "\\Assets\\warrior.fbx";

	std::cout << "Trying to load: " << warriorPath << std::endl;

	unsigned int importFlags = aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded;

	/*if (LoadFBX(warriorPath, importFlags))
	{
		std::cout << "Successfully loaded warrior.fbx!" << std::endl;
	}
	else
	{
		std::cerr << "Failed to load warrior.fbx. Use drag & drop." << std::endl;
	}*/
	// For loading Baker_house at the beggining
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
			// Clean up previous meshes before loading new ones
			ClearMeshes();

			if (LoadFBX(filePath))
			{
				std::cout << "Successfully loaded FBX file!" << std::endl;
			}
			else
			{
				std::cerr << "Failed to load FBX file!" << std::endl;
			}
		}
		else if (fileType == DROPPED_TEXTURE)
		{
			// Apply texture to the current mesh on screen (change it when we have more than one mesh in screen)
			Application::GetInstance().renderer->LoadTexture(filePath);
		}
	}

	return true;
}

bool FileSystem::LoadFBX(const std::string& file_path, unsigned int flag)
{
	std::cout << "Loading FBX: " << file_path << std::endl;

	// Flags with coordinate conversion
	unsigned int importFlags = flag != 0 ? flag :
		aiProcessPreset_TargetRealtime_MaxQuality |
		aiProcess_ConvertToLeftHanded;

	const aiScene* scene = aiImportFile(file_path.c_str(), importFlags);

	if (scene == nullptr)
	{
		std::cerr << "Error loading: " << aiGetErrorString() << std::endl;
		return false;
	}

	if (!scene->HasMeshes())
	{
		std::cerr << "No meshes in scene" << std::endl;
		aiReleaseImport(scene);
		return false;
	}

	for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
	{
		aiMesh* aiMesh = scene->mMeshes[i];
		Mesh mesh;

		mesh.num_vertices = aiMesh->mNumVertices;
		mesh.vertices = new float[mesh.num_vertices * 3];

		// Copy vertices 
		float scale = 0.05f;
		for (unsigned int v = 0; v < mesh.num_vertices; ++v)
		{
			mesh.vertices[v * 3] = aiMesh->mVertices[v].x;
			mesh.vertices[v * 3 + 1] = aiMesh->mVertices[v].y;
			mesh.vertices[v * 3 + 2] = aiMesh->mVertices[v].z;
		}

		if (aiMesh->HasTextureCoords(0))
		{
			mesh.texCoords = new float[mesh.num_vertices * 2];
			for (unsigned int v = 0; v < mesh.num_vertices; ++v)
			{
				mesh.texCoords[v * 2] = aiMesh->mTextureCoords[0][v].x;
				mesh.texCoords[v * 2 + 1] = aiMesh->mTextureCoords[0][v].y;
			}
		}
		else
		{
			// If there are no UVs, create default coordinates
			mesh.texCoords = new float[mesh.num_vertices * 2];
			for (unsigned int v = 0; v < mesh.num_vertices * 2; ++v)
			{
				mesh.texCoords[v * 2] = 0.0f;
				mesh.texCoords[v * 2 + 1] = 0.0f;
			}
		}

		// Indices
		if (aiMesh->HasFaces())
		{
			mesh.num_indices = aiMesh->mNumFaces * 3;
			mesh.indices = new unsigned int[mesh.num_indices];

			for (unsigned int j = 0; j < aiMesh->mNumFaces; ++j)
				memcpy(&mesh.indices[j * 3], aiMesh->mFaces[j].mIndices, 3 * sizeof(unsigned int));
		}

		Application::GetInstance().renderer->LoadMesh(mesh);
		meshes.push_back(mesh);

		std::cout << "Mesh " << i << ": " << mesh.num_vertices << " vertices, " << mesh.num_indices << " indices" << std::endl;
	}

	aiReleaseImport(scene);
	return true;
}

bool FileSystem::LoadFBXFromAssets(const std::string& filename)
{
	return LoadFBX("Assets/" + filename, aiProcessPreset_TargetRealtime_MaxQuality);
}

void FileSystem::AddMesh(const Mesh& mesh)
{
	meshes.push_back(mesh);
}

void FileSystem::ClearMeshes()
{
	for (auto& mesh : meshes)
	{
		Application::GetInstance().renderer->UnloadMesh(mesh);
		delete[] mesh.vertices;
		delete[] mesh.indices;
		delete[] mesh.texCoords;
	}
	meshes.clear();
	std::cout << "All meshes cleared" << std::endl;
}

bool FileSystem::CleanUp()
{
	ClearMeshes();
	aiDetachAllLogStreams();
	return true;
}