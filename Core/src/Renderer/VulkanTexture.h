#pragma once
#include "Vulkan.h"

#include "Buffer/Buffer.h"

#include <filesystem>

struct TextureSpecification
{
	uint32_t Width = 1;
	uint32_t Height = 1;
};

// TODO: Move vkImage to VulkanImage2D
class VulkanTexture
{
public:
	VulkanTexture(const TextureSpecification& specification, const std::filesystem::path& filepath);
	~VulkanTexture();

	static Ref<VulkanTexture> Create(const TextureSpecification& specification, const std::filesystem::path& filepath);

	VkImage GetImage() const { return m_Image; }
	VkImageView GetImageView() const { return m_ImageView; }
	VkSampler GetSampler() const { return m_Sampler; }
private:
	void CreateTextureImageView();
	void CreateTextureSampler();
	void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	Buffer ToBufferFromFile(const std::filesystem::path& path, uint32_t& outWidth, uint32_t& outHeight);
private:
	TextureSpecification m_Specification;
	std::filesystem::path m_Path;

	Buffer m_ImageData;
	
	VkImage m_Image;
	VkImageView m_ImageView;
	VkSampler m_Sampler;

	VkDeviceSize m_Size;
	VkDeviceMemory m_DeviceMemory;
};

