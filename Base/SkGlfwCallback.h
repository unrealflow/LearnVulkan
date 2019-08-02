#pragma once
#include "SkBase.h"
#include "SkMemory.h"

void WindowSizeCallback(GLFWwindow *window, int width, int height);
void CursorPosCallback(GLFWwindow *window, double x, double y);
void MouseButtonFun(GLFWwindow *window, int button, int action, int mods);
void KeyCallback(GLFWwindow *window, int key, int scanCode, int action, int mods);
void ScrollCallback(GLFWwindow *window, double x, double y);

class SkGlfwCallback
{
private:
    SkMemory *mem;
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

    SkGlfwCallback() {}

    void Init(SkBase *initBase, SkMemory *initMem);
    void SetCallback();

    void ResetProjection(float aspect);
    void CreateBuffer()
    {
        mem->CreateBuffer(&uboVS, sizeof(uboVS), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &uniformBufferVS.buffer, &uniformBufferVS.memory);
    }
    void UpdataBuffer();

    void ScrollRoll(float y);

    void MouseCallback(float x, float y);

    void ButtonFun(int button, int action);

    void KeyEvent(int key, int action);

    VkDescriptorBufferInfo GetCamDescriptor()
    {
        VkDescriptorBufferInfo bufDescriptor = {};
        bufDescriptor.buffer = uniformBufferVS.buffer;
        bufDescriptor.offset = 0;
        bufDescriptor.range = sizeof(uboVS);
        return bufDescriptor;
    }
    void CleanUp();

    ~SkGlfwCallback() {}
};
