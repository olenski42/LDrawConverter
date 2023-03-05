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
    

private:
    LDrawConverter* m_converter;
    FbxManager *m_sdkManager;
    FbxScene* m_scene;

    std::map<LDrawFile*, MeshData*> partMap;

    std::map<ColorID, FbxSurfacePhong*> m_materialMap;

    // matrix that transforms the scene upside down
    static glm::mat4 sceneTransform()
    {
        glm::mat4 mat = glm::mat4(1.0f);

        // rotate to match ldraw coordinate system
        mat = glm::rotate(mat, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        // apply scale
        //mat = glm::scale(mat, glm::vec3(1.0f, 1.0f, 1.0f));
        return mat;
    }

    struct MeshColorMapped
    {
        FbxMesh* mesh;
        std::vector<ColorID> materialMap;
    };

    void CopyFromLDraw(LDrawFile const *currentFile, MeshData *meshData, glm::mat4 matToRoot, bool bfcInvert, ColorID color);
    void MergeRecursion(LDrawFile* currentFile, MeshData* meshData, glm::mat4 matToRoot = sceneTransform(), bool bfcInvert = false, ColorID color = 16);
    MeshColorMapped CreateMesh(MeshData *meshData);
    FbxNode* CreateNode(LDrawFile* file);
    void LoadMaterials();
};