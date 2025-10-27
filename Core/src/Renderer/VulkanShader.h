#pragma once

#include <shaderc/shaderc.hpp>
#include <vulkan/vulkan.h>

class VulkanShader
{
public:
    VulkanShader(const std::string& vertShaderPath, const std::string& fragShaderPath);
    virtual ~VulkanShader();

    static void Init();

    VkShaderModule CreateShaderModule(const std::vector<char>& code);

    VkShaderModule GetVertShaderModule() const { return m_VertShaderModule; }
    VkShaderModule GetFragShaderModule() const { return m_FragShaderModule; }

    VkShaderModule ReadShader(const std::string& filepath, int shaderType);
    std::vector<char> LoadShader(const std::string& filepath);

private:
    VkShaderModule m_VertShaderModule;
    VkShaderModule m_FragShaderModule;

    std::vector<char> ReadFile(const std::string& filepath);
    std::vector<uint32_t> CompileToSPV(const std::string& filepath, int shaderType);
    void SaveSPVToFile(const std::vector<uint32_t>& spvData, const std::string& outputPath);
};