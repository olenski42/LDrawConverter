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
    FbxGeometryConverter *m_converterManager;

    // call only if FbxMaterials have not been loaded yet (m_materialMap is empty)
    void LoadMaterials();


    // converts LDraw colors to FbxMaterials
    std::map<ColorID, FbxSurfacePhong *> m_materialMap;
    // cache of Meshes that have already been converted
    std::map<LDrawFile *, MeshColorMapped> m_meshMap;

    /**
     * @param ldrFile The file to convert
     */
    FbxNode *ConvertFile(LDrawFile *ldrFile);
    /**
     * @param thisInstance The file to convert
     * @param parentMeshData The mesh data of the parent file, adds current- and sub-meshes to it if mergeAll is true, or to parts if cachePartData is true
     * @param parentNode The parent node of the file, adds the current node if mergeAll is false
     * @param carryColor The color to carry to the next file
     * @param carryTransform The transform to carry to the next file
     * @param carryInvert The bfc invert to carry to the next file
     */
    void ConvertFileRecursion(SubFile *thisInstance, MeshData *parentMeshData, FbxNode *parentNode, MeshCarryInfo carryInfo);
    void AddSubFileToMeshDataRecursion(SubFile *thisInstance, MeshData *rootMeshData, MeshCarryInfo carryInfo);

    // merge settings
    bool mergeAll = true;
    bool cachePartData = false;
    bool cachePrimitiveData = false;
    bool cachePartMeshMaps = false;

    // helper functions
    inline void MergeLDrawIntoMeshData(MeshData *meshDest, LDrawFile const *ldrawSource, MeshCarryInfo carryInfo);
    inline void MergeMeshDataIntoMeshData(MeshData *meshDest, MeshData const *meshSource, MeshCarryInfo carryInfo);
    inline MeshColorMapped CreateMeshMappedFromMeshData(MeshData const *meshSource);
    inline FbxNode *CreateNodeFromMeshMapped(MeshColorMapped const *meshMappedSource, std::string name = "unnamed", LDrawExporter::MeshCarryInfo carryInfo = MeshCarryInfo());
    inline MeshData* CreateMeshDataFromLDraw(LDrawFile *ldrFile);
};