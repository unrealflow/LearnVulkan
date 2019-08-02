#include "SkGlfwCallback.h"


void WindowSizeCallback(GLFWwindow *window, int width, int height)
{
    if (gBase == nullptr || callback == nullptr)
        return;
    gBase->destWidth = static_cast<uint32_t>(width);
    gBase->destHeight = static_cast<uint32_t>(height);
    gBase->resizing = true;
    gBase->prepare = true;
    reinterpret_cast<SkGlfwCallback *>(callback)->ResetProjection((float)width / height);
}
void CursorPosCallback(GLFWwindow *window, double x, double y)
{
    if (gBase == nullptr || callback == nullptr)
        return;
    reinterpret_cast<SkGlfwCallback *>(callback)->MouseCallback((float)x, (float)y);
}
void MouseButtonFun(GLFWwindow *window, int button, int action, int mods)
{
    if (gBase == nullptr || callback == nullptr)
        return;
    reinterpret_cast<SkGlfwCallback *>(callback)->ButtonFun(button, action);
}
void KeyCallback(GLFWwindow *window, int key, int scanCode, int action, int mods)
{
    if (gBase == nullptr || callback == nullptr)
        return;
    reinterpret_cast<SkGlfwCallback *>(callback)->KeyEvent(key, action);
}
void ScrollCallback(GLFWwindow *window, double x,double y)
{
    if (gBase == nullptr || callback == nullptr)
        return;
    reinterpret_cast<SkGlfwCallback *>(callback)->ScrollRoll((float)y);
}