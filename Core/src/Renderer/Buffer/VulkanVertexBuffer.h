#pragma once
#include "Renderer/Vulkan.h"

#include "Buffer.h"

class VulkanVertexBuffer : public VulkanBuffer
{
public:
    VulkanVertexBuffer();
    ~VulkanVertexBuffer();

    static Ref<VulkanVertexBuffer> Create();
    void Shutdown();

    virtual VkBuffer GetVulkanBuffer() const { return m_VertexBuffer; }
private:
    VkBuffer m_VertexBuffer = nullptr;
    VkDeviceMemory m_VertexBufferMemory = nullptr;
};