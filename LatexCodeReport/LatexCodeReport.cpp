﻿#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <direct.h>
#include <vector>
#include <algorithm>

using namespace std;

bool HasEnds(const string& fileName, const string& ending)
{
    if (fileName.length() < ending.length())
    {
        return false;
    }
    return fileName.compare(fileName.length() - ending.length(), ending.length(), ending) == 0;
}

bool IsCodeFile(const string& fileName)
{
    string codeFilePostfix[] = {".cpp", ".h", ".java", ".py", ".kt", ".cs", ".js", ".css"};

    for (string ending : codeFilePostfix)
    {
        if (HasEnds(fileName, ending))
        {
            return true;
        }
    }
    return false;
}

// Получение папки из которой вызвано приложение
string GetWorkDirectory()
{
    char current_work_dir[FILENAME_MAX];
    _getcwd(current_work_dir, sizeof(current_work_dir));
    return current_work_dir;
}

void GetFiles(vector<string>* paths, const string& currentPath)
{
    for (const auto& file : filesystem::directory_iterator(currentPath))
    {
        if (filesystem::is_directory(file))
        {
            GetFiles(paths, file.path().string());
        }
        else
        {
            (*paths).push_back(file.path().string());
        }
    }
}

string GetCodeFileName(const string& codeFilePath)
{
    string codeFileName = "";
    int i = codeFilePath.length();
    while (codeFilePath[i] != '\\' && codeFilePath[i] != '/' && i >= 0)
    {
        codeFileName = codeFilePath[i] + codeFileName;
        i--;
    }
    // Убираем /0 в конце строки
    codeFileName = codeFileName.substr(0, codeFileName.size() - 1);
    return codeFileName;
}

string GetCodeLanguage(const string& fileName)
{
    if (HasEnds(fileName, ".cpp") || HasEnds(fileName, ".h"))
    {
        return "cpp";
    }
    if (HasEnds(fileName, ".java") || HasEnds(fileName, ".kt"))
    {
        return "java";
    }
    if (HasEnds(fileName, ".py"))
    {
        return "python";
    }
    if (HasEnds(fileName, ".cs"))
    {
        return "csharp";
    }
    if (HasEnds(fileName, ".js"))
    {
        return "js";
    }
    if (HasEnds(fileName, ".css"))
    {
        return "css";
    }
    return "NoN";
}

void WriteCodeToFile(const string& codeFilePath)
{
    std::ifstream file(codeFilePath, std::ios_base::in);
    std::string codeFromFile{std::istreambuf_iterator<char>(file),
                             std::istreambuf_iterator<char>()};
    file.close();

    string codeFileName = GetCodeFileName(codeFilePath);
    string codeString = "\\subsubsection*{" + codeFileName +
                        "}\n\\begin{minted}[breaklines, breakafter=d]{" +
                        GetCodeLanguage(codeFileName) + "}\n" + codeFromFile + "\n\\end{minted}\n";

    ofstream fout("code.tex", ios_base::app);
    fout << codeString;
    fout.close();
}

void SortFileNames(vector<string>* fileNames)
{
    vector<string> sortedFileNames;
    int i = 0;
    while (i < (*fileNames).size())
    {
        // Поиск соответствующего cpp файла для файла .h
        if (HasEnds((*fileNames)[i], ".h"))
        {
            string fileName = GetCodeFileName((*fileNames)[i]);
            fileName = fileName.substr(0, fileName.size() - 2);
            fileName = fileName + ".cpp";
            int j = 0;
            sortedFileNames.push_back((*fileNames)[i]);
            while (j < (*fileNames).size())
            {
                if (HasEnds((*fileNames)[j], fileName))
                {
                    sortedFileNames.push_back((*fileNames)[j]);
                    break;
                }
                j++;
            }
        }
        // Если вектор не содержит данного имени файла
        if (std::find(sortedFileNames.begin(), sortedFileNames.end(), (*fileNames)[i]) !=
            sortedFileNames.end())
        {
            sortedFileNames.push_back((*fileNames)[i]);
        }
        i++;
    }

    // Копируем отсортированный вектор в исходный
    (*fileNames).clear();
    copy(sortedFileNames.begin(), sortedFileNames.end(), back_inserter((*fileNames)));
}

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "rus");
    string workDirectory = GetWorkDirectory();
    vector<string> files;
    bool isFirstFile = true;
    GetFiles(&files, workDirectory);
    SortFileNames(&files);

    for (string file : files)
    {
        if (IsCodeFile(file))
        {
            if (isFirstFile)
            {
                ofstream fout("code.tex");
                fout << "";
                fout.close();
                isFirstFile = false;
            }
            WriteCodeToFile(file);
        }
    }

    return 0;
}