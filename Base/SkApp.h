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
    gBase->destWidth=width;
    gBase->destHeight=height;
    gBase->resizing=true;
}

class SkApp
{
private:
    //获取窗口名称，附带设备及帧率信息
    std::string getWindowTitle();

    //调整窗口大小
    void WindowResize()
    {
        if (!appBase->resizing)
        {
            return;
        }
        fprintf(stderr,"WindowResize...\n");
        vkDeviceWaitIdle(appBase->device);
        // this->ResizeCleanUp();
        swapChain.Create(appBase->destWidth, appBase->destHeight);
        renderPass.RecreateFrameBuffers();
        cmd.RecreateCmdBuffers();
        // this->ResizeInit();
        vkDeviceWaitIdle(appBase->device);
        appBase->resizing=false;
        fprintf(stderr,"WindowResize OK!...\n");
        
    }
    void handleMouseMove(int32_t x, int32_t y);

protected:
    SkBase *appBase;
    //交换链相关的封装
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
        glfwSetFramebufferSizeCallback(appBase->window,resize);
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
        if(appBase->resizing)
        {
            WindowResize();
            return;
        }
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