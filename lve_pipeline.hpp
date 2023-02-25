#pragma once

#include "lve_device.hpp"

#include <string>
#include <vector>

namespace lve
{
    struct PipelineConfigInfo {};
    class LvePipeline
    {
        public:
            LvePipeline(LveDevice& device, const std::string& vertex_shader_filepath, const std::string& frag_shader_filepath, const PipelineConfigInfo& config_info);
            ~LvePipeline() {}

            // deleting copy operator and copy constructor (https://youtu.be/LYKlEIzGmW4?t=549)
            LvePipeline(const LvePipeline&) = delete;
            void operator=(const LvePipeline&) = delete;

            static PipelineConfigInfo default_pipeline_config_info_(uint32_t width, uint32_t height);

        private:
            static std::vector<char> readFile(const std::string& filepath);

            void createGraphicsPipeline(const std::string& vertex_shader_filepath, const std::string& frag_shader_filepath, const PipelineConfigInfo& config_info);

            void createShaderPipeline(const std::vector<char>& code, VkShaderModule* shader_module);

            LveDevice& lve_device_;
            VkPipeline graphics_pipeline_;
            VkShaderModule vertex_shader_module_;
            VkShaderModule fragment_shader_module_;
    };
}