#pragma once
#include "Vulkan.h"

#include <filesystem>

struct TextureSpecification
{
	uint32_t Width = 1;
	uint32_t Height = 1;
};

class VulkanTexture
{
public:
	VulkanTexture(const TextureSpecification& specification, const std::filesystem::path& filepath);
	~VulkanTexture();

	static Ref<VulkanTexture> Create(const TextureSpecification& specification, const std::filesystem::path& filepath);

private:
	void CopyBufferToImage(VkBuffer buffer);
	void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void ToBufferFromFile();
private:
	TextureSpecification m_Specification;
	std::filesystem::path m_Path;

	VkImage m_Image;
	VkDeviceSize m_Size;
	VkDeviceMemory m_DeviceMemory;
};

