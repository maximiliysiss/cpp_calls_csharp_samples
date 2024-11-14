#include <iostream>
#include <filesystem>
#include <regex>
#include "windows.h"

namespace fs = std::filesystem;

typedef int (*proc)(int, int);

void call_library(const std::string& file_name);

int main()
{
    for (const fs::path path("."); auto& p : fs::directory_iterator(path))
    {
        auto file_name = p.path().filename().string();

        if (std::regex_match(file_name, std::regex("^OldRuntime.Wrapper_.*.dll$")))
        {
            call_library(file_name);
            std::cout << std::endl;
        }
    }
}

void call_library(const std::string& file_name)
{
    const std::wstring name(file_name.begin(), file_name.end());
    const auto library_handle = LoadLibrary(name.c_str());

    if (library_handle == nullptr)
    {
        std::cout << "Cannot open library by path '" << file_name << "'\n";
        return;
    }

    std::cout << "Success LoadLibrary(\"" << file_name << "\")\n";

    const auto proc_name = "calculate";
    const auto proc_address = (proc)GetProcAddress(library_handle, proc_name);

    if (proc_address == nullptr)
    {
        std::cout << "Cannot get proc '" << proc_name << """' in file '" << file_name << "'\n";
        return;
    }

    std::cout << "Success GetProcAddress(\"" << proc_name << "\")\n";

    std::cout << "Calculation result is " << proc_address(1, 1) << '\n';
}
