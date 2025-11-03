#include "pch.h"
#include "VulkanVertexBuffer.h"
#include "VulkanContext.h"

VulkanVertexBuffer::VulkanVertexBuffer()
{
	auto device = VulkanContext::Get()->GetCurrentDevice();

	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_StagingBuffer, m_StagingBufferMemory);

	void* data;
	vkMapMemory(device, m_StagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(device, m_StagingBufferMemory);

	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_VertexBuffer, m_VertexBufferMemory);

	CopyBuffer(m_StagingBuffer, m_VertexBuffer, bufferSize);
}

VulkanVertexBuffer::~VulkanVertexBuffer()
{
	Shutdown();
}

void VulkanVertexBuffer::Shutdown()
{
	auto device = VulkanContext::Get()->GetCurrentDevice();

	// 确保设备空闲，避免缓冲区仍在使用时被销毁
	vkDeviceWaitIdle(device);

	vkDestroyBuffer(device, m_VertexBuffer, nullptr);
	vkFreeMemory(device, m_VertexBufferMemory, nullptr);
	vkDestroyBuffer(device, m_StagingBuffer, nullptr);
	vkFreeMemory(device, m_StagingBufferMemory, nullptr);
}

void VulkanVertexBuffer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	auto device = VulkanContext::Get()->GetDevice();
	auto commandPool = VulkanContext::Get()->GetSwapChain().GetCurrentDrawCommandPool();

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device->GetVulkanDevice(), &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	auto graphicsQueue = device->GetGraphicsQueue();
	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);

	vkFreeCommandBuffers(device->GetVulkanDevice(), commandPool, 1, &commandBuffer);
}

Ref<VulkanVertexBuffer> VulkanVertexBuffer::Create()
{
	return CreateRef<VulkanVertexBuffer>();
}

uint32_t VulkanVertexBuffer::GetMemoryTypeIndex(uint32_t typeFilter, VkMemoryPropertyFlags properties) const
{
	auto physicalDevice = VulkanContext::Get()->GetPhysicalDevice()->GetVulkanPhysicalDevice();
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find Graphics memory type!");
}

void VulkanVertexBuffer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	// TODO：你应该交给一个专门的类来处理缓冲区的创建和分配
	auto device = VulkanContext::Get()->GetCurrentDevice();

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_CHECK_RESULT(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer));

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;

	allocInfo.memoryTypeIndex = GetMemoryTypeIndex(memRequirements.memoryTypeBits, properties);

	VK_CHECK_RESULT(vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory));

	vkBindBufferMemory(device, buffer, bufferMemory, 0);
}
