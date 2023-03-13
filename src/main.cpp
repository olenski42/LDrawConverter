#include "LDrawConverter.hpp"
#include "LDrawExporter.hpp"
#include "MeshData.hpp"
#include "portable-file-dialogs.h"
#include <string>

int main(int argc, char const *argv[])
{
    // ldraw library path
    LDrawConverter converter("C:/Program Files/Studio 2.0/ldraw/");

    pfd::open_file fileDialog("Select LDraw file", ".", {"LDraw files (.ldr .mpd)", "*.ldr *.mpd", "All files", "*"});
    if (fileDialog.result().size() == 0)
        return 0;

    std::string filePath = fileDialog.result()[0];
    LDrawFile *file = converter.ParseFile(filePath);

    std::cout << "MeshCount: " << converter.meshCount << std::endl;

    LDrawExporter exporter(&converter);

    
    std::string outPath = pfd::save_file("Save FBX", ".", {"FBX files", "*.fbx", "All files", "*"}).result();
    exporter.Export(file, outPath);
    

    delete file;
    return 0;
}
