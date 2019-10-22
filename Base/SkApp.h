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
#include "SkMesh.h"
#include "SkTexture.h"
#include "SkGlfwCallback.h"
#include "SkAgent.h"
#include "SkModel.h"
class SkApp
{
private:
    //获取窗口名称，附带设备及帧率信息
    std::string getWindowTitle();
    bool isShow = true;
    //调整窗口大小
    void WindowResize()
    {
        if (appBase->destHeight <= 0 || appBase->destWidth <= 0)
        {
            isShow = false;
            return;
        }
        isShow = true;
        fprintf(stderr, "WindowResize...%d,%d\n", appBase->width, appBase->height);
        vkDeviceWaitIdle(appBase->device);
        cmd.FreeCmdBuffers();
        swapChain.Create(appBase->destWidth, appBase->destHeight);
        renderPass.RecreateBuffers();
        Resize0();
        RewriteDescriptorSet();
        cmd.CreateCmdBuffers();
        vkDeviceWaitIdle(appBase->device);
        appBase->resizing = false;
        fprintf(stderr, "WindowResize OK!...\n");
        appBase->camera.upTime = static_cast<float>(glfwGetTime());;
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
    SkAgent agent;
    BScene *scene = nullptr;

public:
    void Run(BScene *s = nullptr)
    {
        this->scene = s;
        InitVulkan();
        MainLoop();
        CleanUp();
    }
    SkApp(std::string Name = "SkApp", bool enableValidation = false)
    {
        appBase = new SkBase();
        appBase->enableDeviceExtensions = deviceExtensions;
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
        agent.Init(appBase);
        renderPass.Init(appBase, &agent);
        cmd.Init(appBase);
        callback.Init(appBase, &agent);
        AppSetup();
    }
    virtual void AppSetup()
    {
    }
    virtual void BeforeDraw(uint32_t imageIndex)
    {
    }
    virtual void RewriteDescriptorSet(bool alloc = false)
    {
    }
    virtual void AfterDraw()
    {
    }
    virtual void Resize0()
    {
    }
    virtual void Draw()
    {
        uint32_t imageIndex;
        VkResult result = (vkAcquireNextImageKHR(appBase->device, appBase->swapChain, UINT64_MAX, appBase->semaphores.presentComplete, (VkFence) nullptr, &(imageIndex)));
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            vkResetFences(appBase->device, 1, &(appBase->waitFences[appBase->currentFrame]));
            WindowResize();
        }
        BeforeDraw(imageIndex);
        appBase->currentTime = static_cast<float>(glfwGetTime());
        appBase->deltaTime = appBase->currentTime - appBase->lastTime;
        appBase->lastTime = appBase->currentTime;
        this->cmd.Draw(imageIndex);
        VkResult _res = this->cmd.Submit(imageIndex);
        if (_res == VK_ERROR_OUT_OF_DATE_KHR)
        {
            WindowResize();
            return;
        }
        VK_CHECK_RESULT(_res);
        AfterDraw();
    }
    void MainLoop()
    {
        appBase->camera.upTime = static_cast<float>(glfwGetTime());
        while (!glfwWindowShouldClose(appBase->window))
        {
            glfwPollEvents();
            if (isShow)
            {
                Draw();
            }
            else
            {
                WindowResize();
            }
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
        agent.CleanUp();
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