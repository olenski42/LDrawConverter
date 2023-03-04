#include "LDrawExporter.hpp"
#include <vector>
#include "Common/Common.h"
#include "glm/glm.hpp"

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

void LDrawExporter::CreateMesh(FbxScene *pScene, MeshData *meshData)
{
    const char *pName = "Triangle";
    FbxNode* lRootNode = pScene->GetRootNode();
    std::vector<glm::ivec3>* faces = &meshData->faces;


    FbxMesh *lMesh = FbxMesh::Create(pScene, pName);
    lMesh->InitControlPoints(convSizetInt(meshData->vertices.size()));
    FbxVector4 *lControlPoints = lMesh->GetControlPoints();

    for (size_t i = 0; i < meshData->vertices.size(); i++)
    {
        lControlPoints[i] = FbxVector4(meshData->vertices.at(i).x, meshData->vertices.at(i).y, meshData->vertices.at(i).z);
    }

    FbxGeometryElementMaterial* lMaterialElement = lMesh->CreateElementMaterial();
    lMaterialElement->SetMappingMode(FbxGeometryElement::eByPolygon);
    lMaterialElement->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

    for (int i = 0; i < meshData->faces.size(); i++)
    {
        lMesh->BeginPolygon(-1);
        lMesh->AddPolygon(faces->at(i).x);
        lMesh->AddPolygon(faces->at(i).y);
        lMesh->AddPolygon(faces->at(i).z);
        lMesh->EndPolygon();
    }
    lMesh->GenerateNormals();

    FbxNode *lNode = FbxNode::Create(pScene, pName);
    lNode->SetNodeAttribute(lMesh);

    //add materials using the ldraw color palette
    for(auto color : converter->colorMap)
    {
        //create material
        FbxSurfacePhong *lMaterial = FbxSurfacePhong::Create(pScene, color.second.name.c_str());
        lMaterial->Emissive.Set(FbxDouble3(0, 0, 0));
        lMaterial->Ambient.Set(FbxDouble3(color.second.color.x, color.second.color.y, color.second.color.z));
        lMaterial->Diffuse.Set(FbxDouble3(color.second.color.x, color.second.color.y, color.second.color.z));
        lMaterial->Specular.Set(FbxDouble3(0.5, 0.5, 0.5));
        lMaterial->Shininess.Set(0.5);
        lMaterial->ReflectionFactor.Set(0.5);
        lMaterial->TransparencyFactor.Set(0);
        lMaterial->ShadingModel.Set("Phong");
        lMaterial->MultiLayer.Set(false);
    }

    lRootNode->AddChild(lNode);
}

void LDrawExporter::Export(MeshData *meshData, const char* outPath)
{
    FbxManager *lSdkManager = NULL;
    FbxScene *lScene = NULL;
    bool lResult;

    // Prepare the FBX SDK.
    InitializeSdkObjects(lSdkManager, lScene);

    CreateMesh(lScene, meshData);

    // Save the scene.
    lResult = SaveScene(lSdkManager, lScene, outPath, 0);

    if (lResult == false)
    {
        FBXSDK_printf("\n\nAn error occurred while saving the scene...\n");
        DestroySdkObjects(lSdkManager, lResult);
        exit(-1);
    }

    // Destroy all objects created by the FBX SDK.
    DestroySdkObjects(lSdkManager, lResult);
}

void LDrawExporter::Merge(LDrawFile& currentFile, MeshData* meshData, glm::mat4 transform)
{
    for (SubFile s : currentFile.subFiles)
    {
        auto itSubFile = converter->fileCache.find(s.id);
        if (itSubFile != converter->fileCache.end())
        {
            MergeRecursion(itSubFile->second, meshData, transform* s.transform, s.bfcInvert, s.color);
        }
        else
            LogW("missing file during merge");
    }
}

void LDrawExporter::MergeRecursion(LDrawFile& currentFile, MeshData* meshData, glm::mat4 matToRoot, bool bfcInvert, ColorID color)
{
    unsigned int rootVertexAmount = convSizetUint(meshData->vertices.size());
    for (glm::vec3 v : currentFile.vertices)
        meshData->vertices.push_back(matToRoot * glm::vec4(v, 1.0f));
    for (glm::ivec3 f : currentFile.faces)
    {
        //invert face direction
        if (bfcInvert == false)
            meshData->faces.push_back(glm::ivec3(f.x + rootVertexAmount, f.y + rootVertexAmount, f.z + rootVertexAmount));
        else
            meshData->faces.push_back(glm::ivec3(f.x + rootVertexAmount, f.z + rootVertexAmount, f.y + rootVertexAmount));
    }
    // for (glm::vec3 n : currentFile.normals)
    //     meshData->normals.push_back(matToRoot * glm::vec4(n, 0.0f));
    // for (glm::vec2 uv : currentFile.uvs)
    //     meshData->uvs.push_back(uv);
    for (ColorID c : currentFile.colors)
    {
        if (c == 16)
            meshData->colors.push_back(converter->colorMap[color].color);
        else
            meshData->colors.push_back(converter->colorMap[c].color);
    }
    for (SubFile s : currentFile.subFiles)
    {
        auto itSubFile = converter->fileCache.find(s.id);
        if (itSubFile != converter->fileCache.end())
            MergeRecursion(itSubFile->second, meshData, matToRoot* s.transform, s.bfcInvert, s.color);
        else
            LogW("missing file during merge");
    }
}