#include "SkApp.h"
#include "stdexcept"
#include "iostream"

#ifdef NDEBUG

const bool validation=false;
#else
const bool validation=true;
#endif

class SkRender : public SkApp
{
public:
    SkRender():SkApp("SkRender",validation)
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