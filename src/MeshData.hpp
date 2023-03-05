#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "MeshData.hpp"

struct MeshData
{
    std::vector<glm::vec3> vertices;
    std::vector<glm::ivec3> faces;
    // std::vector<glm::vec3> normals;
    // std::vector<glm::vec2> uvs;
    std::vector<ColorID> colors;
};
