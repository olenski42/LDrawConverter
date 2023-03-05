#include "LDrawExporter.hpp"
#include <vector>
#include "Common/Common.h"
#include "glm/glm.hpp"
#include <queue>


void ReduceMesh(MeshData* meshData)
{
    std::vector<glm::vec3> vertices;
    std::vector<glm::ivec3> faces;
    std::vector<ColorID> colors;

    std::vector<int> vertexMap(meshData->vertices.size(), -1);

    for (int i = 0; i < meshData->faces.size(); i++)
    {
        glm::ivec3 face = meshData->faces[i];
        ColorID color = meshData->colors[i];

        for (int j = 0; j < 3; j++)
        {
            int vertexIndex = face[j];
            glm::vec3 vertex = meshData->vertices[vertexIndex];

            bool found = false;
            for (int k = 0; k < vertices.size(); k++)
            {
                if (vertices[k] == vertex)
                {
                    vertexMap[vertexIndex] = k;
                    found = true;
                    break;
                }
            }

            if (found == false)
            {
                vertexMap[vertexIndex] = vertices.size();
                vertices.push_back(vertex);
                colors.push_back(color);
            }
        }

        faces.push_back(glm::ivec3(vertexMap[face.x], vertexMap[face.y], vertexMap[face.z]));
    }

    meshData->vertices = vertices;
    meshData->faces = faces;
    meshData->colors = colors;
}

inline unsigned int convSizetUint(size_t size)
{
    if (size > UINT_MAX)
    {
        throw std::length_error(std::string("invalid conversion from size_t to unsigend int (size: %z)", size));
    }
    return static_cast<unsigned int>(size);
}

inline int convSizetInt(size_t size)
{
    if (size > INT_MAX)
    {
        throw std::length_error(std::string("invalid conversion from size_t to int (size: %z)", size));
    }
    return static_cast<int>(size);
}

LDrawExporter::LDrawExporter(LDrawConverter* converter)
    : m_converter(converter)
{
    m_sdkManager = FbxManager::Create();
    m_scene = FbxScene::Create(m_sdkManager, "My Scene");
    InitializeSdkObjects(m_sdkManager, m_scene);
}

void LDrawExporter::Export(LDrawFile* file, const char* outPath)
{
    bool lResult;

    FbxNode* lRootNode = m_scene->GetRootNode();

    FbxNode* node = CreateNode(file);
    lRootNode->AddChild(node);

    // Save the scene.
    lResult = SaveScene(m_sdkManager, m_scene, outPath, 0);

    if (lResult == false)
    {
        FBXSDK_printf("\n\nAn error occurred while saving the scene...\n");
        DestroySdkObjects(m_sdkManager, lResult);
        exit(-1);
    }

    // Destroy all objects created by the FBX SDK.
    DestroySdkObjects(m_sdkManager, lResult);
}

void LDrawExporter::LoadParts()
{
    for (auto it = m_converter->nameResolver.begin(); it != m_converter->nameResolver.end(); it++)
    {
        LDrawFile *ldrawFile = it->second;
        if (ldrawFile->fileType == FILETYPE_PART)
        {
            MeshData meshData;
            MergeRecursion(ldrawFile, &meshData, glm::mat4(1.0f), false, 16);
            FbxMesh* mesh = CreateMesh(&meshData);
            partMap.emplace(ldrawFile, mesh);
        }
    }
}

FbxNode* LDrawExporter::CreateNode(LDrawFile* ldrawFile)
{
    FbxNode* node = FbxNode::Create(m_scene, "unnamed");

    MeshData meshData;
    MergeRecursion(ldrawFile, &meshData, glm::mat4(1.0f), false, 16);
    //ReduceMesh(&meshData);
    FbxMesh* mesh = CreateMesh(&meshData);
    node->SetNodeAttribute(mesh);
    
    return node;
}

FbxMesh* LDrawExporter::CreateMesh(MeshData *meshData)
{
    FbxMesh *mesh = FbxMesh::Create(m_scene, "Mesh");
    mesh->InitControlPoints(convSizetInt(meshData->vertices.size()));

    mesh->InitControlPoints(meshData->vertices.size());
    for (int i = 0; i < meshData->vertices.size(); ++i)
    {
        FbxVector4 vertex(meshData->vertices[i].x, meshData->vertices[i].y, meshData->vertices[i].z);
        mesh->SetControlPointAt(vertex, i);
    }


     // Add faces to the mesh
    for (int i = 0; i < meshData->faces.size(); ++i)
    {
        mesh->BeginPolygon();
        mesh->AddPolygon(meshData->faces[i].x);
        mesh->AddPolygon(meshData->faces[i].y);
        mesh->AddPolygon(meshData->faces[i].z);
        mesh->EndPolygon();
    }

    mesh->GenerateNormals();


    return mesh;
}

void LDrawExporter::MergeRecursion(LDrawFile* currentFile, MeshData* meshData, glm::mat4 matToRoot, bool bfcInvert, ColorID color)
{
    unsigned int rootVertexAmount = convSizetUint(meshData->vertices.size());
    for(int i = 0; i < currentFile->vertices.size(); i++)
    {
        glm::vec4 vertex = glm::vec4(currentFile->vertices[i], 1.0f);
        vertex = matToRoot * vertex;

        meshData->vertices.push_back(glm::vec3(vertex.x, vertex.y, vertex.z));
    }

    for (Face& f : currentFile->faces)
    {
        //invert face direction
        if (bfcInvert == false)
            meshData->faces.push_back(glm::ivec3(f.vertexIndices.x + rootVertexAmount, f.vertexIndices.y + rootVertexAmount, f.vertexIndices.z + rootVertexAmount));
        else
            meshData->faces.push_back(glm::ivec3(f.vertexIndices.z + rootVertexAmount, f.vertexIndices.y + rootVertexAmount, f.vertexIndices.x + rootVertexAmount));

        if (f.color == 16)
            meshData->colors.push_back(color);
        else
            meshData->colors.push_back(f.color);
    }

    for (SubFile& s : currentFile->subFiles)
    {
        MergeRecursion(s.file, meshData, matToRoot* s.transform, s.bfcInvert != bfcInvert, s.color);
    }
}