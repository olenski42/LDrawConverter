#include "LDrawConverter.hpp"
#include "LDrawExporter.hpp"
#include "MeshData.hpp"
#include "Config.hpp"
#include "portable-file-dialogs.h"
#include <string>

int main(int argc, char const *argv[])
{
    Config config = Config();

    // ldraw library path
    LDrawConverter converter(config.ldrawLibraryPath.c_str());

    LogI("Select a file to convert");
    pfd::open_file fileDialog("Select LDraw file", ".", {"LDraw files (.ldr .mpd .dat)", "*.ldr *.mpd *.dat", "All files", "*"});
    if (fileDialog.result().size() == 0)
    {
        LogI("No file selected");
        return 0;
    }
    std::string filePath = fileDialog.result()[0];
    LDrawFile *file = converter.ParseFile(filePath);

    LDrawExporter exporter(&converter);
    LogI("Select a file to save the FBX to");
    auto pfdSave = pfd::save_file("Save FBX", ".", {"FBX files", "*.fbx", "All files", "*"}, pfd::opt::force_overwrite);
    FbxScene *scene = exporter.LoadScene(file);
    std::string outPath = pfdSave.result();
    if (outPath.empty())
    {
        LogI("No file selected");
        return 0;
    }
    exporter.Export(scene, outPath);

    delete file;
    return 0;
}
