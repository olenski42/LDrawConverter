#include "LDrawConverter.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>


LDrawConverter::LDrawConverter(const char *libraryPath)
    : libPath(libraryPath)
{
}

inline unsigned int convSizetUint(size_t size)
{
    if (size > UINT_MAX)
    {
        throw std::length_error(std::string("invalid conversion from size_t to unsigend int (size: %z)", size));
    }
    return static_cast<unsigned int>(size);
}

inline int convSizetInt(size_t size)
{
    if (size > INT_MAX)
    {
        throw std::length_error(std::string("invalid conversion from size_t to int (size: %z)", size));
    }
    return static_cast<int>(size);
}

inline FileType nextFileType(FileType type)
{
    return FileType(type > 0 ? type - 1 : 0);
}

inline std::vector<std::string> parseLine(std::ifstream &fileStream)
{
    std::vector<std::string> parsedLine;

    std::string line;
    std::getline(fileStream, line);

    std::stringstream sstrm(line);

    for (std::string element; std::getline(sstrm, element, ' ');)
    {
        if (element != "")
            parsedLine.emplace_back(element);
    }

    return parsedLine;
}

FileID LDrawConverter::GetFileID(std::string path, FileType parentType)
{
    FileID id;

    // replaces "\" with "/" for relative paths e.g. s\3005s01.dat to s/3005s01.dat (which is not a primitive)
    bool relativePath = false;
    for (auto it = path.begin(); it != path.end(); it++)
    {
        if (char(92) == *it)
        {
            *it = '/';
            relativePath = true;
        }
    }

    auto itToFile = fileIDs.find(path);
    if (itToFile == fileIDs.end())
    {
        id = convSizetUint(fileIDs.size());
        fileIDs.emplace(path, id);

        FileType fileType = relativePath ? parentType : nextFileType(parentType);
        unresolvedFiles.push_back(UnresolvedFile{id, path, fileType});
    }
    else
        id = itToFile->second;

    return id;
}

bool LDrawConverter::ParseFile(LDrawFile &file, std::string filePath, FileType fileType)
{
    std::ifstream fileStream;
    fileStream.open(filePath);

    if (!fileStream.is_open())
    {
        LogE("file \"" << filePath << "\" could not be opened!");
    }

    return ParseFile(file, fileStream, fileType);
}

bool LDrawConverter::ParseFile(LDrawFile &file, std::ifstream &fileStream, FileType fileType)
{
    bool invertNext = false;
    bool CCW = true;
    bool MPD = false;

    LDrawFile *currentFile = &file;

    while (fileStream.good())
    {
        // create vector of elements in the line
        std::vector<std::string> line = parseLine(fileStream);

        if (line.size() >= 3 && line[0] == "0")
        {
            if (line[1] == "BFC")
            {
                if (line.size() == 3)
                {
                    if (line[2] == "INVERTNEXT")
                    {
                        invertNext = true;
                        continue;
                    }
                    else if (line[2] == "NOCLIP")
                    {
                        LogE("noclip is not supported!");
                    }
                    else if (line[2] == "CW")
                        CCW = false;
                    else if (line[2] == "CCW")
                        CCW = true;
                }
                else if (line.size() == 4)
                {
                    if (line[3] == "CLIP")
                    {
                        LogE("clipping settings are not supported!");
                    }
                    else if (line[3] == "CW")
                        CCW = false;
                    else if (line[3] == "CCW")
                        CCW = true;
                }
            }

            else if (line[1] == "FILE")
            {
                std::string currentFileName;
                for (int i = 2; i < line.size(); i++)
                {
                    currentFileName.append(line[i]);
                    if (i < line.size() - 1)
                        currentFileName.append(" ");
                }

                for (char &c : currentFileName)
                {
                    if (c <= 'Z' && c >= 'A')
                        c -= ('Z' - 'z');
                }

                FileID currentFileID = GetFileID(currentFileName, FILETYPE_MULTIPART);

                if (MPD == true)
                    currentFile = &fileCache[currentFileID];
                else
                    MPD = true;

                for (std::vector<UnresolvedFile>::iterator it = unresolvedFiles.begin(); it != unresolvedFiles.end();)
                {
                    if (it->fileID == currentFileID)
                        it = unresolvedFiles.erase(it);
                    else
                        it++;
                }
            }
        }

        else if (line.size() >= 15 && line[0] == "1")
        {
            glm::mat4 transform;

            transform[3].x = stof(line[2]);
            transform[3].y = stof(line[3]);
            transform[3].z = stof(line[4]);

            transform[0].x = stof(line[5]);
            transform[1].x = stof(line[6]);
            transform[2].x = stof(line[7]);

            transform[0].y = stof(line[8]);
            transform[1].y = stof(line[9]);
            transform[2].y = stof(line[10]);

            transform[0].z = stof(line[11]);
            transform[1].z = stof(line[12]);
            transform[2].z = stof(line[13]);

            transform[0].w = 0;
            transform[1].w = 0;
            transform[2].w = 0;
            transform[3].w = 1;

            std::string subFileName;
            for (int i = 14; i < line.size(); i++)
            {
                subFileName.append(line[i]);
                if (i < line.size() - 1)
                    subFileName.append(" ");
            }

            meshCount++;
            currentFile->subFiles.emplace_back(SubFile{transform, GetFileID(subFileName, fileType), invertNext != (glm::determinant(transform) < 0), stoul(line[1])});
        }

        else if (line.size() == 11 && line[0] == "3")
        {
            currentFile->vertices.push_back(glm::vec3{stof(line[2])});
            currentFile->vertices.back().y = stof(line[3]);
            currentFile->vertices.back().z = stof(line[4]);

            currentFile->vertices.push_back(glm::vec3{stof(line[5])});
            currentFile->vertices.back().y = stof(line[6]);
            currentFile->vertices.back().z = stof(line[7]);

            currentFile->vertices.push_back(glm::vec3{stof(line[8])});
            currentFile->vertices.back().y = stof(line[9]);
            currentFile->vertices.back().z = stof(line[10]);

            currentFile->colors.push_back(stoul(line[1]));
            currentFile->colors.push_back(stoul(line[1]));
            currentFile->colors.push_back(stoul(line[1]));

            unsigned int index = convSizetUint(currentFile->vertices.size()) - 1;

            if (CCW)
                currentFile->faces.emplace_back(index, index - 1, index - 2);
            else
                currentFile->faces.emplace_back(index - 2, index - 1, index);
        }

        else if (line.size() == 14 && line[0] == "4")
        {
            currentFile->vertices.push_back(glm::vec3{stof(line[2])});
            currentFile->vertices.back().y = stof(line[3]);
            currentFile->vertices.back().z = stof(line[4]);

            currentFile->vertices.push_back(glm::vec3{stof(line[5])});
            currentFile->vertices.back().y = stof(line[6]);
            currentFile->vertices.back().z = stof(line[7]);

            currentFile->vertices.push_back(glm::vec3{stof(line[8])});
            currentFile->vertices.back().y = stof(line[9]);
            currentFile->vertices.back().z = stof(line[10]);

            currentFile->vertices.push_back(glm::vec3{stof(line[11])});
            currentFile->vertices.back().y = stof(line[12]);
            currentFile->vertices.back().z = stof(line[13]);

            currentFile->colors.push_back(stoul(line[1]));
            currentFile->colors.push_back(stoul(line[1]));
            currentFile->colors.push_back(stoul(line[1]));
            currentFile->colors.push_back(stoul(line[1]));

            unsigned int index = convSizetUint(currentFile->vertices.size()) - 1;
            if (CCW)
            {
                currentFile->faces.emplace_back(index - 3, index, index - 2);
                currentFile->faces.emplace_back(index, index - 1, index - 2);
            }
            else
            {
                currentFile->faces.emplace_back(index - 2, index, index - 3);
                currentFile->faces.emplace_back(index - 2, index - 1, index);
            }
        }

        invertNext = false;
    }

    fileStream.close();
    return true;
}

