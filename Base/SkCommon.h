#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include "SkTools.h"

const int WIDTH =960;
const int HEIGHT=540;

const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
std::vector<const char *> validationLayers = {"VK_LAYER_LUNARG_standard_validation"};