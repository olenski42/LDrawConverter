#include "LDrawExporter.hpp"
#include <vector>
#include "Common/Common.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_decompose.hpp>
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
    m_converterManager = new FbxGeometryConverter(m_sdkManager);
}

void LDrawExporter::Export(LDrawFile *file, std::string outPath)
{
    if(m_materialMap.size() == 0)
    {
        m_converter->LoadColorFile();
        LoadMaterials();
    }

    bool lResult;
    FbxNode *lRootNode = m_scene->GetRootNode();
    FbxNode *node = ConvertFile(file);
    node->LclRotation.Set(FbxDouble3(0, 0, -180));
    lRootNode->AddChild(node);

    // Save the scene.
    LogI("Saving scene to \"" << outPath << "\"...");
    lResult = SaveScene(m_sdkManager, m_scene, outPath.c_str(), 0);

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
    LogI("Loading materials...");

    if(m_converter->colorMap.size() == 0)
    {
        LogE("No materials loaded!");
        return;
    }

    for (auto &material : m_converter->colorMap)
    {
        FbxSurfacePhong *fbxMaterial = FbxSurfacePhong::Create(m_scene, material.second.name.c_str());
        fbxMaterial->SetName(material.second.name.c_str());

        FbxDouble3 color = FbxDouble3(material.second.color.r, material.second.color.g, material.second.color.b);
        FbxDouble3 ambientColor(color[0] * 1.0, color[1] * 1.0, color[2] * 1.0);
        FbxDouble3 diffuseColor(color[0] * 1.0, color[1] * 1.0, color[2] * 1.0);
        FbxDouble3 specularColor(color[0] * 0.8, color[1] * 0.8, color[2] * 0.8);
        FbxDouble3 reflectionColor(color[0] * 0.2, color[1] * 0.2, color[2] * 0.2);

        float shininess = 20.0f;
        float specular = 0.2f;
        float reflection = 0.1f;

        if (material.second.transparency)
        {
            fbxMaterial->TransparentColor.Set(color);
            fbxMaterial->TransparencyFactor.Set(material.second.alpha);
        }
        if (material.second.glow)
        {
            fbxMaterial->Emissive.Set(color);
            fbxMaterial->EmissiveFactor.Set(material.second.luminance);
        }

        if (material.second.metallic)
        {
            shininess = 50.0f;
            specularColor = (color[0] * 0.5, color[1] * 0.5, color[2] * 0.5);
            specular = 0.2f;
            reflectionColor = (color[0] * 0.9, color[1] * 0.9, color[2] * 0.9);
            reflection = 0.25f;
        }
        else if (material.second.rubber)
        {
            shininess = 2.0f;
            specularColor = (color[0] * 0.01, color[1] * 0.01, color[2] * 0.01);
            specular = 0.01f;
            reflectionColor = (color[0] * 0.1, color[1] * 0.1, color[2] * 0.1);
            reflection = 0.02f;
        }
        else if (material.second.chrome)
        {
            shininess = 500.0f;
            specularColor = (color[0], color[1], color[2]);
            specular = 0.9f;
            reflectionColor = (color[0] * 0.9, color[1] * 0.9, color[2] * 0.9);
            reflection = 0.9f;
        }
        else if (material.second.pearl) // TODO: actual perl material
        {
            shininess = 200.0f;
            specularColor = (color[1], color[2], color[0]);
            specular = 0.9f;
            reflectionColor = (color[0] * 0.9, color[2] * 0.9, color[2] * 0.4);
            reflection = 0.9f;
        }
        else if (material.second.matteMetallic)
        {
            shininess = 50.0f;
            specularColor = (color[0] * 0.5, color[1] * 0.5, color[2] * 0.5);
            specular = 0.2f;
            reflectionColor = (color[0] * 0.9, color[1] * 0.9, color[2] * 0.9);
            reflection = 0.25f;
        }

        fbxMaterial->Ambient.Set(ambientColor);
        fbxMaterial->Diffuse.Set(diffuseColor);
        fbxMaterial->Specular.Set(specularColor);
        fbxMaterial->Reflection.Set(reflectionColor);

        fbxMaterial->Shininess.Set(shininess);
        fbxMaterial->SpecularFactor.Set(specular);
        fbxMaterial->ReflectionFactor.Set(reflection);

        m_materialMap[material.first] = fbxMaterial;
    }
}

