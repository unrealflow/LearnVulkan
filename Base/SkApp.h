#pragma once

#include "SkBase.h"

#include "SkInitalizers.h"
#include "SkDebug.h"
#include "SkInstance.h"
#include "SkSwapChain.h"
#include "SkDevice.h"
#include "SkRenderPass.h"
#include "SkGraphicsPipeline.h"

class SkApp
{
private:
    //获取窗口名称，附带设备及帧率信息
    std::string getWindowTitle();
    
    //调整窗口大小
    void windowResize();
    void handleMouseMove(int32_t x, int32_t y);
    SkBase * appBase;
    //交换链相关的封装
    SkInstance instance;
    SkSwapChain swapChain;
    SkDevice device;
    SkRenderPass renderPass;
    SkGraphicsPipeline pipeline;
public:
    
    void Run()
    {
        initVulkan();
        mainLoop();
        cleanUp();
    }
    SkApp(std::string Name = "SkApp", bool enableValidation = false)
    {
        appBase=new SkBase();
        appBase->settings.name = Name;
        appBase->settings.validation = enableValidation;
    }

protected:
    
    
    void initVulkan()
    {
        instance.Init(appBase);
        device.Init(appBase);
        swapChain.Init(appBase,&device);
        renderPass.Init(appBase);
        pipeline.Init(appBase);
    }
    void mainLoop()
    {
        while (!glfwWindowShouldClose(appBase->window))
        {
            glfwPollEvents();
        }
    }
    void cleanUp()
    {
        fprintf(stderr,"first cleanup...\n");
        
        pipeline.CleanUp();
        renderPass.CleanUp();
        swapChain.CleanUp();
        device.CleanUp();
        instance.CleanUp();
    }
    

    ~SkApp()
    {
    }

};