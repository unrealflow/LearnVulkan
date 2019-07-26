#pragma once
#include "SkBase.h"

static SkBase *gBase = nullptr;
static void *callback = nullptr;

void WindowSizeCallback(GLFWwindow *window, int width, int height);
void CursorPosCallback(GLFWwindow *window, double x, double y);
void MouseButtonFun(GLFWwindow *window,int button,int action,int mods);

class SkGlfwCallback
{
private:
    glm::vec3 rotation = glm::vec3();
    glm::vec3 cameraPos = glm::vec3();
    glm::vec2 mousePos = glm::vec2();
    float zoom = 0;

    struct
    {
        bool left = false;
        bool right = false;
        bool middle = false;
    } mouseButtons;

public:
    struct {
		VkDeviceMemory memory;		
		VkBuffer buffer;			
		VkDescriptorBufferInfo descriptor;
	}  uniformBufferVS;

    struct {
		glm::mat4 projectionMatrix;
		glm::mat4 modelMatrix;
		glm::mat4 viewMatrix;
	} uboVS;
    
    SkGlfwCallback();

    void Init(SkBase *initBase)
    {
        gBase = initBase;
        callback = this;
        SetCallback();
    }
    void SetCallback()
    {
        glfwSetWindowSizeCallback(gBase->window, WindowSizeCallback);
        glfwSetCursorPosCallback(gBase->window, CursorPosCallback);
        glfwSetMouseButtonCallback(gBase->window,MouseButtonFun);
    }
    void MouseCallback(float x, float y)
    {
        float dx = mousePos.x - x;
        float dy = mousePos.y - y;
        bool handled = false;

        if (mouseButtons.left)
        {
            rotation.x += dy * 1.25f * gBase->rotationSpeed;
            rotation.y -= dx * 1.25f * gBase->rotationSpeed;
            gBase->camera.rotate(glm::vec3(dy * gBase->camera.rotationSpeed, -dx * gBase->camera.rotationSpeed, 0.0f));
            gBase->viewUpdated = true;
        }
        else if (mouseButtons.right)
        {
            zoom += dy * .005f * gBase->zoomSpeed;
            gBase->camera.translate(glm::vec3(-0.0f, 0.0f, dy * .005f * gBase->zoomSpeed));
            gBase->viewUpdated = true;
        }
        else if (mouseButtons.middle)
        {
            cameraPos.x -= dx * 0.01f;
            cameraPos.y -= dy * 0.01f;
            gBase->camera.translate(glm::vec3(-dx * 0.01f, -dy * 0.01f, 0.0f));
            gBase->viewUpdated = true;
        }

        mousePos = glm::vec2(x, y);
    }
    void ButtonFun(int button,int action)
    {
        bool isPress=(action==GLFW_PRESS);
        switch (button)
        {
        case GLFW_MOUSE_BUTTON_LEFT: mouseButtons.left=isPress; 
            break;
        case GLFW_MOUSE_BUTTON_RIGHT: mouseButtons.right=isPress;
            break;
        case GLFW_MOUSE_BUTTON_MIDDLE: mouseButtons.middle=isPress;
            break;
        default:
            break;
        }
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
    if (gBase == nullptr)
        return;
    gBase->destWidth = static_cast<uint32_t>(width);
    gBase->destHeight = static_cast<uint32_t>(height);
    gBase->resizing = true;
    gBase->prepare = true;
}
void CursorPosCallback(GLFWwindow *window, double x, double y)
{
    if (gBase == nullptr || callback == nullptr)
        return;
    reinterpret_cast<SkGlfwCallback *>(callback)->MouseCallback((float)x, (float)y);
}
void MouseButtonFun(GLFWwindow *window,int button,int action,int mods)
{
     if (gBase == nullptr || callback == nullptr)
        return;
    reinterpret_cast<SkGlfwCallback *>(callback)->ButtonFun(button,action);
}
