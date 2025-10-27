#pragma once

#include <shaderc/shaderc.hpp>
#include <vulkan/vulkan.h>

class VulkanShader
{
public:
    VulkanShader(const std::string& vertShaderPath, const std::string& fragShaderPath);
    virtual ~VulkanShader();

    static Ref<VulkanShader> Init();

    VkShaderModule CreateShaderModule(const std::vector<char>& code);

    const std::vector<VkPipelineShaderStageCreateInfo>& GetPipelineShaderStageCreateInfos() const { return m_PipelineShaderStageCreateInfos; }

    VkShaderModule ReadShader(const std::string& filepath, int shaderType);
    std::vector<char> LoadShader(const std::string& filepath);

private:
    std::vector<char> ReadFile(const std::string& filepath);
    std::vector<uint32_t> CompileToSPV(const std::string& filepath, int shaderType);
    void SaveSPVToFile(const std::vector<uint32_t>& spvData, const std::string& outputPath);

private:
    std::vector<VkPipelineShaderStageCreateInfo> m_PipelineShaderStageCreateInfos;
};