#include "lve_pipeline.hpp"

#include <fstream>
#include <iostream>

namespace lve
{
    LvePipeline::LvePipeline(LveDevice& device, const std::string& vertex_shader_filepath, const std::string& frag_shader_filepath, const PipelineConfigInfo& config_info): lve_device_(device)
    {
        createGraphicsPipeline(vertex_shader_filepath, frag_shader_filepath, config_info);
    }

    std::vector<char> LvePipeline::readFile(const std::string& filepath)
    {
        std::ifstream file(filepath, std::ios::ate | std::ios::binary);
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open file " + filepath);
        }

        std::size_t file_size = static_cast<std::size_t>(file.tellg());
        std::vector<char> buffer(file_size);

        file.seekg(0);
        file.read(buffer.data(), file_size);

        file.close();
        return buffer;
    }

    void LvePipeline::createGraphicsPipeline(const std::string& vertex_shader_filepath, const std::string& frag_shader_filepath, const PipelineConfigInfo& config_info)
    {
        auto vertex_code = readFile(vertex_shader_filepath);
        auto frag_code = readFile(frag_shader_filepath);

        std::cout << "Sizes:\n\tvertex shader = " << vertex_code.size() << "\n\tfragment shader = " << frag_code.size() << std::endl;
    }

    void LvePipeline::createShaderPipeline(const std::vector<char>& code, VkShaderModule* shader_module)
    {
        VkShaderModuleCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        info.codeSize = code.size();
        info.pCode = reinterpret_cast<const uint32_t*>(code.data());

        if (vkCreateShaderModule(lve_device_.device(), &info, nullptr, shader_module) != VK_SUCCESS)
        {
            throw std::runtime_error("LvePipeline::createShaderPipeline(); failed to create shader module");
        }
    }

    PipelineConfigInfo LvePipeline::default_pipeline_config_info_(uint32_t width, uint32_t height)
    {
        PipelineConfigInfo info{};

        return info;
    }
}