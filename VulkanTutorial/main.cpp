#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vld.h>

#include "Application.h"

int main() 
{
    try 
    {
        Application application{ 800, 600 };
        application.Run();
    }
    catch (const std::exception& exception) 
    {
        std::cerr << exception.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}