FbxNode *LDrawExporter::ConvertFile(LDrawFile *ldrFile)
{
    FbxNode *node;
    if (mergeAll)
    {
        MeshData* rootMesh = CreateMeshDataFromLDraw(ldrFile);
        for(auto &subFile : ldrFile->subFiles)
        {
            AddSubFileToMeshDataRecursion(&subFile, rootMesh, {subFile.transform, subFile.bfcInvert, subFile.color == 16 ? 0 : subFile.color});
        }
        
        MeshColorMapped meshMapped = CreateMeshMappedFromMeshData(rootMesh);
        node = CreateNodeFromMeshMapped(&meshMapped);
    }
    else
    {
        LogE("Invalid merge settings!");
    }

    return node;
}

void LDrawExporter::AddSubFileToMeshDataRecursion(SubFile *thisInstance, MeshData *rootMeshData, MeshCarryInfo carryInfo)
{
    MergeLDrawIntoMeshData(rootMeshData, thisInstance->file, carryInfo);
    for (auto &subFile : thisInstance->file->subFiles)
    {
        AddSubFileToMeshDataRecursion(&subFile, rootMeshData, {carryInfo.matToRoot * subFile.transform, carryInfo.bfcInvert != subFile.bfcInvert, subFile.color == 16 ? carryInfo.color : subFile.color});
    }
}

void LDrawExporter::ConvertFileRecursion(SubFile *thisInstance, MeshData *parentMeshData, FbxNode *parentNode, MeshCarryInfo carryInfo)
{
    LogE("Not implemented yet!");
}

void LDrawExporter::MergeLDrawIntoMeshData(MeshData *meshDest, LDrawFile const *ldrawSource, MeshCarryInfo carryInfo)
{
    unsigned int rootVertexAmount = convSizetUint(meshDest->vertices.size());
    for (int i = 0; i < ldrawSource->vertices.size(); i++)
    {
        glm::vec4 vertex = glm::vec4(ldrawSource->vertices[i], 1.0f);
        vertex = carryInfo.matToRoot * vertex;

        meshDest->vertices.push_back(glm::vec3(vertex.x, vertex.y, vertex.z));
    }

    for (const Face &f : ldrawSource->faces)
    {
        // invert face direction
        if (carryInfo.bfcInvert == false)
            meshDest->faces.push_back(glm::ivec3(f.vertexIndices.x + rootVertexAmount, f.vertexIndices.y + rootVertexAmount, f.vertexIndices.z + rootVertexAmount));
        else
            meshDest->faces.push_back(glm::ivec3(f.vertexIndices.z + rootVertexAmount, f.vertexIndices.y + rootVertexAmount, f.vertexIndices.x + rootVertexAmount));

        if (f.color == 16)
            meshDest->colors.push_back(carryInfo.color);
        else
            meshDest->colors.push_back(f.color);
    }
}

