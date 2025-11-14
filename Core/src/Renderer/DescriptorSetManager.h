#pragma once
#include "Vulkan.h"
#include <glm/glm.hpp>

class DescriptorSetManager
{
public:
    DescriptorSetManager();

private:
    VkDescriptorPool m_DescriptorPool = nullptr;
};

