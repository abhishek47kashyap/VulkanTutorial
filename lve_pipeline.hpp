#pragma once

#include "lve_device.hpp"

#include <string>
#include <vector>

namespace lve
{
    struct PipelineConfigInfo
    {
        VkViewport viewport;
        VkRect2D scissor;
        VkPipelineInputAssemblyStateCreateInfo input_assembly_info;
        VkPipelineRasterizationStateCreateInfo rasterization_info;
        VkPipelineMultisampleStateCreateInfo multisample_info;
        VkPipelineColorBlendAttachmentState color_blend_attachment;
        VkPipelineColorBlendStateCreateInfo color_blend_info;
        VkPipelineDepthStencilStateCreateInfo depth_stencil_info;
        VkPipelineLayout pipeline_layout = nullptr;
        VkRenderPass render_pass = nullptr;
        uint32_t subpass = 0;
    };
    class LvePipeline
    {
        public:
            LvePipeline(LveDevice& device, const std::string& vertex_shader_filepath, const std::string& frag_shader_filepath, const PipelineConfigInfo& config_info);
            ~LvePipeline();

            // deleting copy operator and copy constructor (https://youtu.be/LYKlEIzGmW4?t=549)
            LvePipeline(const LvePipeline&) = delete;
            void operator=(const LvePipeline&) = delete;

            void bind(VkCommandBuffer command_buffer);

            static PipelineConfigInfo default_pipeline_config_info_(uint32_t width, uint32_t height);

        private:
            static std::vector<char> readFile(const std::string& filepath);

            void createGraphicsPipeline(const std::string& vertex_shader_filepath, const std::string& frag_shader_filepath, const PipelineConfigInfo& config_info);

            void createShaderModule(const std::vector<char>& code, VkShaderModule* shader_module);

            LveDevice& lve_device_;
            VkPipeline graphics_pipeline_;
            VkShaderModule vertex_shader_module_;
            VkShaderModule fragment_shader_module_;
    };
}