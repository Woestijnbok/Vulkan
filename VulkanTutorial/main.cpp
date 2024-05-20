#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vld.h>
#include <format>

#include "Application.h"

int main() 
{
    try 
    {
        std::cout << std::format("The application is {} bytes.", sizeof(Application)) << std::endl;
        Application application{ 1600, 900 };
        application.Run();
    }
    catch (const std::exception& exception) 
    {
        std::cerr << exception.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}