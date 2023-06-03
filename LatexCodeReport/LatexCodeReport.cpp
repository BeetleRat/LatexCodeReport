#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <direct.h>
#include <vector>
#include <map>
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

bool IsCodeFile(const string& fileName, const map<string, string>& codeFilePostfix)
{
    for (const auto ending : codeFilePostfix)
    {
        if (HasEnds(fileName, ending.first))
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

string GetCodeLanguage(const string& fileName, const map<string, string>& codeFilePostfix)
{
    for (const auto code : codeFilePostfix)
    {
        if (HasEnds(fileName, code.first))
        {
            return code.second;
        }
    }
    return "NoN";
}

void WriteCodeToFile(const string& codeFilePath, const map<string, string>& codeFilePostfix)
{
    std::ifstream file(codeFilePath, std::ios_base::in);
    std::string codeFromFile{std::istreambuf_iterator<char>(file),
                             std::istreambuf_iterator<char>()};
    file.close();

    string codeFileName = GetCodeFileName(codeFilePath);
    string codeString =
        "\\subsubsection*{" + codeFileName + "}\n\\begin{minted}[fontsize=\\tiny, breaklines, breakafter=d]{" +
        GetCodeLanguage(codeFileName, codeFilePostfix) + "}\n" + codeFromFile + "\n\\end{minted}\n";

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
                    auto fileDublicate =
                        std::find(sortedFileNames.begin(), sortedFileNames.end(), (*fileNames)[j]);
                    if (fileDublicate != sortedFileNames.end())
                    {
                        sortedFileNames.erase(fileDublicate);
                    }
                    sortedFileNames.push_back((*fileNames)[j]);
                    break;
                }
                j++;
            }
        }
        // Если вектор не содержит данного имени файла
        if (std::find(sortedFileNames.begin(), sortedFileNames.end(), (*fileNames)[i]) ==
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

void createCodeFileMap(map<string, string>* newMap)
{
    (*newMap)[".cpp"] = "cpp";
    (*newMap)[".h"] = "cpp";
    (*newMap)[".java"] = "java";
    (*newMap)[".kt"] = "java";
    (*newMap)[".py"] = "python";
    (*newMap)[".cs"] = "csharp";
    (*newMap)[".js"] = "js";
    (*newMap)[".css"] = "css";
}

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "rus");
    map<string, string> codeFilePostfix;
    createCodeFileMap(&codeFilePostfix);

    string workDirectory = GetWorkDirectory();
    vector<string> allFiles;
    vector<string> codeFiles;
    GetFiles(&allFiles, workDirectory);

    for (string file : allFiles)
    {
        if (IsCodeFile(file, codeFilePostfix))
        {
            codeFiles.push_back(file);
        }
    }

    if (codeFiles.size() > 0)
    {
        SortFileNames(&codeFiles);
        ofstream fout("code.tex");
        fout << "";
        fout.close();
        for (string file : codeFiles)
        {
            WriteCodeToFile(file, codeFilePostfix);
        }
    }

    return 0;
}
