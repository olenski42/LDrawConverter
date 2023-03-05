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

inline FileType nextFileType(FileType fileType)
{
    if (fileType != FILETYPE_PRIMITIVE)
        return static_cast<FileType>(fileType - 1);
    else
        return FILETYPE_PRIMITIVE;
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

LDrawFile *LDrawConverter::GetFile(std::string path, FileType fileType)
{
    for (auto it = path.begin(); it != path.end(); it++)
    {
        // convert to forward slash
        if (*it == '\\')
            *it = '/';

        // remove leading slash
        if (it == path.begin() && *it == '/')
            path.erase(it);
    }

    LDrawFile *file;
    auto itToFile = nameResolver.find(path);
    if (itToFile == nameResolver.end())
    {
        file = new LDrawFile();
        nameResolver.insert(std::pair<std::string, LDrawFile *>(path, file));

        std::transform(path.begin(), path.end(), path.begin(), ::tolower);

        if (path.size() > 2 && path[0] == 's' && path[1] == '/')
            fileType = FILETYPE_PART;

        file->fileType = fileType;
        unresolvedFiles.push_back(UnresolvedFile{file, path, fileType});
    }
    else
        file = itToFile->second;

    return file;
}

LDrawFile *LDrawConverter::ParseFile(std::string filePath)
{
    std::ifstream fileStream;
    fileStream.open(filePath);

    if (!fileStream.is_open())
    {
        LogE("file \"" << filePath << "\" could not be opened!");
    }

    LDrawFile *file = GetFile(filePath, FILETYPE_MULTIPART);
    ResolveAll();

    return unresolvedFiles.size() == 0 ? file : nullptr;
}

bool LDrawConverter::ParseFile(LDrawFile *file, std::ifstream &fileStream, FileType fileType)
{
    bool invertNext = false;
    bool CCW = true;

    // indicates if the current file is a subfile file of a mpd file
    bool MPDsub = false;

    LDrawFile *currentFile = file;

    while (fileStream.good())
    {
        // create vector of elements in the line
        std::vector<std::string> line = parseLine(fileStream);

        // skip empty lines
        if (line.size() == 0)
            continue;

        else if (line.size() >= 3 && line[0] == "0")
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
                // read file name from line(might be multiple words)
                std::string currentFileName;
                for (int i = 2; i < line.size(); i++)
                {
                    currentFileName.append(line[i]);
                    if (i < line.size() - 1)
                        currentFileName.append(" ");
                }

                // make name lower
                std::transform(currentFileName.begin(), currentFileName.end(), currentFileName.begin(), ::tolower);

                if (fileType != FILETYPE_MULTIPART && fileType != FILETYPE_SUBPART)
                    LogW("file \"" << currentFileName << "\" is a subfile of a non-mpd file!");

                if (MPDsub == false)
                {
                    MPDsub = true;
                }
                else
                {
                    LDrawFile *currentSubFile = GetFile(currentFileName, FILETYPE_MULTIPART);
                    currentFile = currentSubFile;
                }

                // TODO: set root file

                for (std::vector<UnresolvedFile>::iterator it = unresolvedFiles.begin(); it != unresolvedFiles.end();)
                {
                    if (it->file == currentFile)
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

            std::transform(subFileName.begin(), subFileName.end(), subFileName.begin(), ::tolower);

            meshCount++;
            bool determinant = glm::determinant(transform) < 0;
            bool invert = invertNext != determinant;
            SubFile s = SubFile{GetFile(subFileName, nextFileType(fileType)), stoul(line[1]), transform, invert};
            currentFile->subFiles.emplace_back(s);
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

            unsigned int index = convSizetUint(currentFile->vertices.size()) - 1;

            if (CCW)
                currentFile->faces.emplace_back(Face{stoul(line[1]), glm::ivec3(index - 2, index - 1, index)});
            else
                currentFile->faces.emplace_back(Face{stoul(line[1]), glm::ivec3(index, index - 1, index - 2)});
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

            unsigned int index = convSizetUint(currentFile->vertices.size()) - 1;
            if (CCW)
            {
                currentFile->faces.emplace_back(Face{stoul(line[1]), glm::ivec3(index - 3, index - 2, index - 1)});
                currentFile->faces.emplace_back(Face{stoul(line[1]), glm::ivec3(index - 3, index - 1, index)});
            }
            else
            {
                currentFile->faces.emplace_back(Face{stoul(line[1]), glm::ivec3(index - 1, index - 2, index - 3)});
                currentFile->faces.emplace_back(Face{stoul(line[1]), glm::ivec3(index, index - 1, index - 3)});
            }
        }

        invertNext = false;
    }

    fileStream.close();
    return true;
}

std::ifstream LDrawConverter::FindFile(UnresolvedFile *file)
{
    if (file->fileType == FILETYPE_SUBPART)
    {
        file->fileType = FILETYPE_PART;
    }

    std::ifstream stream;
    std::string path;

    if (file->fileType == FILETYPE_MULTIPART)
        path = file->fileName;
    else if (file->fileType == FILETYPE_PART)
        path = libPath + "parts/" + file->fileName;
    else if (file->fileType == FILETYPE_PRIMITIVE)
        path = libPath + "p/" + file->fileName;

    stream.open(path);

    if (stream.is_open() == false)
    {
        LogW("file \"" << path << "\" not found, extending search");
        file->fileType = FILETYPE_MULTIPART;
    }
    while (stream.is_open() == false && file->fileType > 0)
    {
        file->fileType = (FileType)((int)file->fileType - 1);

        if (file->fileType == FILETYPE_MULTIPART)
            path = file->fileName;
        else if (file->fileType == FILETYPE_PART)
            path = libPath + "parts/" + file->fileName;
        else if (file->fileType == FILETYPE_PRIMITIVE)
            path = libPath + "p/" + file->fileName;
        else if (file->fileType == FILETYPE_SUBPART)
            continue;

        stream.open(path);
    }

    if (stream.is_open() == false)
        LogW("File \"" << file->fileName << "\" could not be found");

    return stream;
}

void LDrawConverter::ResolveAll()
{
    while (!unresolvedFiles.empty())
    {
        UnresolvedFile currentFile = unresolvedFiles.back();
        unresolvedFiles.pop_back();

        std::ifstream stream = FindFile(&currentFile);
        if (!stream.is_open())
        {
            LogE("Could not open file " << currentFile.fileName);
            continue;
        }

        currentFile.file->fileType = currentFile.fileType;
        ParseFile(currentFile.file, stream, currentFile.fileType);
    }
    LogI(unresolvedFiles.size() << " unresolved files!");
}

void LDrawConverter::LoadColorFile()
{
    std::ifstream fileStream((libPath + "LDConfig.ldr").c_str());
    if (!fileStream.is_open())
    {
        LogE("Could not open color file! (missing \"LDConfig.ldr\" in the lib folder)");
        return;
    }

    // parse the color file
    while (fileStream.good())
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
            color.r = stoul(tmp, nullptr, 16) / 255.0f;
            tmp = line[6].substr(3, 2);
            color.g = stoul(tmp, nullptr, 16) / 255.0f;
            tmp = line[6].substr(5, 2);
            color.b = stoul(tmp, nullptr, 16) / 255.0f;

            glm::vec3 edgeColor;
            tmp = line[8].substr(1);
            edgeColor.r = stoul(tmp, nullptr, 16) / 255.0f;
            tmp = line[8].substr(1);
            edgeColor.g = stoul(tmp, nullptr, 16) / 255.0f;
            tmp = line[8].substr(1);
            edgeColor.b = stoul(tmp, nullptr, 16) / 255.0f;

            colorMap[colorID] = LDrawColor();
            colorMap[colorID].name = name;
            colorMap[colorID].color = color;
            colorMap[colorID].edgeColor = edgeColor;

            // load optional color values
            for (int i = 9; i < line.size(); i++)
            {
                if (line[i] == "ALPHA")
                {
                    colorMap[colorID].transparency = true;
                    if(i + 1 < line.size())
                    {
                        colorMap[colorID].alpha = stof(line[i+1])/ 255.0f;
                        i++;
                    }
                    else
                        LogW("Missing alpha value for color " << colorID);
                }
                else if (line[i] == "LUMINANCE")
                {
                    colorMap[colorID].glow = true;
                    if(i + 1 < line.size())
                    {
                        colorMap[colorID].luminance = stof(line[i+1])/ 255.0f;
                        i++;
                    }
                    else
                        LogW("Missing luminance value for color " << colorID);
                }
                else if (line[i] == "CHROME")
                {
                    colorMap[colorID].chrome = true;
                }
                else if (line[i] == "PEARLESCENT")
                {
                    colorMap[colorID].pearl = true;
                }
                else if (line[i] == "RUBBER")
                {
                    colorMap[colorID].rubber = true;
                }
                else if (line[i] == "MATTE_METALLIC")
                {
                    colorMap[colorID].matteMetallic = true;
                }
                else if (line[i] == "METAL")
                {
                    colorMap[colorID].metallic = true;
                }
                else if (line[i] == "MATERIAL")
                {
                    break;
                }
                else
                {
                    LogW("Unknown color value: " << line[i]);
                }
            }
        }
    }
}