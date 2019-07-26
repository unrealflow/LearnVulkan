#pragma once

#include "SkBase.h"

#include "SkInitalizers.h"
#include "SkDebug.h"
#include "SkInstance.h"
#include "SkSwapChain.h"
#include "SkDevice.h"
#include "SkRenderPass.h"
#include "SkGraphicsPipeline.h"
#include "SkCmd.h"
#include "SkModel.h"
#include "SkTexture.h"

static SkBase * gBase=nullptr;
void resize(GLFWwindow* window,int width,int height)
{
    if(gBase==nullptr)
        return;
    gBase->destWidth=static_cast<uint32_t>(width);
    gBase->destHeight=static_cast<uint32_t>(height);
    gBase->resizing=true;
    gBase->prepare=true;
    glfwSetWindowSize(window,width,height);
}

class SkApp
{
private:
    //��ȡ�������ƣ������豸��֡����Ϣ
    std::string getWindowTitle();

    //�������ڴ�С
    void WindowResize()
    {
        fprintf(stderr,"WindowResize...%d,%d\n",appBase->width,appBase->height);
        vkDeviceWaitIdle(appBase->device);
        // this->ResizeCleanUp();
        cmd.FreeCmdBuffers();
        renderPass.CleanUp();
        swapChain.Create(appBase->destWidth, appBase->destHeight);
        renderPass.Init(appBase);
        cmd.CreateCmdBuffers();
        // this->ResizeInit();
        vkDeviceWaitIdle(appBase->device);
        appBase->resizing=false;
        fprintf(stderr,"WindowResize OK!...\n");
        
    }
    void handleMouseMove(int32_t x, int32_t y);

protected:
    SkBase *appBase;
    //��������صķ�װ
    SkInstance instance;
    SkSwapChain swapChain;
    SkDevice device;
    SkRenderPass renderPass;
    SkGraphicsPipeline pipeline;
    SkCmd cmd;

public:
    void Run()
    {
        InitVulkan();
        MainLoop();
        CleanUp();
    }
    SkApp(std::string Name = "SkApp", bool enableValidation = false)
    {
        appBase = new SkBase();
        gBase=appBase;
        appBase->settings.name = Name;
        appBase->settings.validation = enableValidation;
    }

protected:
    void InitVulkan()
    {
        instance.Init(appBase);
        device.Init(appBase);
        swapChain.Init(appBase);
        renderPass.Init(appBase);
        pipeline.Init(appBase);
        cmd.Init(appBase, &device);
        AppSetup();
        // glfwSetFramebufferSizeCallback();
        glfwSetWindowSizeCallback(appBase->window,resize);
    }
    virtual void AppSetup()
    {
        fprintf(stderr, "SkApp::AppSetup...\n");
    }
    virtual void BeforeDraw()
    {
    }
    virtual void AfterDraw()
    {
    }
    virtual void Draw()
    {
        VkResult _res= this->cmd.Submit();
        if(_res==VK_ERROR_OUT_OF_DATE_KHR)
        {
            WindowResize();
            return;
        }
        VK_CHECK_RESULT(_res);
    }
    void MainLoop()
    {
        while (!glfwWindowShouldClose(appBase->window))
        {
            BeforeDraw();
            Draw();
            AfterDraw();
            glfwPollEvents();
        }
    }

    //Before all Base CleanUp;
    virtual void CleanUp0()
    {
    }

    //after cmd.CleanUp();
    virtual void CleanUp1()
    {
    }
    //after pipeline.CleanUp();
    virtual void CleanUp2()
    {
    }
    void CleanUp()
    {

        fprintf(stderr, "App::Cleanup...\n");
        vkDeviceWaitIdle(appBase->device);
         CleanUp0();
        cmd.CleanUp();
        CleanUp1();
        pipeline.CleanUp();
        CleanUp2();
        renderPass.CleanUp();
        swapChain.CleanUp();
        device.CleanUp();
        instance.CleanUp();
    }

    ~SkApp()
    {
        delete appBase;
        appBase = nullptr;
    }
};