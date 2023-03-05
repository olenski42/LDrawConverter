#pragma once
#include <string>
#include <vector>
#include <map>
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "Log.hpp"
#include "LDrawFile.hpp"


inline unsigned int convSizetUint(size_t size);
inline int convSizetInt(size_t size);


class LDrawConverter
{
public:
    LDrawConverter(const char* libraryPath);

    std::map<std::string, LDrawFile*> nameResolver;
    std::vector<UnresolvedFile> unresolvedFiles;


    /* @param file the file it should fill with data
    *  @param filePath the absolute filepath
    *  @return whether the file could be parsed. Only use the file if this returns true!
    */
    LDrawFile* ParseFile(std::string filePath);
    std::ifstream FindFile(UnresolvedFile* file);
    LDrawFile* GetFile(std::string fileName, FileType fileType);

    void LoadColorFile();
    std::map<ColorID, LDrawColor> colorMap;

    unsigned int meshCount = 0;

private:
    void ResolveAll();
    bool ParseFile(LDrawFile* file, std::ifstream& fileStream, FileType parentType = FILETYPE_MULTIPART);
    
    std::string libPath;
};