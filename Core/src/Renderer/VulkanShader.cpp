#include "pch.h"
#include "VulkanShader.h"

#include <shaderc/shaderc.hpp>

#include "VulkanContext.h"

VulkanShader::VulkanShader(const std::string& vertShaderPath, const std::string& fragShaderPath)
{
    // 在构造函数中调用ReadShader来读取和编译着色器
    auto vertShaderModule = ReadShader(vertShaderPath, 0); // 0表示顶点着色器
    auto fragShaderModule = ReadShader(fragShaderPath, 1); // 1表示片段着色器

    auto device = VulkanContext::Get()->GetDevice()->GetVulkanDevice();

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    m_PipelineShaderStageCreateInfos = {vertShaderStageInfo, fragShaderStageInfo};

    CreateDescriptors();
}

VulkanShader::~VulkanShader()
{
    auto device = VulkanContext::Get()->GetCurrentDevice();

    for (auto shaderModule : m_PipelineShaderStageCreateInfos)
        vkDestroyShaderModule(device, shaderModule.module, nullptr);
    m_PipelineShaderStageCreateInfos.clear();

    vkDestroyDescriptorSetLayout(device, m_DescriptorSetLayout, nullptr);
}

Ref<VulkanShader> VulkanShader::Init()
{
    return CreateRef<VulkanShader>("Shaders/shader.vert", "Shaders/shader.frag");
}

VkShaderModule VulkanShader::CreateShaderModule(const std::vector<char>& code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    auto device = VulkanContext::Get()->GetDevice()->GetVulkanDevice();

    VK_CHECK_RESULT(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule));

    return shaderModule;
}

void VulkanShader::CreateGraphicsPipeline()
{
}

void VulkanShader::CreateDescriptors()
{
    auto device = VulkanContext::Get()->GetDevice()->GetVulkanDevice();

    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_DescriptorSetLayout))
}

VkShaderModule VulkanShader::ReadShader(const std::string& filepath, int shaderType)
{
    // 编译着色器到SPV格式
    std::vector<uint32_t> spvCode = CompileToSPV(filepath, shaderType);
    
    // 生成SPV文件路径
    std::filesystem::path shaderPath(filepath);
    std::string spvFilePath = shaderPath.replace_extension(".spv").string();
    
    // 保存SPV到文件
    SaveSPVToFile(spvCode, spvFilePath);

    return CreateShaderModule(LoadShader(spvFilePath));
}

std::vector<char> VulkanShader::LoadShader(const std::string& filepath)
{
    return ReadFile(filepath);
}

std::vector<char> VulkanShader::ReadFile(const std::string& filepath)
{
    // 以二进制模式和ate模式打开文件
    // ate模式可以直接定位到文件末尾，方便我们确定文件大小
    std::ifstream file(filepath, std::ios::ate | std::ios::binary);

    if (!file.is_open()) 
    {
        throw std::runtime_error("failed to open file: " + filepath);
    }

    // 获取文件大小
    size_t fileSize = (size_t)file.tellg();
    
    // 创建缓冲区
    std::vector<char> buffer(fileSize);
    
    // 重置文件指针到起始位置并读取所有数据
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    
    file.close();
    
    return buffer;
}

std::vector<uint32_t> VulkanShader::CompileToSPV(const std::string& filepath, int shaderType)
{
    // 使用shaderc库将GLSL代码编译为SPIR-V
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;
    
    // 读取着色器源码
    auto shaderSource = ReadFile(filepath);
    std::string source(shaderSource.begin(), shaderSource.end());
    
    // 确定着色器类型
    shaderc_shader_kind kind;
    switch (shaderType) 
    {
        case 0: // 顶点着色器
            kind = shaderc_glsl_vertex_shader;
            break;
        case 1: // 片段着色器
            kind = shaderc_glsl_fragment_shader;
            break;
        default:
            throw std::runtime_error("Unsupported shader type");
    }
    
    // 编译着色器
    shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, kind, filepath.c_str());
    
    if (module.GetCompilationStatus() != shaderc_compilation_status_success) 
    {
        std::string message = module.GetErrorMessage();
        throw std::runtime_error("Shader compilation failed: " + message);
    }
    
    // 返回编译后的SPIR-V字节码
    return {module.cbegin(), module.cend()};
}

void VulkanShader::SaveSPVToFile(const std::vector<uint32_t>& spvData, const std::string& outputPath)
{
    // 以二进制模式写入文件
    std::ofstream file(outputPath, std::ios::binary);
    
    if (!file.is_open()) 
    {
        throw std::runtime_error("Failed to create output file: " + outputPath);
    }
    
    // 将SPIR-V数据写入文件
    file.write(reinterpret_cast<const char*>(spvData.data()), spvData.size() * sizeof(uint32_t));
    
    file.close();
}