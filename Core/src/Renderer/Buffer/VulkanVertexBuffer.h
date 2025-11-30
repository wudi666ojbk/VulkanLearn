#pragma once
#include "Renderer/Vulkan.h"

#include "Buffer.h"

class VulkanVertexBuffer : public VulkanBuffer
{
public:
    VulkanVertexBuffer(void* data, uint64_t size);
    ~VulkanVertexBuffer();

    static Ref<VulkanVertexBuffer> Create(void* data, uint64_t size);
    void Shutdown();

    virtual VkBuffer GetVulkanBuffer() const { return m_VertexBuffer; }
private:
    VkBuffer m_VertexBuffer = nullptr;
    VkDeviceMemory m_VertexBufferMemory = nullptr;
};