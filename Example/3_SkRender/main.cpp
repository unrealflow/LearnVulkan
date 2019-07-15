#include "SkApp.h"
#include "stdexcept"
#include "iostream"
class SkRender : public SkApp
{
public:
    SkRender():SkApp("SkRender",true)
    {

    }



};




int main()
{
    SkRender vr;
    try
    {
        vr.Run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}