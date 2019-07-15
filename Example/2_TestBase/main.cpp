#include "SkApp.h"
#include "stdexcept"
#include "iostream"
class TestBase : public SkApp
{

};




int main()
{
    TestBase tb;
    try
    {
        tb.Run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}