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
	glDeleteVertexArrays(1, &mVAO);
	glDeleteBuffers(1, &mVBO);
}

//-----------------------------------------------------------------------------
// Loads a Wavefront OBJ model
//
// NOTE: This is not a complete, full featured OBJ loader.  It is greatly
// simplified.
// Assumptions!
//  - OBJ file must contain only triangles
//  - We ignore materials
//  - We ignore normals
//  - only commands "v", "vt" and "f" are supported
//-----------------------------------------------------------------------------
bool Mesh::loadOBJ(const std::string& filename)
{
    std::vector<unsigned int> vertexIndices, uvIndices;
    std::vector<glm::vec3> tempVertices;
    std::vector<glm::vec2> tempUVs;

    if (filename.find(".obj") != std::string::npos)
    {
        std::ifstream fin(filename, std::ios::in);
        if (!fin)
        {
            std::cerr << "Cannot open " << filename << std::endl;
            return false;
        }

        std::cout << "Loading OBJ file " << filename << " ..." << std::endl;

        std::string lineBuffer;
        while (std::getline(fin, lineBuffer))
        {
            if (lineBuffer.substr(0, 2) == "v ")
            {
                std::istringstream v(lineBuffer.substr(2));
                glm::vec3 vertex;
                v >> vertex.x; v >> vertex.y; v >> vertex.z;
                tempVertices.push_back(vertex);
            }
            else if (lineBuffer.substr(0, 2) == "vt")
            {
                std::istringstream vt(lineBuffer.substr(3));
                glm::vec2 uv;
                vt >> uv.s; vt >> uv.t;
                tempUVs.push_back(uv);
            }
            else if (lineBuffer.substr(0, 2) == "f ")
            {
                // Parse the face data
                std::istringstream f(lineBuffer.substr(2));
                std::vector<std::string> faceIndices;
                std::string indexGroup;

                while (f >> indexGroup)
                {
                    faceIndices.push_back(indexGroup);
                }

                // Check if it's a triangle or a quad
                if (faceIndices.size() == 3 || faceIndices.size() == 4)
                {
                    // Process face as triangles
                    for (size_t i = 0; i < faceIndices.size() - 2; i++)
                    {
                        // Always create a triangle (triangulating the quad)
                        processFaceVertex(faceIndices[0], vertexIndices, uvIndices);
                        processFaceVertex(faceIndices[i + 1], vertexIndices, uvIndices);
                        processFaceVertex(faceIndices[i + 2], vertexIndices, uvIndices);
                    }
                }
                else
                {
                    std::cerr << "Unsupported face format in OBJ file" << std::endl;
                }
            }
        }

        // Close the file
        fin.close();

        // For each vertex of each triangle
        for (unsigned int i = 0; i < vertexIndices.size(); i++)
        {
            // Get the attributes using the indices
            glm::vec3 vertex = tempVertices[vertexIndices[i] - 1];
            glm::vec2 uv = tempUVs[uvIndices[i] - 1];

            Vertex meshVertex;
            meshVertex.position = vertex;
            meshVertex.texCoords = uv;

            mVertices.push_back(meshVertex);
        }

        // Create and initialize the buffers
        initBuffers();

        return (mLoaded = true);
    }

    // We shouldn't get here so return failure
    return false;
}

void Mesh::processFaceVertex(
    const std::string& faceData,
    std::vector<unsigned int>& vertexIndices,
    std::vector<unsigned int>& uvIndices)
{
    int p = 0, t = 0, n = 0;
    if (sscanf(faceData.c_str(), "%d/%d/%d", &p, &t, &n) >= 2)
    {
        vertexIndices.push_back(p);
        uvIndices.push_back(t);
    }
    else if (sscanf(faceData.c_str(), "%d//%d", &p, &n) == 2)
    {
        vertexIndices.push_back(p);
    }
    else if (sscanf(faceData.c_str(), "%d", &p) == 1)
    {
        vertexIndices.push_back(p);
    }
    else
    {
        std::cerr << "Failed to parse face vertex: " << faceData << std::endl;
    }
}

//-----------------------------------------------------------------------------
// Create and initialize the vertex buffer and vertex array object
// Must have valid, non-empty std::vector of Vertex objects.
//-----------------------------------------------------------------------------
void Mesh::initBuffers()
{
	glGenVertexArrays(1, &mVAO);
	glGenBuffers(1, &mVBO);

	glBindVertexArray(mVAO);
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, mVertices.size() * sizeof(Vertex), &mVertices[0], GL_STATIC_DRAW);

	// Vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);

	// Vertex Texture Coords
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(3 * sizeof(GLfloat)));

	// unbind to make sure other code does not change it somewhere else
	glBindVertexArray(0);
}

//-----------------------------------------------------------------------------
// Render the mesh
//-----------------------------------------------------------------------------
void Mesh::draw()
{
	if (!mLoaded) return;

	glBindVertexArray(mVAO);
	glDrawArrays(GL_TRIANGLES, 0, mVertices.size());
	glBindVertexArray(0);
}

