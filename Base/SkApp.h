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
#include "SkGlfwCallback.h"
#include "SkMemory.h"
#include "SkScene.h"
class SkApp
{
private:
    //获取窗口名称，附带设备及帧率信息
    std::string getWindowTitle();

    //调整窗口大小
    void WindowResize()
    {
        fprintf(stderr, "WindowResize...%d,%d\n", appBase->width, appBase->height);
        vkDeviceWaitIdle(appBase->device);
        cmd.FreeCmdBuffers();
        swapChain.Create(appBase->destWidth, appBase->destHeight);
        renderPass.RecreateBuffers();
        RewriteDescriptorSet();
        cmd.CreateCmdBuffers();
        vkDeviceWaitIdle(appBase->device);
        appBase->resizing = false;
        fprintf(stderr, "WindowResize OK!...\n");
    }
    void handleMouseMove(int32_t x, int32_t y);

protected:
    SkBase *appBase;
    SkInstance instance;
    SkSwapChain swapChain;
    SkDevice device;
    SkRenderPass renderPass;
    SkCmd cmd;
    SkGlfwCallback callback;
    SkMemory mem;
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
        appBase->enableDeviceExtensions=deviceExtensions;
        appBase->enableInstanceExtensions.clear();
        appBase->settings.name = Name;
        appBase->settings.validation = enableValidation;
    }

protected:
    void InitVulkan()
    {
        instance.Init(appBase);
        device.Init(appBase);
        swapChain.Init(appBase);
        mem.Init(appBase);
        renderPass.Init(appBase,&mem);
        cmd.Init(appBase); 
        callback.Init(appBase, &mem);
        AppSetup();
    }
    virtual void AppSetup()
    {
        fprintf(stderr, "SkApp::AppSetup...\n");
    }
    virtual void BeforeDraw()
    {
    }
    virtual void RewriteDescriptorSet(bool alloc=false)
    {

    }
    virtual void AfterDraw()
    {
    }
    virtual void Draw()
    {
        appBase->currentTime = glfwGetTime();
        appBase->deltaTime = (float)(appBase->currentTime - appBase->lastTime);
        appBase->lastTime = appBase->currentTime;
        VkResult _res = this->cmd.Submit();
        if (_res == VK_ERROR_OUT_OF_DATE_KHR)
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
        callback.CleanUp();
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