#pragma once
#include <vector>
#include <string>
#include "glm/glm.hpp"

typedef unsigned int FileID;
typedef unsigned int ColorID;

struct LDrawColor
{
    std::string name;
    glm::vec3 color;
    glm::vec3 edgeColor;
};


enum FileType
{
    FILETYPE_PRIMITIVE,
    FILETYPE_PART,
    FILETYPE_MULTIPART
};

struct SubFile
{
    glm::mat4 transform = glm::mat4(1);
    FileID id;
    bool bfcInvert;
    ColorID color;
};

struct LDrawFile
{
    std::vector<SubFile> subFiles;
    std::vector<glm::vec3> vertices;
    std::vector<glm::ivec3> faces;
    std::vector<ColorID> colors;
    bool CCW = true;
};