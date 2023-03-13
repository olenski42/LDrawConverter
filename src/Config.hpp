#pragma once
#include "Log.hpp"
#include "portable-file-dialogs.h"
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

class Config
{
public:
    Config()
    {
        std::ifstream is_file("config.txt");

        // config file found
        if (is_file.is_open())
        {
            loadConfigFile(is_file);
            is_file.close();
        }
        // no config file found
        else
        {
            getLibraryPath();
            createConfig();
        }
        // config file is empty
        if (ldrawLibraryPath.size() == 0)
        {
            getLibraryPath();
            createConfig();
        }
    }

    std::string ldrawLibraryPath;

private:
    void loadConfigFile(std::ifstream &is_file)
    {
        std::string line;
        while (std::getline(is_file, line))
        {
            std::stringstream is_line(line);
            std::string key;
            if (std::getline(is_line, key, '='))
            {
                std::string value;
                if (std::getline(is_line, value))
                {
                    if (key == "LDrawLibraryPath")
                        ldrawLibraryPath = value;
                }
            }
        }
    }

    void getLibraryPath()
    {
        LogI("Select LDraw library folder")
        ldrawLibraryPath = pfd::select_folder("Select LDraw library folder", ".").result();

        // convert \\ to /
        for (size_t i = 0; i < ldrawLibraryPath.size(); i++)
        {
            if (ldrawLibraryPath[i] == '\\')
                ldrawLibraryPath[i] = '/';
        }
    }

    void createConfig()
    {
        std::ofstream os_file("config.txt");
        if(!os_file.is_open())
            LogE("Could not create config file!");

        os_file << "LDrawLibraryPath=" << ldrawLibraryPath;
        os_file.close();
    }
};