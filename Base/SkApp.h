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
    //��ȡ�������ƣ������豸��֡����Ϣ
    std::string getWindowTitle();
    
    //�������ڴ�С
    void windowResize();
    void handleMouseMove(int32_t x, int32_t y);
protected:
    SkBase * appBase;
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
        appBase=new SkBase();
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
        cmd.Init(appBase,&device);
        AppSetup();
    }
    virtual void AppSetup()
    {
        fprintf(stderr,"SkApp::AppSetup...\n");
        
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
    void CleanUp()
    {
       
        fprintf(stderr,"App::Cleanup...\n");
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
        appBase=nullptr;
    }

};