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

class SkApp
{
private:
    //获取窗口名称，附带设备及帧率信息
    std::string getWindowTitle();

    //调整窗口大小
    void WindowResize()
    {
        if (!appBase->prepare)
        {
            return;
        }
        appBase->prepare = false;
        vkDeviceWaitIdle(appBase->device);
        swapChain.Create(appBase->destWidth, appBase->destHeight);
        this->ResizeCleanUp();
        this->ResizeInit();
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
        appBase->settings.name = Name;
        appBase->settings.validation = enableValidation;
    }

protected:
    void InitVulkan()
    {
        instance.Init(appBase);
        device.Init(appBase);
        swapChain.Init(appBase);
    }
    void ResizeInit()
    {
        renderPass.Init(appBase);
        pipeline.Init(appBase);
        cmd.Init(appBase, &device);
        AppSetup();
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
    void ResizeCleanUp()
    {
        CleanUp0();
        cmd.CleanUp();
        CleanUp1();
        pipeline.CleanUp();
        CleanUp2();
        renderPass.CleanUp();
    }
    void CleanUp()
    {

        fprintf(stderr, "App::Cleanup...\n");
        vkDeviceWaitIdle(appBase->device);
        ResizeCleanUp();
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