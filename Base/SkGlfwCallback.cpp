#include "SkGlfwCallback.h"

static SkBase *gBase = nullptr;
static SkGlfwCallback *callback = nullptr;

void SkGlfwCallback::Init(SkBase *initBase, SkMemory *initMem)
{
    gBase = initBase;
    mem = initMem;
    callback = this;
    gBase->camera.setPosition(glm::vec3(0.0f, 0.0f, -10.0f));
    gBase->camera.setRotation(glm::vec3(-29.2f, 15.0f, 0.0f));
    gBase->camera.type = Camera::CameraType::lookat;
    uboVS.modelMatrix = glm::mat4(1.0);
    // uboVS.modelMatrix = glm::rotate(uboVS.modelMatrix, glm::radians(-30.0f), {1.0f, 0.0f, 0.0f});
    // uboVS.modelMatrix = glm::rotate(uboVS.modelMatrix, glm::radians(-90.0f), {0.0f, 1.0f, 0.0f});
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
    gBase->camera.setPerspective((45.0f), aspect, 0.1f, 200.0f);
    // UpdataBuffer();
}
void SkGlfwCallback::CreateBuffer()
{
    mem->CreateBuffer(&uboVS, sizeof(uboVS), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, &gBase->vpBuffer);
    mem->SetupDescriptor(&gBase->vpBuffer);
    mem->Map(&gBase->vpBuffer);
}
float RadicalInverse(uint32_t Base, uint64_t i)
{
    float Digit, Radical, Inverse;
    Digit = Radical = 1.0f / (float)Base;
    Inverse = 0.0f;
    while (i)
    {
        // i余Base求出i在"Base"进制下的最低位的数
        // 乘以Digit将这个数镜像到小数点右边
        Inverse += Digit * (float)(i % Base);
        Digit *= Radical;

        // i除以Base即可求右一位的数
        i /= Base;
    }
    return Inverse;
}
uint64_t i = 0;
void SkGlfwCallback::UpdataBuffer()
{
    gBase->camera.matrices.perspective[2][0] += (1.0f - 2.0f * RadicalInverse(5, i)) / gBase->width;
    gBase->camera.matrices.perspective[2][1] += (1.0f - 2.0f * RadicalInverse(3, i)) / gBase->height;
    uboVS.projectionMatrix = gBase->camera.matrices.perspective;
    uboVS.viewMatrix = gBase->camera.matrices.view;
    // gBase->camera.matrices.perspective[2][0] += (1.0f - 2.0f * RadicalInverse(2, i)) / gBase->width;
    // gBase->camera.matrices.perspective[2][1] += (1.0f - 2.0f * RadicalInverse(3, i)) / gBase->height;
    // uboVS.projectionMatrix[2][0] += (1.0f-2.0f * RadicalInverse(2,i)) / gBase->width;
    // uboVS.projectionMatrix[2][1] += (1.0f-2.0f * RadicalInverse(3,i)) / gBase->height;
    i++;
    memcpy(gBase->vpBuffer.data, &uboVS, sizeof(uboVS));
}
void SkGlfwCallback::ScrollRoll(float y)
{
    zoom += y * 0.5f * this->zoomSpeed;
    gBase->camera.translate(glm::vec3(-0.0f, 0.0f, y * 0.5f * this->zoomSpeed));
    gBase->viewUpdated = true;
    gBase->camera.upTime = gBase->currentTime;
    // fprintf(stderr, "%f,%f,%f...\n", gBase->camera.rotation.x, gBase->camera.rotation.y, gBase->camera.rotation.z);

    // UpdataBuffer();
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
        gBase->camera.upTime = gBase->currentTime;
    }
    else if (mouseButtons.right)
    {
        zoom += dy * .005f * this->zoomSpeed;
        gBase->camera.translate(glm::vec3(-0.0f, 0.0f, dy * .05f * this->zoomSpeed));
        gBase->viewUpdated = true;
        gBase->camera.upTime = gBase->currentTime;
    }
    else if (mouseButtons.middle)
    {
        cameraPos.x -= dx * 0.01f;
        cameraPos.y -= dy * 0.01f;
        gBase->camera.translate(glm::vec3(-dx * 0.01f, -dy * 0.01f, 0.0f));
        gBase->viewUpdated = true;
        gBase->camera.upTime = gBase->currentTime;
    }
    // UpdataBuffer();
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
        gBase->camera.upTime = gBase->currentTime;
        break;
    case GLFW_KEY_S:
        gBase->camera.keys.down = isPress;
        gBase->camera.update(gBase->deltaTime);
        gBase->camera.upTime = gBase->currentTime;
        break;
    case GLFW_KEY_A:
        gBase->camera.keys.left = isPress;
        gBase->camera.update(gBase->deltaTime);
        gBase->camera.upTime = gBase->currentTime;
        break;
    case GLFW_KEY_D:
        gBase->camera.keys.right = isPress;
        gBase->camera.update(gBase->deltaTime);
        gBase->camera.upTime = gBase->currentTime;
        break;
    default:
        break;
    }
    // UpdataBuffer();
}
void SkGlfwCallback::CleanUp()
{
    mem->FreeBuffer(&gBase->vpBuffer);
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
    callback->ResetProjection((float)width / height);
}
void CursorPosCallback(GLFWwindow *window, double x, double y)
{
    if (gBase == nullptr || callback == nullptr)
    {
        fprintf(stderr, "gBase is Null...\n");
        return;
    }
    callback->MouseCallback((float)x, (float)y);
}
void MouseButtonFun(GLFWwindow *window, int button, int action, int mods)
{
    if (gBase == nullptr || callback == nullptr)
    {
        fprintf(stderr, "gBase is Null...\n");
        return;
    }
    callback->ButtonFun(button, action);
}
void KeyCallback(GLFWwindow *window, int key, int scanCode, int action, int mods)
{
    if (gBase == nullptr || callback == nullptr)
    {
        fprintf(stderr, "gBase is Null...\n");
        return;
    }
    callback->KeyEvent(key, action);
}
void ScrollCallback(GLFWwindow *window, double x, double y)
{
    if (gBase == nullptr || callback == nullptr)
    {
        fprintf(stderr, "gBase is Null...\n");
        return;
    }
    callback->ScrollRoll((float)y);
}