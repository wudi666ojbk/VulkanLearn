#include "pch.h"
#include "VulkanUniformBuffer.h"

#include "Renderer/VulkanContext.h"

VulkanUniformBuffer::VulkanUniformBuffer()
{
    auto device = VulkanContext::Get()->GetCurrentDevice();
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    uint32_t framesInFlight = VulkanContext::Get()->GetConfig().FramesInFlight;


    m_UniformBuffers.resize(framesInFlight);
    m_UniformBuffersMemory.resize(framesInFlight);
    m_UniformBuffersMapped.resize(framesInFlight);

    for (size_t i = 0; i < framesInFlight; i++) 
    {
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_UniformBuffers[i], m_UniformBuffersMemory[i]);

        vkMapMemory(device, m_UniformBuffersMemory[i], 0, bufferSize, 0, &m_UniformBuffersMapped[i]);
    }
}

Ref<VulkanUniformBuffer> VulkanUniformBuffer::Create()
{
    return CreateRef<VulkanUniformBuffer>();
}

VulkanUniformBuffer::~VulkanUniformBuffer()
{
    auto device = VulkanContext::Get()->GetCurrentDevice();
    uint32_t framesInFlight = VulkanContext::Get()->GetConfig().FramesInFlight;

    for (size_t i = 0; i < framesInFlight; i++) {
        vkDestroyBuffer(device, m_UniformBuffers[i], nullptr);
        vkFreeMemory(device, m_UniformBuffersMemory[i], nullptr);
    }
}

void VulkanUniformBuffer::UpdateUniformBuffer(uint32_t currentImage)
{
    auto swapChain = VulkanContext::Get()->GetSwapChain();
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), swapChain.GetWidth() / (float)swapChain.GetHight(), 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;

    memcpy(m_UniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}
