#pragma once
#include "Renderer/Vulkan.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Buffer.h"

struct UniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

class VulkanUniformBuffer : VulkanBuffer
{
public:
    VulkanUniformBuffer();
    ~VulkanUniformBuffer();

    static Ref<VulkanUniformBuffer> Create();

    std::vector<VkBuffer> GetVulkanBuffer() const { return m_UniformBuffers; }

    void UpdateUniformBuffer(uint32_t currentImage);
private:
    std::vector<VkBuffer> m_UniformBuffers;
    std::vector<VkDeviceMemory> m_UniformBuffersMemory;
    std::vector<void*> m_UniformBuffersMapped;
};

