#pragma once

#include <stdexcept>
#include <filesystem>
#include <Windows.h>

#include "coreclr_delegates.h"
#include "hostfxr.h"

namespace fs = std::filesystem;

class Wrapper
{
    struct WrapperParams
    {
        std::wstring assembly;
        std::wstring type;
        std::wstring method;
    };

    typedef int (*proc)(int, int);

    static Wrapper* instance_;

    WrapperParams params_;
    proc proc_ = nullptr;

    explicit Wrapper(const WrapperParams& params)
    {
        params_ = params;
    }

    void init()
    {
        // Get hostfxr procedures
        const auto hostfxr_library = LoadLibraryW(L"hostfxr.dll");

        const auto init_proc = "hostfxr_initialize_for_runtime_config";
        const auto runtime_proc = "hostfxr_get_runtime_delegate";
        const auto close_proc = "hostfxr_close";

        const auto init_fptr = (hostfxr_initialize_for_runtime_config_fn)GetProcAddress(hostfxr_library, init_proc);
        const auto get_delegate_fptr = (hostfxr_get_runtime_delegate_fn)GetProcAddress(hostfxr_library, runtime_proc);
        const auto close_fptr = (hostfxr_close_fn)GetProcAddress(hostfxr_library, close_proc);

        if (!(init_fptr && get_delegate_fptr && close_fptr))
            throw std::runtime_error("Failed to load hostfxr");

        const fs::path runtime_config_path = params_.assembly + L".runtimeconfig.json";

        // Initialize the .NET Core runtime
        hostfxr_handle context = nullptr;
        int rc = init_fptr(runtime_config_path.c_str(), nullptr, &context);

        if (rc != 0 || context == nullptr)
            throw std::runtime_error("Failed to initialize runtime " + std::to_string(rc));

        // Get the load_assembly_and_get_function_pointer delegate
        load_assembly_and_get_function_pointer_fn load_assembly_and_get_function_pointer = nullptr;
        rc = get_delegate_fptr(context,
                               hdt_load_assembly_and_get_function_pointer,
                               (void**)&load_assembly_and_get_function_pointer);

        if (rc != 0 || load_assembly_and_get_function_pointer == nullptr)
        {
            close_fptr(context);

            throw std::runtime_error(
                "Failed to get load_assembly_and_get_function_pointer delegate " + std::to_string(rc));
        }

        // Load the managed assembly and get a function pointer to the C# method
        rc = load_assembly_and_get_function_pointer(
            (params_.assembly + L".dll").c_str(), // assembly file
            params_.type.c_str(), // full qualified type
            params_.method.c_str(), // method
            UNMANAGEDCALLERSONLY_METHOD,
            nullptr,
            (void**)&proc_);

        if (rc != 0 || proc_ == nullptr)
        {
            close_fptr(context);

            throw std::runtime_error("Failed to get managed method pointer " + std::to_string(rc));
        }
    }

public:
    static Wrapper* get_instance(const std::wstring& assembly, const std::wstring& type, const std::wstring& method)
    {
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
