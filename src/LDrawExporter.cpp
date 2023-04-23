#include "LDrawExporter.hpp"
#include <vector>
#include "Common/Common.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/projection.hpp>
#include <queue>

inline void RemoveDublicateVertices(MeshData *meshData)
{
    std::vector<glm::vec3> vertices;
    std::vector<MeshFace> faces;

    std::vector<int> vertexMap(meshData->vertices.size(), -1);

    for (int i = 0; i < meshData->faces.size(); i++)
    {
        MeshFace face = meshData->faces[i];

        for (int j = 0; j < 3; j++)
        {
            int vertexIndex = face.vertexIndices[j];
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
            }
        }
        face.vertexIndices = glm::ivec3(vertexMap[face.vertexIndices.x], vertexMap[face.vertexIndices.y], vertexMap[face.vertexIndices.z]);

        faces.push_back(face);
    }

    LogI("Removed " << meshData->vertices.size() - vertices.size() << " dublicate vertices");
    meshData->vertices = vertices;
    meshData->faces = faces;
}

inline FbxAMatrix GetMatrix(glm::mat4 mat)
{
    glm::vec3 translation;
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(mat, scale, rotation, translation, skew, perspective);

    FbxVector4 fbxTranslation(translation.x, translation.y, translation.z);
    FbxVector4 fbxScale(scale.x, scale.y, scale.z);
    FbxQuaternion fbxRotation(rotation.x, rotation.y, rotation.z, rotation.w);

    return FbxAMatrix(fbxTranslation, fbxRotation, fbxScale);
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
    LogI("Initializing FBX SDK...");
    m_sdkManager = FbxManager::Create();
    m_geometryConverter = new FbxGeometryConverter(m_sdkManager);
}

FbxScene *LDrawExporter::LoadScene(LDrawFile *file)
{
    LogI("Creating scene...");
    m_scene = FbxScene::Create(m_sdkManager, "My Scene");
    InitializeSdkObjects(m_sdkManager, m_scene);

    if (m_materialMap.size() == 0)
    {
        m_converter->LoadColorFile();
        LoadMaterials();
    }

    LogI("Converting file...");
    glm::mat4 rootTransform = glm::mat4(1.0f);
    rootTransform = glm::rotate(rootTransform, glm::radians(180.0f), glm::vec3(0, 0, 1));
    SubFile rootSubFile = {file, 16, rootTransform, false};
    ConvertFile(&rootSubFile, m_scene->GetRootNode(), MeshCarryInfo());
    LogI("File converted!");

    return m_scene;
}

void LDrawExporter::Export(FbxScene *scene, std::string outPath)
{
    m_scene = scene;

    bool lResult;
    FbxNode *lRootNode = m_scene->GetRootNode();

    // Save the scene.
    LogI("Saving scene to \"" << outPath << "\"...");
    lResult = SaveScene(m_sdkManager, m_scene, outPath.c_str(), 0);

    if (lResult == false)
    {
        DestroySdkObjects(m_sdkManager, lResult);
        LogE("An error occurred while saving the scene!");
    }

    // Destroy all objects created by the FBX SDK.
    DestroySdkObjects(m_sdkManager, lResult);

    LogI("Scene saved!");
}

