﻿#include "SkGlfwCallback.h"

static SkBase *gBase = nullptr;
static SkGlfwCallback *callback = nullptr;
#define SAMPLE_COUNT 20
static float dx[SAMPLE_COUNT];
static float dy[SAMPLE_COUNT];
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
float RadicalInverse_VdC(uint32_t bits) 
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10f; // / 0x100000000
}
// ----------------------------------------------------------------------------
glm::vec2 Hammersley(uint32_t i, uint32_t N)
{
    return glm::vec2(float(i)/float(N), RadicalInverse_VdC(i));
} 
void SkGlfwCallback::Init(SkBase *initBase, SkAgent *initAgent)
{
    gBase = initBase;
    agent = initAgent;
    for (int i = 0; i < SAMPLE_COUNT; i++)
    {
        dx[i]=RadicalInverse(2,i);
        dy[i]=RadicalInverse(3,i);
        // fprintf(stderr,"%ff,",dy[i]);   
    }
    // fprintf(stderr,"...\n");
    
    
    callback = this;
    gBase->camera.setPosition(glm::vec3(0.0f, 0.0f, -6.0f));
    gBase->camera.setRotation(glm::vec3(-27.1f, 8.0f, 0.0f));
    gBase->camera.type = Camera::CameraType::lookat;
    ResetProjection(gBase->GetAspect());
    gBase->ubo.proj = gBase->camera.matrices.perspective;
    gBase->ubo.view = gBase->camera.matrices.view;
    ShowMat(gBase->ubo.proj* gBase->ubo.view);
    gBase->ubo.model = glm::mat4(1.0);
    gBase->ubo.projInverse = glm::inverse(gBase->ubo.proj);
    gBase->ubo.viewInverse = glm::inverse(gBase->ubo.view);

    gBase->ubo.iTime = gBase->currentTime;
    gBase->ubo.delta = 1.0f;
    gBase->ubo.upTime = gBase->currentTime;
    gBase->ubo.lightCount = 0;
    // gBase->ubo.modelMatrix = glm::rotate(gBase->ubo.modelMatrix, glm::radians(-30.0f), {1.0f, 0.0f, 0.0f});
    // gBase->ubo.modelMatrix = glm::rotate(gBase->ubo.modelMatrix, glm::radians(-90.0f), {0.0f, 1.0f, 0.0f});
    CreateBuffer();
    SetCallback();
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
    agent->CreateBuffer(&gBase->ubo, sizeof(gBase->ubo), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, &gBase->UBO);
    agent->SetupDescriptor(&gBase->UBO);
    agent->Map(&gBase->UBO);
}

uint64_t i = 0;
void SkGlfwCallback::UpdataBuffer()
{
    gBase->ubo.preProj = gBase->ubo.proj;
    gBase->ubo.preView = gBase->ubo.view;
    gBase->ubo.proj = gBase->camera.matrices.perspective;
    gBase->ubo.view = gBase->camera.matrices.view;
    gBase->ubo.jitterProj = gBase->ubo.proj;
    gBase->ubo.jitterProj[2][0] += (1.0f - 2.0f * dx[i%SAMPLE_COUNT]) / gBase->width;
    gBase->ubo.jitterProj[2][1] += (1.0f - 2.0f * dy[i%SAMPLE_COUNT]) / gBase->height;
    gBase->ubo.projInverse = glm::inverse(gBase->ubo.jitterProj);
    gBase->ubo.viewInverse = glm::inverse(gBase->ubo.view);
    gBase->ubo.iTime = gBase->currentTime;
    gBase->ubo.upTime = gBase->camera.upTime;
    gBase->ubo.delta = gBase->deltaTime;
    i++;
    memcpy(gBase->UBO.data, &gBase->ubo, sizeof(gBase->ubo));
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
    // fprintf(stderr, "%f,%f,%f...\n", gBase->camera.rotation.x, gBase->camera.rotation.y, gBase->camera.rotation.z);

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
    case GLFW_KEY_SPACE:
        fprintf(stderr,"iFrame:%lld\tiTime:%f\tFPS:%f...\n",i,gBase->currentTime,1.0f/gBase->deltaTime);
        // ShowVec(gBase->camera.matrices.view[2]);
        // ShowVec(glm::inverse(gBase->camera.matrices.view)[2]);
        // ShowVec(gBase->camera.GetFront());
        break;
    default:
        break;
    }
    // UpdataBuffer();
}
void SkGlfwCallback::CleanUp()
{
    agent->FreeBuffer(&gBase->UBO);
    fprintf(stderr,"average FPS:%f...\n",i/gBase->currentTime);
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
    if (height <= 0)
    {
        return;
    }
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