#include "SkGlfwCallback.h"

static SkBase *gBase = nullptr;
static void *callback = nullptr;

void SkGlfwCallback::Init(SkBase *initBase, SkMemory *initMem)
{
    gBase = initBase;
    mem = initMem;
    callback = this;
    gBase->camera.setPosition(glm::vec3(0.0f, 0.0f, -10.0f));
    gBase->camera.type = Camera::CameraType::firstperson;
    uboVS.modelMatrix = glm::mat4(1.0);
    uboVS.modelMatrix = glm::rotate(uboVS.modelMatrix, glm::radians(-30.0f), {1.0f, 0.0f, 0.0f});
    uboVS.modelMatrix = glm::rotate(uboVS.modelMatrix, glm::radians(-90.0f), {0.0f, 1.0f, 0.0f});
    CreateBuffer();
    SetCallback();
    ResetProjection(gBase->GetAspect());
}
void SkGlfwCallback::SetCallback()
{
    glfwSetWindowSizeCallback(gBase->window, WindowSizeCallback);
    glfwSetCursorPosCallback(gBase->window, CursorPosCallback);
    glfwSetMouseButtonCallback(gBase->window, MouseButtonFun);
    glfwSetKeyCallback(gBase->window, KeyCallback);
    glfwSetScrollCallback(gBase->window, ScrollCallback);
}
void SkGlfwCallback::ResetProjection(float aspect)
{
    gBase->camera.setPerspective((45.0f), aspect, 0.0f, 200.0f);
    UpdataBuffer();
}
void SkGlfwCallback::UpdataBuffer()
{
    uboVS.projectionMatrix = gBase->camera.matrices.perspective;
    uboVS.viewMatrix = gBase->camera.matrices.view;
    void *data;
    vkMapMemory(gBase->device, uniformBufferVS.memory, 0, sizeof(uboVS), 0, &data);
    memcpy(data, &uboVS, sizeof(uboVS));
    vkUnmapMemory(gBase->device, uniformBufferVS.memory);
    // fprintf(stderr, "UpdataBuffer...\n");
}
void SkGlfwCallback::ScrollRoll(float y)
{
    zoom += y * 0.5f * this->zoomSpeed;
    gBase->camera.translate(glm::vec3(-0.0f, 0.0f, y * 0.5f * this->zoomSpeed));
    gBase->viewUpdated = true;
    UpdataBuffer();
}
void SkGlfwCallback::MouseCallback(float x, float y)
{
    float dx = mousePos.x - x;
    float dy = mousePos.y - y;
    bool handled = false;

    if (mouseButtons.left)
    {
        rotation.x += dy * 1.25f * gBase->camera.rotationSpeed;
        rotation.y -= dx * 1.25f * gBase->camera.rotationSpeed;
        gBase->camera.rotate(glm::vec3(0.1 * dy * gBase->camera.rotationSpeed, 0.1 * -dx * gBase->camera.rotationSpeed, 0.0f));
        gBase->viewUpdated = true;
    }
    else if (mouseButtons.right)
    {
        zoom += dy * .005f * this->zoomSpeed;
        gBase->camera.translate(glm::vec3(-0.0f, 0.0f, dy * .05f * this->zoomSpeed));
        gBase->viewUpdated = true;
    }
    else if (mouseButtons.middle)
    {
        cameraPos.x -= dx * 0.01f;
        cameraPos.y -= dy * 0.01f;
        gBase->camera.translate(glm::vec3(-dx * 0.01f, -dy * 0.01f, 0.0f));
        gBase->viewUpdated = true;
    }
    UpdataBuffer();
    mousePos = glm::vec2(x, y);
}
void SkGlfwCallback::ButtonFun(int button, int action)
{
    bool isPress = (action == GLFW_PRESS);
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
        mouseButtons.left = isPress;
        break;
    case GLFW_MOUSE_BUTTON_RIGHT:
        mouseButtons.right = isPress;
        break;
    case GLFW_MOUSE_BUTTON_MIDDLE:
        mouseButtons.middle = isPress;
        break;
    default:
        break;
    }
}
void SkGlfwCallback::KeyEvent(int key, int action)
{

    bool isPress = (action == GLFW_PRESS || action == GLFW_REPEAT);
    switch (key)
    {
    case GLFW_KEY_W:
        gBase->camera.keys.up = isPress;
        gBase->camera.update(gBase->deltaTime);
        break;
    case GLFW_KEY_S:
        gBase->camera.keys.down = isPress;
        gBase->camera.update(gBase->deltaTime);
        break;
    case GLFW_KEY_A:
        gBase->camera.keys.left = isPress;
        gBase->camera.update(gBase->deltaTime);
        break;
    case GLFW_KEY_D:
        gBase->camera.keys.right = isPress;
        gBase->camera.update(gBase->deltaTime);
        break;
    default:
        break;
    }
    UpdataBuffer();
}
void SkGlfwCallback::CleanUp()
{
    vkDestroyBuffer(gBase->device, uniformBufferVS.buffer, nullptr);
    vkFreeMemory(gBase->device, uniformBufferVS.memory, nullptr);
}
void WindowSizeCallback(GLFWwindow *window, int width, int height)
{
    if (gBase == nullptr || callback == nullptr)
    {
        fprintf(stderr, "gBase is Null...\n");
        return;
    }
    gBase->destWidth = static_cast<uint32_t>(width);
    gBase->destHeight = static_cast<uint32_t>(height);
    gBase->resizing = true;
    gBase->prepare = true;
    reinterpret_cast<SkGlfwCallback *>(callback)->ResetProjection((float)width / height);
}
void CursorPosCallback(GLFWwindow *window, double x, double y)
{
    if (gBase == nullptr || callback == nullptr)
    {
        fprintf(stderr, "gBase is Null...\n");
        return;
    }
    reinterpret_cast<SkGlfwCallback *>(callback)->MouseCallback((float)x, (float)y);
}
void MouseButtonFun(GLFWwindow *window, int button, int action, int mods)
{
    if (gBase == nullptr || callback == nullptr)
    {
        fprintf(stderr, "gBase is Null...\n");
        return;
    }
    reinterpret_cast<SkGlfwCallback *>(callback)->ButtonFun(button, action);
}
void KeyCallback(GLFWwindow *window, int key, int scanCode, int action, int mods)
{
    if (gBase == nullptr || callback == nullptr)
    {
        fprintf(stderr, "gBase is Null...\n");
        return;
    }
    reinterpret_cast<SkGlfwCallback *>(callback)->KeyEvent(key, action);
}
void ScrollCallback(GLFWwindow *window, double x, double y)
{
    if (gBase == nullptr || callback == nullptr)
    {
        fprintf(stderr, "gBase is Null...\n");
        return;
    }
    reinterpret_cast<SkGlfwCallback *>(callback)->ScrollRoll((float)y);
}