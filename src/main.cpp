#include "LDrawConverter.hpp"
#include "LDrawExporter.hpp"
#include "MeshData.hpp"
#include "portable-file-dialogs.h"
#include <string>

int main(int argc, char const *argv[])
{
    // ldraw library path
    LDrawConverter converter("LDRAWLIBRARY_PATH/");

    pfd::open_file fileDialog("Select LDraw file", ".", {"LDraw files (.ldr .mpd)", "*.ldr *.mpd", "All files", "*"});
    if (fileDialog.result().size() == 0)
        return 0;

    std::string filePath = fileDialog.result()[0];
    LDrawFile *file = converter.ParseFile(filePath);

    LDrawExporter exporter(&converter);
    auto pfdSave = pfd::save_file("Save FBX", ".", {"FBX files", "*.fbx", "All files", "*"});
    FbxScene* scene = exporter.LoadScene(file);
    std::string outPath = pfdSave.result();
    exporter.Export(scene, outPath);
    

    delete file;
    return 0;
}
