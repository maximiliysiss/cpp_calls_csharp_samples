#import "../Com/bin/publish/Com.tlb" raw_interfaces_only

#include <iostream>

int main()
{
    HRESULT hr = CoInitialize(nullptr); // Initialize COM
    if (FAILED(hr))
    {
        std::cerr << "Failed to initialize COM." << std::endl;
        return -1;
    }

    std::cout << "Successfully initialized COM." << std::endl;

    try
    {
        // Create an instance of the COM class
        Com::IComInterfacePtr comObject;
        hr = comObject.CreateInstance(__uuidof(Com::ComInterface));
        if (FAILED(hr))
        {
            std::cerr << "Failed to create COM object. Status - " << hr << "/" << std::hex << hr << std::endl;
            CoUninitialize();
            return -1;
        }

        std::cout << "Successfully created COM object." << std::endl;

        // Call the method
        long result = 0;
        comObject->Calculate(1, 1, &result);

        std::cout << "Calculation result is " << result << '\n';
    }
    catch (const _com_error& e)
    {
        std::cerr << "COM error: " << e.ErrorMessage() << std::endl;
    }

    CoUninitialize(); // Clean up COM
    return 0;
}
