#pragma once
#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <vector>


const int WIDTH =800;
const int HEIGHT=600;

const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
std::vector<const char *> validationLayers = {"VK_LAYER_LUNARG_standard_validation"};