#pragma once
#include <glm/glm.hpp>
#include <vector>

typedef uint32_t ColorID;

struct MeshFace
{
    glm::ivec3 vertexIndices;
    ColorID color;
};

struct MeshData
{
    std::vector<glm::vec3> vertices;
    std::vector<MeshFace> faces;
};
