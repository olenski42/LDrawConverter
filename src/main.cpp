#include "LDrawConverter.hpp"
#include "LDrawExporter.hpp"
#include "MeshData.hpp"

int main(int argc, char const *argv[])
{
    // ldraw library path
    LDrawConverter converter("LIBRARY_PATH");
    
    LDrawFile file;
    converter.ParseFile(file, "IMPORT_PATH/FILE.ldr");
    converter.ResolveAll();
    std::cout << "MeshCount: " << converter.meshCount << std::endl;

    LDrawExporter exporter(&converter);
    MeshData mesh;
    exporter.Merge(file, &mesh);
    exporter.Export(&mesh, "EXPORT_PATH/MESH.fbx");

    return 0;
}
