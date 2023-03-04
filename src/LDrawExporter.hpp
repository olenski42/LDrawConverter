#pragma once
#include "LDrawFile.hpp"
#include <fbxsdk.h>
#include "LDrawConverter.hpp"
#include "MeshData.hpp"

class LDrawExporter
{
public:
    LDrawExporter(LDrawConverter* pConverter)
        : converter(pConverter) {}
    void Export(MeshData *meshData, const char* outPath);

    void Merge(LDrawFile& currentFile, MeshData* meshData, glm::mat4 transform = glm::mat4(1));

private:
    LDrawConverter* converter = NULL;

    void CreateMesh(FbxScene *pScene, MeshData *ldrawFile);
    void MergeRecursion(LDrawFile& currentFile, MeshData* meshData, glm::mat4 matToRoot, bool bfcInvert, ColorID color);
};