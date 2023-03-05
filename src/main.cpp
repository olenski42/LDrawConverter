#include "LDrawConverter.hpp"
#include "LDrawExporter.hpp"
#include "MeshData.hpp"

int main(int argc, char const *argv[])
{
    // ldraw library path
    LDrawConverter converter("C:/Users/Ole/Documents/Assets/Lego/ldraw/");
    
    LDrawFile* file = converter.ParseFile("C:/Users/Ole/Desktop/L/X-Wing.ldr");
    // LDrawFile* file = converter.ParseFile("C:/Users/Ole/Desktop/L/2x2.ldr");
    std::cout << "MeshCount: " << converter.meshCount << std::endl;

    LDrawExporter exporter(&converter);
    //exporter.LoadParts();

    exporter.Export(file, "C:/Users/Ole/Desktop/a/newXwing.fbx");

    delete file;
    return 0;
}
