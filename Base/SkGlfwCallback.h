#pragma once
#include "SkBase.h"
#include "SkCmd.h"

static SkBase *gBase = nullptr;
static void *callback = nullptr;

void WindowSizeCallback(GLFWwindow *window, int width, int height);
void CursorPosCallback(GLFWwindow *window, double x, double y);
void MouseButtonFun(GLFWwindow *window, int button, int action, int mods);
void KeyCallback(GLFWwindow *window, int key, int scanCode, int action, int mods);
void ScrollCallback(GLFWwindow *window, double x,double y);


class SkGlfwCallback
{
private:
    SkCmd *cmd;
    glm::vec3 rotation = glm::vec3();
    glm::vec3 cameraPos = glm::vec3();
    glm::vec2 mousePos = glm::vec2();
    float scrollPos = 0.0;
    float zoomSpeed = 1.0;
    float zoom = 0;

    struct
    {
        bool left = false;
        bool right = false;
        bool middle = false;
    } mouseButtons;

public:
    struct
    {
        VkDeviceMemory memory;
        VkBuffer buffer;
        VkDescriptorBufferInfo descriptor;
    } uniformBufferVS;

    struct
    {
        glm::mat4 projectionMatrix;
        glm::mat4 modelMatrix;
        glm::mat4 viewMatrix;
    } uboVS;

    SkGlfwCallback();

    void Init(SkBase *initBase, SkCmd *initCmd)
    {
        gBase = initBase;
        cmd = initCmd;
        callback = this;
        gBase->camera.setPosition(glm::vec3(0.0f, 0.0f, -100.0f));
        // gBase->camera.type = Camera::CameraType::firstperson;
        uboVS.modelMatrix = glm::mat4(1.0);
        uboVS.modelMatrix = glm::rotate(uboVS.modelMatrix,glm::radians(-30.0f),{1.0f,0.0f,0.0f});
        uboVS.modelMatrix = glm::rotate(uboVS.modelMatrix,glm::radians(-90.0f),{0.0f,1.0f,0.0f});
        ResetProjection(gBase->GetAspect());

        CreateBuffer();
        UpdataBuffer();
        SetCallback();
    }
    void SetCallback()
    {
        glfwSetWindowSizeCallback(gBase->window, WindowSizeCallback);
        glfwSetCursorPosCallback(gBase->window, CursorPosCallback);
        glfwSetMouseButtonCallback(gBase->window, MouseButtonFun);
        glfwSetKeyCallback(gBase->window, KeyCallback);
        glfwSetScrollCallback(gBase->window, ScrollCallback);
    }
    void ResetProjection(float aspect)
    {
        gBase->camera.setPerspective(glm::radians(45.0f), aspect, 0.0f, 200.0f);
    }
    void CreateBuffer()
    {
        cmd->CreateBuffer(&uboVS, sizeof(uboVS), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &uniformBufferVS.buffer, &uniformBufferVS.memory);
    }
    void UpdataBuffer()
    {
        uboVS.projectionMatrix = gBase->camera.matrices.perspective;
        uboVS.viewMatrix = gBase->camera.matrices.view;
        void *data;
        vkMapMemory(gBase->device, uniformBufferVS.memory, 0, sizeof(uboVS), 0, &data);
        memcpy(data, &uboVS, sizeof(uboVS));
        vkUnmapMemory(gBase->device, uniformBufferVS.memory);
        // fprintf(stderr, "UpdataBuffer...\n");
    }
    void ScrollRoll(float y)
    {
        zoom += y * 3.5f * this->zoomSpeed;
        gBase->camera.translate(glm::vec3(-0.0f, 0.0f, y * 3.5f * this->zoomSpeed));
        gBase->viewUpdated = true;
        UpdataBuffer();
    }
    void MouseCallback(float x, float y)
    {
        float dx = mousePos.x - x;
        float dy = mousePos.y - y;
        bool handled = false;

        if (mouseButtons.left)
        {
            rotation.x += dy * 1.25f * gBase->camera.rotationSpeed;
            rotation.y -= dx * 1.25f * gBase->camera.rotationSpeed;
            gBase->camera.rotate(glm::vec3(0.1*dy * gBase->camera.rotationSpeed,0.1* -dx * gBase->camera.rotationSpeed, 0.0f));
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
    void ButtonFun(int button, int action)
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
    void KeyEvent(int key, int action)
    {

        bool isPress = (action == GLFW_PRESS || action == GLFW_REPEAT);
        switch (key)
        {
        case GLFW_KEY_W:
            gBase->camera.keys.up = isPress;
            gBase->camera.update(gBase->deltaTime);
            fprintf(stderr, "Key:%d....Press%d...\n", key, isPress);
            break;
        case GLFW_KEY_S:
            gBase->camera.keys.down = isPress;
            gBase->camera.update(gBase->deltaTime);
            fprintf(stderr, "Key:%d....Press%d...\n", key, isPress);
            break;
        case GLFW_KEY_A:
            gBase->camera.keys.left = isPress;
            gBase->camera.update(gBase->deltaTime);
            fprintf(stderr, "Key:%d....Press%d...\n", key, isPress);
            break;
        case GLFW_KEY_D:
            gBase->camera.keys.right = isPress;
            gBase->camera.update(gBase->deltaTime);
            fprintf(stderr, "Key:%d....Press%d...\n", key, isPress);
            break;
        default:
            break;
        }
        UpdataBuffer();
    }
    VkDescriptorBufferInfo GetCamDescriptor()
    {
        VkDescriptorBufferInfo bufDescriptor={};
        bufDescriptor.buffer=uniformBufferVS.buffer;
        bufDescriptor.offset=0;
        bufDescriptor.range=sizeof(uboVS);
        return bufDescriptor;
    }
    void CleanUp()
    {
        vkDestroyBuffer(gBase->device, uniformBufferVS.buffer, nullptr);
        vkFreeMemory(gBase->device, uniformBufferVS.memory, nullptr);
    }
    ~SkGlfwCallback();
};

SkGlfwCallback::SkGlfwCallback(/* args */)
{
}

SkGlfwCallback::~SkGlfwCallback()
{
}
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