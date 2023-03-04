#pragma once
#include <string>
#include <vector>
#include <map>
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "Log.hpp"
#include "LDrawFile.hpp"


inline unsigned int convSizetUint(size_t size);


struct UnresolvedFile
{
    FileID fileID;
    std::string fileName;
    FileType fileType; // The file type it was requested from
};


class LDrawConverter
{
public:
    LDrawConverter(const char* libraryPath);

    std::map<std::string, FileID> fileIDs;
    std::vector<UnresolvedFile> unresolvedFiles;
    std::map<FileID, LDrawFile> fileCache;


    /* @param file the file it should fill with data
    *  @param filePath the absolute filepath
    *  @return whether the file could be parsed. Only use the file if this returns true!
    */
    bool ParseFile(LDrawFile& file, std::string filePath, FileType fileType = FILETYPE_MULTIPART);
    void ResolveAll();
    std::ifstream FindFile(std::string file, FileType FileType);
    FileID GetFileID(std::string fileName, FileType parentType);

    void LoadColorFile();
    std::map<ColorID, LDrawColor> colorMap;

    unsigned int meshCount = 0;
    unsigned int unresolvedFileAmnt = 0;

private:
    bool ParseFile(LDrawFile& file, std::ifstream& fileStream, FileType fileType = FILETYPE_MULTIPART);
    
    std::string libPath;
};