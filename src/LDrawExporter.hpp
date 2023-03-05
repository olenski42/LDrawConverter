#pragma once
#include "LDrawFile.hpp"
#include <fbxsdk.h>
#include "LDrawConverter.hpp"
#include "MeshData.hpp"

class LDrawExporter
{
public:
    LDrawExporter(LDrawConverter* converter);
    
    // LoadParts() before exporting & before converting new files (convert all files before exporting)
    void Export(LDrawFile* file, const char* outPath);
    
    void LoadParts();

private:
    LDrawConverter* m_converter;
    FbxManager *m_sdkManager;
    FbxScene* m_scene;

    std::map<ColorID, FbxSurfacePhong*> materialMap;
    std::map<LDrawFile*, FbxMesh*> partMap;

    void MergeRecursion(LDrawFile* currentFile, MeshData* meshData, glm::mat4 matToRoot = glm::mat4(1.0f), bool bfcInvert = false, ColorID color = 16);
    FbxMesh* CreateMesh(MeshData *meshData);
    FbxNode* CreateNode(LDrawFile* file);
};