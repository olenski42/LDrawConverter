#pragma once
#include <vector>
#include <string>
#include "glm/glm.hpp"

struct LDrawFile;
typedef unsigned int ColorID;

struct LDrawColor
{
    std::string name;
    glm::vec3 color;
    glm::vec3 edgeColor;
    float alpha;
    float luminance;

    bool transparency = false;
    bool glow = false;

    bool chrome = false;
    bool pearl = false;
    bool rubber = false;
    bool matteMetallic = false;
    bool metallic = false;
};

enum FileType
{
    FILETYPE_PRIMITIVE,
    FILETYPE_PART,
    FILETYPE_SUBMODEL,
    FILETYPE_MULTIPART,
    FILETYPE_AMOUNT // This is not a valid file type, it is used to get the amount of file types
};

struct UnresolvedFile
{
    LDrawFile* file;
    std::string fileName;
    FileType fileType;
};

struct SubFile
{
    LDrawFile* file;
    ColorID color;
    glm::mat4 transform = glm::mat4(1);
    bool bfcInvert;
};

struct Face
{
    ColorID color;
    glm::ivec3 vertexIndices;
};

struct LDrawFile
{
    std::string name;
    FileType fileType;
    std::vector<SubFile> subFiles;
    std::vector<Face> faces;
    std::vector<glm::vec3> vertices;
    bool CCW = true;
};