void LDrawExporter::LoadMaterials()
{
    LogI("Loading materials...");

    if (m_converter->colorMap.size() == 0)
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
        FbxDouble3 specularColor(color[0] * 1, color[1] * 1, color[2] * 1);
        FbxDouble3 reflectionColor(color[0] * 1, color[1] * 1, color[2] * 1);

        float shininess = 1.0f;
        float specular = 0.0f;
        float reflection = 0.0f;

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

void printChildrenMaterialAmount(FbxNode *node, int level = 0)
{
    for (int i = 0; i < node->GetChildCount(); i++)
    {
        FbxNode *n = node->GetChild(i);
        LogI(level << ": " << n->GetName() << " " << n->GetMaterialCount());
        printChildrenMaterialAmount(n, level + 1);
    }
}

void LDrawExporter::AddSubFileToMeshDataRecursion(SubFile *thisInstance, MeshData *rootMeshData, MeshCarryInfo carryInfo)
{
    if (addGaps && thisInstance->file->fileType == FILETYPE_PART)
        carryInfo.matToRoot = glm::scale(carryInfo.matToRoot, glm::vec3(partSize, partSize, partSize));

    // TODO multiply matrix with scaling value if scale parts is enabled
    MergeLDrawIntoMeshData(rootMeshData, thisInstance->file, carryInfo);
    for (auto &subFile : thisInstance->file->subFiles)
    {
        AddSubFileToMeshDataRecursion(&subFile, rootMeshData, {carryInfo.matToRoot * subFile.transform, carryInfo.bfcInvert != subFile.bfcInvert, subFile.color == 16 ? carryInfo.color : subFile.color});
    }
}

void LDrawExporter::ConvertFile(SubFile *thisInstance, FbxNode *parentNode, MeshCarryInfo carryInfo)
{
    FbxNode *node;
    FbxAMatrix mat = GetMatrix(thisInstance->transform);

    // resize node if it woud not be resized in sthe mesh data recursion
    if (addGaps && exportDepth != FILETYPE_PART && thisInstance->file->fileType == FILETYPE_PART)
        mat.SetS(mat.GetS() * partSize);

    if (thisInstance->file->fileType > exportDepth)
    {
        node = FbxNode::Create(m_scene, thisInstance->file->name.c_str());

        for (auto &subFile : thisInstance->file->subFiles)
        {
            ConvertFile(&subFile, node, {carryInfo.matToRoot * subFile.transform, carryInfo.bfcInvert != subFile.bfcInvert, subFile.color == 16 ? carryInfo.color : subFile.color});
        }

        node->LclTranslation.Set(mat.GetT());
        node->LclScaling.Set(mat.GetS());
        node->LclRotation.Set(mat.GetR());

        parentNode->AddChild(node);
    }
    else if (thisInstance->file->fileType == exportDepth)
    {
        MeshColorMapped meshMapped;
        if (cacheFiles[thisInstance->file->fileType])
        {
            auto it = m_meshMap.find(thisInstance->file);
            if (it != m_meshMap.end())
            {
                meshMapped = it->second;
            }
            else
            {
                MeshData rootMesh;

                AddSubFileToMeshDataRecursion(thisInstance, &rootMesh, {glm::mat4(1.0f), false, 16});
                meshMapped = CreateMeshMappedFromMeshData(&rootMesh);
                m_meshMap[thisInstance->file] = meshMapped;
            }
        }
        else
        {
            MeshData rootMesh;

            AddSubFileToMeshDataRecursion(thisInstance, &rootMesh, {glm::mat4(1.0f), false, 16});
            meshMapped = CreateMeshMappedFromMeshData(&rootMesh);
        }

        node = CreateNodeFromMeshMapped(&meshMapped, thisInstance->file->name.c_str(), carryInfo.color);
        node->LclTranslation.Set(mat.GetT());
        node->LclScaling.Set(mat.GetS());
        node->LclRotation.Set(mat.GetR());

        parentNode->AddChild(node);
    }
    else // if (thisInstance->file->fileType < exportDepth)
    {
        LogE("Error: exportDepth is too low");
    }
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
        MeshFace face;
        // invert face direction
        if (carryInfo.bfcInvert == false)
            face.vertexIndices = glm::ivec3(f.vertexIndices.x + rootVertexAmount, f.vertexIndices.y + rootVertexAmount, f.vertexIndices.z + rootVertexAmount);
        else
            face.vertexIndices = glm::ivec3(f.vertexIndices.z + rootVertexAmount, f.vertexIndices.y + rootVertexAmount, f.vertexIndices.x + rootVertexAmount);

        if (f.color == 16)
            face.color = carryInfo.color;
        else
            face.color = f.color;

        meshDest->faces.push_back(face);
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

    for (const MeshFace &f : meshSource->faces)
    {
        MeshFace face;

        // invert face direction
        if (carryInfo.bfcInvert == false)
            face.vertexIndices = glm::ivec3(f.vertexIndices.x + rootVertexAmount, f.vertexIndices.y + rootVertexAmount, f.vertexIndices.z + rootVertexAmount);
        else
            face.vertexIndices = glm::ivec3(f.vertexIndices.z + rootVertexAmount, f.vertexIndices.y + rootVertexAmount, f.vertexIndices.x + rootVertexAmount);

        if (f.color == 16)
            face.color = carryInfo.color;
        else
            face.color = f.color;
    }
}

LDrawExporter::MeshColorMapped LDrawExporter::CreateMeshMappedFromMeshData(MeshData const *meshSource)
{
    MeshColorMapped meshMapped = MeshColorMapped();
    meshMapped.mesh = FbxMesh::Create(m_scene, "");
    auto lLayer = meshMapped.mesh->GetLayer(0);
    if (lLayer == nullptr)
    {
        meshMapped.mesh->CreateLayer();
        lLayer = meshMapped.mesh->GetLayer(0);
    }

    meshMapped.mesh->InitControlPoints(convSizetInt(meshSource->vertices.size()));

    for (int i = 0; i < meshSource->vertices.size(); ++i)
    {
        FbxVector4 vertex(meshSource->vertices[i].x, meshSource->vertices[i].y, meshSource->vertices[i].z);
        meshMapped.mesh->SetControlPointAt(vertex, i);
    }

    // Set material indices
    FbxLayerElementMaterial *lMaterialLayer = FbxLayerElementMaterial::Create(meshMapped.mesh, "MaterialIndices");
    lMaterialLayer->SetMappingMode(FbxLayerElement::eByPolygon);
    lMaterialLayer->SetReferenceMode(FbxLayerElement::eIndexToDirect);
    lLayer->SetMaterials(lMaterialLayer);

    // Add faces to the mesh
    for (int i = 0; i < meshSource->faces.size(); i++)
    {
        int colorIndex;
        for (colorIndex = 0; colorIndex < meshMapped.materialMap.size(); colorIndex++)
        {
            if (meshMapped.materialMap[colorIndex] == meshSource->faces[i].color)
            {
                break;
            }
        }

        if (colorIndex == meshMapped.materialMap.size())
        {
            meshMapped.materialMap.push_back(meshSource->faces[i].color);
        }

        meshMapped.mesh->BeginPolygon(colorIndex);
        meshMapped.mesh->AddPolygon(meshSource->faces[i].vertexIndices.x);
        meshMapped.mesh->AddPolygon(meshSource->faces[i].vertexIndices.y);
        meshMapped.mesh->AddPolygon(meshSource->faces[i].vertexIndices.z);
        meshMapped.mesh->EndPolygon();
    }

    meshMapped.mesh->GenerateNormals();

    return meshMapped;
}

FbxNode *LDrawExporter::CreateNodeFromMeshMapped(LDrawExporter::MeshColorMapped const *meshMappedSource, std::string name, ColorID carryColor)
{
    FbxNode *node = FbxNode::Create(m_scene, name.c_str());
    node->SetShadingMode(FbxNode::eTextureShading);
    node->SetNodeAttribute(meshMappedSource->mesh);

    // set Materials
    for (int i = 0; i < meshMappedSource->materialMap.size(); i++)
    {
        ColorID color = meshMappedSource->materialMap[i];

        if (color == 16)
        {
            color = carryColor;
        }

        if (color == 16)
        {
            LogW("Material 16");
            node->AddMaterial(m_materialMap[15]);
            continue;
        }

        auto a = m_materialMap.find(color);
        if (a == m_materialMap.end())
        {
            LogW("Material " << color << " not found");
            node->AddMaterial(m_materialMap[15]);
        }
        else
        {
            node->AddMaterial(a->second);
        }
    }

    return node;
}