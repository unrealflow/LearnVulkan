#pragma once
#include "SkBase.h"
#include "SkMemory.h"

void WindowSizeCallback(GLFWwindow *window, int width, int height);
void CursorPosCallback(GLFWwindow *window, double x, double y);
void MouseButtonFun(GLFWwindow *window, int button, int action, int mods);
void KeyCallback(GLFWwindow *window, int key, int scanCode, int action, int mods);
void ScrollCallback(GLFWwindow *window, double x, double y);

//设置鼠标点击、滚轮、按键等事件的函数调用
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
    // SkBuffer uniformBufferVS;

    struct
    {
        glm::mat4 modelMatrix;
        glm::mat4 viewMatrix;
        glm::mat4 projectionMatrix;
    } uboVS;

    SkGlfwCallback() {}

    void Init(SkBase *initBase, SkMemory *initMem);
    void SetCallback();

    void ResetProjection(float aspect);
    void CreateBuffer();

    void UpdataBuffer();

    void ScrollRoll(float y);

    void MouseCallback(float x, float y);

    void ButtonFun(int button, int action);

    void KeyEvent(int key, int action);

    void CleanUp();

    ~SkGlfwCallback() {}
};