void LDrawExporter::MergeMeshDataIntoMeshData(MeshData *meshDest, MeshData const *meshSource, MeshCarryInfo carryInfo)
{
    unsigned int rootVertexAmount = convSizetUint(meshDest->vertices.size());
    for (int i = 0; i < meshSource->vertices.size(); i++)
    {
        glm::vec4 vertex = glm::vec4(meshSource->vertices[i], 1.0f);
        vertex = carryInfo.matToRoot * vertex;

        meshDest->vertices.push_back(glm::vec3(vertex.x, vertex.y, vertex.z));
    }

    for (const glm::ivec3 &f : meshSource->faces)
    {
        // invert face direction
        if (carryInfo.bfcInvert == false)
            meshDest->faces.push_back(glm::ivec3(f.x + rootVertexAmount, f.y + rootVertexAmount, f.z + rootVertexAmount));
        else
            meshDest->faces.push_back(glm::ivec3(f.z + rootVertexAmount, f.y + rootVertexAmount, f.x + rootVertexAmount));
    }

    for (const ColorID &c : meshSource->colors)
    {
        if (c == 16)
            meshDest->colors.push_back(carryInfo.color);
        else
            meshDest->colors.push_back(c);
    }
}

LDrawExporter::MeshColorMapped LDrawExporter::CreateMeshMappedFromMeshData(MeshData const *meshSource)
{
    MeshColorMapped meshMapped = MeshColorMapped();
    meshMapped.mesh = FbxMesh::Create(m_scene, "");

    meshMapped.mesh->InitControlPoints(convSizetInt(meshSource->vertices.size()));

    for (int i = 0; i < meshSource->vertices.size(); ++i)
    {
        FbxVector4 vertex(meshSource->vertices[i].x, meshSource->vertices[i].y, meshSource->vertices[i].z);
        meshMapped.mesh->SetControlPointAt(vertex, i);
    }

    FbxGeometryElementMaterial *lMaterialElement = meshMapped.mesh->CreateElementMaterial();
    lMaterialElement->SetMappingMode(FbxGeometryElement::eByPolygon);
    lMaterialElement->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

    if (meshSource->faces.size() != meshSource->colors.size())
    {
        LogE("MeshData has different amount of faces and colors");
    }

    // Add faces to the mesh
    for (int i = 0; i < meshSource->faces.size(); i++)
    {
        int colorIndex;
        for (colorIndex = 0; colorIndex < meshMapped.materialMap.size(); colorIndex++)
        {
            if (meshMapped.materialMap[colorIndex] == meshSource->colors[i])
            {
                break;
            }
        }

        if (colorIndex == meshMapped.materialMap.size())
        {
            meshMapped.materialMap.push_back(meshSource->colors[i]);
        }

        meshMapped.mesh->BeginPolygon(colorIndex);
        meshMapped.mesh->AddPolygon(meshSource->faces[i].x);
        meshMapped.mesh->AddPolygon(meshSource->faces[i].y);
        meshMapped.mesh->AddPolygon(meshSource->faces[i].z);
        meshMapped.mesh->EndPolygon();
    }

    meshMapped.mesh->GenerateNormals();

    return meshMapped;
}

FbxNode *LDrawExporter::CreateNodeFromMeshMapped(LDrawExporter::MeshColorMapped const *meshMappedSource, std::string name, LDrawExporter::MeshCarryInfo carryInfo)
{
    FbxNode *node = FbxNode::Create(m_scene, name.c_str());
    node->SetNodeAttribute(meshMappedSource->mesh);

    // set Materials
    for (int i = 0; i < meshMappedSource->materialMap.size(); i++)
    {
        ColorID color = meshMappedSource->materialMap[i];

        if (color == 16)
            color = carryInfo.color;

        if (color == 16)
        {
            LogW("Material 16");
            node->AddMaterial(m_materialMap[0]);
            continue;
        }

        auto a = m_materialMap.find(color);
        if (a == m_materialMap.end())
        {
            LogW("Material " << meshMappedSource->materialMap[i] << " not found");
            node->AddMaterial(m_materialMap[0]);
        }
        else
        {
            node->AddMaterial(a->second);
        }
    }

    return node;
}

MeshData* LDrawExporter::CreateMeshDataFromLDraw(LDrawFile *ldrFile)
{
    MeshData *meshData = new MeshData();
    MergeLDrawIntoMeshData(meshData, ldrFile, MeshCarryInfo());
    return meshData;
}