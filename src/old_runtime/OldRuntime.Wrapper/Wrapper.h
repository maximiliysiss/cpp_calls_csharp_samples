#pragma once

#include <stdexcept>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <Windows.h>
#include <regex>
#include <vector>

#include "coreclrhost.h"

namespace fs = std::filesystem;

class Wrapper
{
    struct WrapperParams
    {
        std::string assembly;
        std::string type;
        std::string method;
    };

    typedef int (*proc)(int, int);

    static Wrapper* instance_;
    static std::mutex mutex_;

    WrapperParams params_;
    proc proc_ = nullptr;

    explicit Wrapper(const WrapperParams& params)
    {
        params_ = params;
    }

    void init()
    {
        const auto executable_path = fs::current_path();
        const auto assembly_path = executable_path / (params_.assembly + ".dll;");

        // Step 1: Load coreclr.dll
        const auto coreclr_file_name = "coreclr.dll";

        const auto paths = split(getenv("PATH"), ';');
        const auto runtime_path = find_path(coreclr_file_name, paths);

        const auto core_clr = LoadLibraryExA(coreclr_file_name, nullptr, 0);
        if (core_clr == nullptr)
            throw std::runtime_error("Failed to load CoreCLR");

        // Step 2: Get CoreCLR hosting functions
        const auto initialize_core_clr = (coreclr_initialize_ptr)GetProcAddress(core_clr, "coreclr_initialize");
        const auto create_managed_delegate = (coreclr_create_delegate_ptr)GetProcAddress(
            core_clr, "coreclr_create_delegate");
        const auto shutdown_core_clr = (coreclr_shutdown_ptr)GetProcAddress(core_clr, "coreclr_shutdown");

        if (!(initialize_core_clr && create_managed_delegate && shutdown_core_clr))
            throw std::runtime_error("Cannot get coreclr procedures");

        // Step 3: Construct AppDomain properties used when starting the runtime

        // Construct the trusted platform assemblies (TPA) list
        std::string tpa_list(assembly_path.string());

        for (const auto& file : fs::directory_iterator(runtime_path))
        {
            auto file_name = file.path().string();
            if (std::regex_match(file_name, std::regex(".*.dll$")))
                tpa_list += file_name + ';';
        }

        // Define CoreCLR properties
        const char* property_keys[] = {
            "TRUSTED_PLATFORM_ASSEMBLIES"
        };

        const char* property_values[] = {
            tpa_list.c_str(),
        };

        // Step 4: Start the CoreCLR runtime
        void* host_handle;
        unsigned int domain_id;

        int hr = initialize_core_clr(
            executable_path.string().c_str(), // AppDomain base path
            "SampleHost", // AppDomain friendly name
            sizeof(property_keys) / sizeof(char*), // Property count
            property_keys, // Property names
            property_values, // Property values
            &host_handle, // Host handle
            &domain_id); // AppDomain ID

        if (hr < 0)
            throw std::runtime_error("coreclr_initialize failed - status: " + std::to_string(hr));


        // Step 5: Create delegate to managed code
        hr = create_managed_delegate(
            host_handle,
            domain_id,
            params_.assembly.c_str(),
            params_.type.c_str(),
            params_.method.c_str(),
            (void**)&proc_);

        if (hr != S_OK)
        {
            shutdown_core_clr(host_handle, domain_id);
            throw std::runtime_error("coreclr_create_delegate failed - status: " + std::to_string(hr));
        }
    }

    static std::vector<std::string> split(const std::string& str, char delimiter)
    {
        std::vector<std::string> tokens;
        std::stringstream ss(str);
        std::string token;
        while (std::getline(ss, token, delimiter))
        {
            if (!token.empty())
            {
                tokens.push_back(token);
            }
        }
        return tokens;
    }

    static std::string find_path(const std::string& filename, const std::vector<std::string>& paths)
    {
        for (const auto& path : paths)
        {
            fs::path full_path = fs::path(path) / filename;
            if (fs::exists(full_path.string()))
                return path;
        }

        return "";
    }

    static std::string getenv(const char* name)
    {
        size_t required_size;

        getenv_s(&required_size, nullptr, 0, name);
        if (required_size == 0)
            return "";

        const auto env = new char[required_size];
        getenv_s(&required_size, env, required_size, name);

        auto basic_string = std::string(env);

        delete[] env;

        return basic_string;
    }

public:
    static Wrapper* get_instance(const std::string& assembly, const std::string& type, const std::string& method)
    {
        std::lock_guard lock(mutex_);

        if (!instance_)
        {
            instance_ = new Wrapper(WrapperParams{assembly, type, method});
            instance_->init();
        }

        return instance_;
    }

    int calculate(int a, int b) const
    {
        return proc_(a, b);
    }
};

Wrapper* Wrapper::instance_ = nullptr;
std::mutex Wrapper::mutex_;
