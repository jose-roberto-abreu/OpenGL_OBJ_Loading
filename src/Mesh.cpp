//-----------------------------------------------------------------------------
// Mesh.cpp by Steve Jones 
// Copyright (c) 2015-2019 Game Institute. All Rights Reserved.
//
// Basic Mesh class
//-----------------------------------------------------------------------------
#include "Mesh.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
Mesh::Mesh()
	:mLoaded(false)
{
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
Mesh::~Mesh()
{
    for (const auto& entry : mMeshes) 
    {
        glDeleteVertexArrays(1, &entry.VAO);
        glDeleteBuffers(1, &entry.VBO);
        glDeleteBuffers(1, &entry.EBO);
    }
}

void Mesh::loadModel(const std::string& path)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
    {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }

    std::cout << "Number of meshes: " << scene->mNumMeshes << std::endl;
    mMeshes.clear();

    for (unsigned int i = 0; i < scene->mNumMeshes; i++) 
    {
        aiMesh *mesh = scene->mMeshes[i];
        MeshEntry entry;

        for (unsigned int j = 0; j < mesh->mNumVertices; j++)
        {
            Vertex aVertex;
            aiVector3D vertex = mesh->mVertices[j];
            aVertex.position = glm::vec3(vertex.x, vertex.y, vertex.z);

            if (mesh->HasTextureCoords(0))
            {
                aiVector3D texCoord = mesh->mTextureCoords[0][j];
                aVertex.texCoords = glm::vec2(texCoord.x, texCoord.y);
            }

            entry.vertices.push_back(aVertex);
        }

        for (unsigned int j = 0; j < mesh->mNumFaces; j++) 
        {
            aiFace face = mesh->mFaces[j];
            for (unsigned int k = 0; k < face.mNumIndices; k++)
            {
                entry.indices.push_back(face.mIndices[k]);
            }
        }

        mMeshes.push_back(entry);
    }

    std::cout << "Model has been loaded" << std::endl;
    mLoaded = true;
    initBuffers();
}

//-----------------------------------------------------------------------------
// Create and initialize the vertex buffer and vertex array object
// Must have valid, non-empty std::vector of Vertex objects.
//-----------------------------------------------------------------------------
void Mesh::initBuffers()
{
    for (auto& entry: mMeshes)
    {
        glGenVertexArrays(1, &entry.VAO);
        glGenBuffers(1, &entry.VBO);
        glGenBuffers(1, &entry.EBO);

        glBindVertexArray(entry.VAO);
        glBindBuffer(GL_ARRAY_BUFFER, entry.VBO);
        glBufferData(GL_ARRAY_BUFFER, entry.vertices.size() * sizeof(Vertex), entry.vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, entry.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, entry.indices.size() * sizeof(unsigned int), entry.indices.data(), GL_STATIC_DRAW);

        // Vertex Positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);

        // Vertex Texture Coords
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(3 * sizeof(GLfloat)));

        // unbind to make sure other code does not change it somewhere else
        glBindVertexArray(0);
    }
}

//-----------------------------------------------------------------------------
// Render the mesh
//-----------------------------------------------------------------------------
void Mesh::draw()
{
	if (!mLoaded) return;

    for (const auto& entry : mMeshes) 
    {
	    glBindVertexArray(entry.VAO);
	    glDrawElements(GL_TRIANGLES, entry.indices.size(), GL_UNSIGNED_INT, 0);
    }

	glBindVertexArray(0);
}

