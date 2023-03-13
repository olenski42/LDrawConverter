#pragma once
#include "LDrawFile.hpp"
#include <fbxsdk.h>
#include "LDrawConverter.hpp"
#include "MeshData.hpp"

class LDrawExporter
{
public:
    LDrawExporter(LDrawConverter *converter);
    void Export(LDrawFile *file, std::string outPath);

private:
    struct MeshCarryInfo
    {
        glm::mat4 matToRoot = glm::mat4(1);
        bool bfcInvert = false;
        ColorID color = 16;
    };

    struct MeshColorMapped
    {
        FbxMesh *mesh;
        std::vector<ColorID> materialMap;
    };

private:
    LDrawConverter *m_converter;
    FbxManager *m_sdkManager;
    FbxScene *m_scene;
    FbxGeometryConverter *m_geometryConverter;

    // call only if FbxMaterials have not been loaded yet (m_materialMap is empty)
    void LoadMaterials();


    // converts LDraw colors to FbxMaterials
    std::map<ColorID, FbxSurfacePhong *> m_materialMap;
    // cache of Meshes that have already been converted
    std::map<LDrawFile *, MeshColorMapped> m_meshMap;


    void AddSubFileToMeshDataRecursion(SubFile *thisInstance, MeshData *rootMeshData, MeshCarryInfo carryInfo);
    void ConvertFile(SubFile *thisInstance, FbxNode *parentNode, MeshCarryInfo carryInfo);

    // merge settings
    // what the deepest file type to turn into a mesh is
    FileType exportDepth = FILETYPE_SUBMODEL;
    // which file types to cache, export depth must be at least as high to cache it
    bool cacheFiles[FILETYPE_AMOUNT] = { false, true, false, false }; // primitive, part, subpart, multipart
    bool addGaps = false;
    float partSize = 0.99f;

    // helper functions
    inline void MergeLDrawIntoMeshData(MeshData *meshDest, LDrawFile const *ldrawSource, MeshCarryInfo carryInfo);
    inline void MergeMeshDataIntoMeshData(MeshData *meshDest, MeshData const *meshSource, MeshCarryInfo carryInfo);
    inline MeshColorMapped CreateMeshMappedFromMeshData(MeshData const *meshSource);
    inline MeshColorMapped CreateMeshMappedFromLDraw(LDrawFile const *ldrawSource, LDrawExporter::MeshCarryInfo carryInfo);
    inline FbxNode *CreateNodeFromMeshMapped(MeshColorMapped const *meshMappedSource, std::string name, ColorID carryColor);
    //inline FbxNode *CreateNodeFromLDraw(LDrawFile const *ldrawSource, std::string name, LDrawExporter::MeshCarryInfo carryInfo);
};