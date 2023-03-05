#include "LDrawConverter.hpp"
#include "LDrawExporter.hpp"
#include "MeshData.hpp"

int main(int argc, char const *argv[])
{
    // ldraw library path
    LDrawConverter converter("LIBRARY_PATH/");
    
    LDrawFile* file = converter.ParseFile("IMPORT_PATH/FILE.ldr");
    std::cout << "MeshCount: " << converter.meshCount << std::endl;

    LDrawExporter exporter(&converter);
    exporter.Export(file, "C:/Users/Ole/Desktop/a/newXwing.fbx");

    delete file;
    return 0;
}
