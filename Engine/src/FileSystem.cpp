#include "FileSystem.h"
#include <assimp/scene.h>
#include <assimp/postprocess.h> 
#include <assimp/cimport.h>
#include <iostream>
#include "Application.h"

FileSystem::FileSystem() : Module() {}
FileSystem::~FileSystem() {}

bool FileSystem::Awake()
{
	return true;
}

bool FileSystem::Start()
{
    LoadFBX("C:\\Users\\aalca\\Documents\\GitHub\\-engine\\Engine\\Assets\\warrior.FBX");
	return true;
}

bool FileSystem::Update()
{
	return true;
}

bool FileSystem::LoadFBX(const std::string& file_path, unsigned int flag)
{
	std::cout << "Loading FBX: " << file_path << std::endl;

	const aiScene* scene = aiImportFile(file_path.c_str(),
		flag != 0 ? flag : aiProcessPreset_TargetRealtime_MaxQuality);

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
		memcpy(mesh.vertices, aiMesh->mVertices, sizeof(float) * mesh.num_vertices * 3);

		// Scale
		for (unsigned int k = 0; k < mesh.num_vertices * 3; ++k)
			mesh.vertices[k] *= 0.005f;

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
	}
	meshes.clear();
}

bool FileSystem::CleanUp()
{
	ClearMeshes();
	aiDetachAllLogStreams();
	return true;
}