std::ifstream LDrawConverter::FindFile(std::string file, FileType type)
{
    std::string tmp = libPath + (type == FILETYPE_PRIMITIVE ? "p/" : "parts/") + file;
    std::ifstream stream(tmp.c_str());
    if (!stream.is_open())
    {
        LogW("file \"" << tmp << "\" not found, extending search");
        tmp = libPath + (type == FILETYPE_PRIMITIVE ? "parts/" : "p/") + file;
        stream.open(tmp);

        if (!stream.is_open())
        {
            LogW("file \"" << tmp << "\" not found, extending search");
            stream.open(file);
            if (!stream.is_open())
            {
                LogW("file \"" << tmp << "\" not found!");
                unresolvedFileAmnt++;
            }
            else
                LogW("file \"" << tmp << "\" resolved!");
        }
        else
            LogW("file \"" << tmp << "\" resolved!");
    }

    return stream;
}

void LDrawConverter::ResolveAll()
{
    while (!unresolvedFiles.empty())
    {
        UnresolvedFile currentFile = unresolvedFiles.back();
        unresolvedFiles.pop_back();

        // fill the file in fileCache
        if (!ParseFile(fileCache[currentFile.fileID], FindFile(currentFile.fileName, currentFile.fileType), currentFile.fileType))
        {
            fileCache.erase(fileCache.find(currentFile.fileID));
            LogW("File \"" << currentFile.fileName << "\" could not be found"); // TODO: move this to Find file
        }
    }
    LogI(unresolvedFileAmnt << " unresolved files!");
}

void LDrawConverter::LoadColorFile()
{
    std::ifstream fileStream((libPath + "LDConfig.ldr").c_str());
    if (!fileStream.is_open())
    {
        LogE("Could not open color file!");
        return;
    }

    
    //parse the color file
    while(fileStream.good())
    {
        std::vector<std::string> line = parseLine(fileStream);
        if (line.size() == 0)
            continue;

        if (line.size() >= 9 && line[0] == "0" && line[1] == "!COLOUR")
        {
            std::string name = line[2];

            int colorID = stoi(line[4]);

            glm::vec3 color;
            std::string tmp = line[6].substr(1, 2);
            color.r = stof(tmp) / 255.0f;
            tmp = line[6].substr(3, 2);
            color.g = stof(tmp) / 255.0f;
            tmp = line[6].substr(5, 2);
            color.b = stof(tmp) / 255.0f;


            glm::vec3 edgeColor;
            tmp = line[7].substr(1);
            edgeColor.r = stof(tmp) / 255.0f;
            tmp = line[8].substr(1);
            edgeColor.g = stof(tmp) / 255.0f;
            tmp = line[9].substr(1);
            edgeColor.b = stof(tmp) / 255.0f;
            
            colorMap[colorID] = LDrawColor();
            colorMap[colorID].name = name;
            colorMap[colorID].color = color;
            colorMap[colorID].edgeColor = edgeColor;
        }
    }
}