#include "LDrawExporter.hpp"
#include <vector>
#include "Common/Common.h"
#include "glm/glm.hpp"
#include <queue>

void ReduceMesh(MeshData *meshData)
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
                vertexMap[vertexIndex] = convSizetInt(vertices.size());
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

LDrawExporter::LDrawExporter(LDrawConverter *converter)
    : m_converter(converter)
{
    m_sdkManager = FbxManager::Create();
    m_scene = FbxScene::Create(m_sdkManager, "My Scene");
    InitializeSdkObjects(m_sdkManager, m_scene);
}

void LDrawExporter::Export(LDrawFile *file, const char *outPath)
{
    bool lResult;

    FbxNode *lRootNode = m_scene->GetRootNode();

    FbxNode *node = CreateNode(file);
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

void LDrawExporter::LoadMaterials()
{
    for(auto& material : m_converter->colorMap)
    {
        FbxSurfacePhong *fbxMaterial = FbxSurfacePhong::Create(m_scene, material.second.name.c_str());
        fbxMaterial->Emissive.Set(FbxDouble3(0, 0, 0));
        fbxMaterial->Ambient.Set(FbxDouble3(material.second.color.r, material.second.color.g, material.second.color.b));
        fbxMaterial->Diffuse.Set(FbxDouble3(material.second.color.r, material.second.color.g, material.second.color.b));
        fbxMaterial->Shininess.Set(0);
        fbxMaterial->ReflectionFactor.Set(0);
        fbxMaterial->TransparencyFactor.Set(0);
        fbxMaterial->ShadingModel.Set("Phong");
        fbxMaterial->MultiLayer.Set(false);
        fbxMaterial->EmissiveFactor.Set(0);
        fbxMaterial->AmbientFactor.Set(1);
        fbxMaterial->DiffuseFactor.Set(1);

        m_materialMap[material.first] = fbxMaterial;
    }
}

FbxNode *LDrawExporter::CreateNode(LDrawFile *ldrawFile)
{
    if(m_materialMap.size() == 0)
    {
        m_converter->LoadColorFile();
        LoadMaterials();
    }
    
    FbxNode *node = FbxNode::Create(m_scene, "unnamed");

    MeshData meshData;
    MergeRecursion(ldrawFile, &meshData);
    // ReduceMesh(&meshData);


    MeshColorMapped meshMapped = CreateMesh(&meshData);
    node->SetNodeAttribute(meshMapped.mesh);

    for (int i = 0; i < meshMapped.materialMap.size(); i++)
    {
        if(meshMapped.materialMap[i] == 16)
        {
            LogW("Material 16");
            node->AddMaterial(m_materialMap[0]);
        }
        auto a = m_materialMap.find(meshMapped.materialMap[i]);
        if(a == m_materialMap.end())
        {
            LogE("Material not found");
        }
        else
        {
            node->AddMaterial(a->second);
        }
    }
    

    return node;
}

LDrawExporter::MeshColorMapped LDrawExporter::CreateMesh(MeshData *meshData)
{
    FbxMesh *mesh = FbxMesh::Create(m_scene, "Mesh");
    mesh->InitControlPoints(convSizetInt(meshData->vertices.size()));

    mesh->InitControlPoints(convSizetInt(meshData->vertices.size()));
    for (int i = 0; i < meshData->vertices.size(); ++i)
    {
        FbxVector4 vertex(meshData->vertices[i].x, meshData->vertices[i].y, meshData->vertices[i].z);
        mesh->SetControlPointAt(vertex, i);
    }

    // material mapping
    std::vector<ColorID> materialMap;

    FbxGeometryElementMaterial* lMaterialElement = mesh->CreateElementMaterial();
    lMaterialElement->SetMappingMode(FbxGeometryElement::eByPolygon);
    lMaterialElement->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

    // Add faces to the mesh
    for (int i = 0; i < meshData->faces.size(); ++i)
    {
        int colorIndex;
        for(colorIndex = 0; colorIndex < materialMap.size(); colorIndex++)
        {
            if (materialMap[colorIndex] == meshData->colors[i])
            {
                break;
            }
        }
        
        if (colorIndex == materialMap.size())
        {
            materialMap.push_back(meshData->colors[i]);
        }

        mesh->BeginPolygon(colorIndex);
        mesh->AddPolygon(meshData->faces[i].x);
        mesh->AddPolygon(meshData->faces[i].y);
        mesh->AddPolygon(meshData->faces[i].z);
        mesh->EndPolygon();
    }

    mesh->GenerateNormals();

    MeshColorMapped meshMapped;
    meshMapped.mesh = mesh;
    meshMapped.materialMap = materialMap;
    return meshMapped;
}

void CopyFromMesh(MeshData *meshDest, MeshData const *meshSource, glm::mat4 matToRoot, bool bfcInvert, bool bcfInvert, ColorID color)
{
    unsigned int rootVertexAmount = convSizetUint(meshDest->vertices.size());
    for (int i = 0; i < meshSource->vertices.size(); i++)
    {
        glm::vec4 vertex = glm::vec4(meshSource->vertices[i], 1.0f);
        vertex = matToRoot * vertex;

        meshDest->vertices.push_back(glm::vec3(vertex.x, vertex.y, vertex.z));
    }

    for (const glm::ivec3 &f : meshSource->faces)
    {
        // invert face direction
        if (bfcInvert == false)
            meshDest->faces.push_back(glm::ivec3(f.x + rootVertexAmount, f.y + rootVertexAmount, f.z + rootVertexAmount));
        else
            meshDest->faces.push_back(glm::ivec3(f.z + rootVertexAmount, f.y + rootVertexAmount, f.x + rootVertexAmount));
    }

    for (const ColorID &c : meshSource->colors)
    {
        if (c == 16)
            meshDest->colors.push_back(color);
        else
            meshDest->colors.push_back(c);
    }
}


void LDrawExporter::CopyFromLDraw(LDrawFile const *currentFile, MeshData *meshData, glm::mat4 matToRoot, bool bfcInvert, ColorID color)
{
    unsigned int rootVertexAmount = convSizetUint(meshData->vertices.size());
    for (int i = 0; i < currentFile->vertices.size(); i++)
    {
        glm::vec4 vertex = glm::vec4(currentFile->vertices[i], 1.0f);
        vertex = matToRoot * vertex;

        meshData->vertices.push_back(glm::vec3(vertex.x, vertex.y, vertex.z));
    }

    for (const Face &f : currentFile->faces)
    {
        // invert face direction
        if (bfcInvert == false)
            meshData->faces.push_back(glm::ivec3(f.vertexIndices.x + rootVertexAmount, f.vertexIndices.y + rootVertexAmount, f.vertexIndices.z + rootVertexAmount));
        else
            meshData->faces.push_back(glm::ivec3(f.vertexIndices.z + rootVertexAmount, f.vertexIndices.y + rootVertexAmount, f.vertexIndices.x + rootVertexAmount));

        if (f.color == 16)
            meshData->colors.push_back(color);
        else
            meshData->colors.push_back(f.color);
    }

    for (const SubFile &s : currentFile->subFiles)
    {
        MergeRecursion(s.file, meshData, matToRoot * s.transform, s.bfcInvert != bfcInvert, s.color);
    }
}

void LDrawExporter::MergeRecursion(LDrawFile *currentFile, MeshData *meshData, glm::mat4 matToRoot, bool bfcInvert, ColorID color)
{
    // only copy from LDraw file if it is not a part (or has not been cached yet)
    if (currentFile->fileType == FILETYPE_PART)
    {
        //get part from cache
        MeshData *mesh;

        // create new mesh if does not already exist
        if (partMap.find(currentFile) == partMap.end())
        {
            mesh = new MeshData();
            CopyFromLDraw(currentFile, mesh, glm::mat4(1.0f), bfcInvert, color); // calls MergeRecursion
        }

        // use existing mesh
        else
        {
            mesh = partMap[currentFile];
        }

        // merge mesh to root mesh
        CopyFromMesh(meshData, mesh, matToRoot, bfcInvert, bfcInvert, color);
    }

    // copy from LDraw file
    else
    {
        CopyFromLDraw(currentFile, meshData, matToRoot, bfcInvert, color); // calls MergeRecursion
    }
}