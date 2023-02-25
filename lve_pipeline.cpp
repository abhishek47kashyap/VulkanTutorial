#include "lve_pipeline.hpp"

#include <fstream>
#include <iostream>
#include <cassert>

namespace lve
{
    LvePipeline::LvePipeline(LveDevice& device, const std::string& vertex_shader_filepath, const std::string& frag_shader_filepath, const PipelineConfigInfo& config_info): lve_device_(device)
    {
        createGraphicsPipeline(vertex_shader_filepath, frag_shader_filepath, config_info);
    }

    LvePipeline::~LvePipeline()
    {
        vkDestroyShaderModule(lve_device_.device(), vertex_shader_module_, nullptr);
        vkDestroyShaderModule(lve_device_.device(), fragment_shader_module_, nullptr);
        vkDestroyPipeline(lve_device_.device(), graphics_pipeline_, nullptr);
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
        assert(config_info.pipeline_layout != VK_NULL_HANDLE && "Cannot create graphics pipeline if pipeline_layout is not provided");
        assert(config_info.render_pass != VK_NULL_HANDLE && "Cannot create graphics pipeline if render_pass is not provided");

        auto vertex_code = readFile(vertex_shader_filepath);
        auto frag_code = readFile(frag_shader_filepath);

        createShaderModule(vertex_code, &vertex_shader_module_);
        createShaderModule(frag_code, &fragment_shader_module_);

        VkPipelineShaderStageCreateInfo shader_stages[2];
        {
            shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
            shader_stages[0].module = vertex_shader_module_;
            shader_stages[0].pName = "main";   // name of the entry function in vertex shader (see simple_shader.vert)
            shader_stages[0].flags = 0;
            shader_stages[0].pNext = nullptr;
            shader_stages[0].pSpecializationInfo = nullptr;

            shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shader_stages[1].module = fragment_shader_module_;
            shader_stages[1].pName = "main";   // name of the entry function in vertex shader (see simple_shader.vert)
            shader_stages[1].flags = 0;
            shader_stages[1].pNext = nullptr;
            shader_stages[1].pSpecializationInfo = nullptr;
        }

        VkPipelineVertexInputStateCreateInfo vertex_input_info{};
        {
            vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertex_input_info.vertexAttributeDescriptionCount = 0;
            vertex_input_info.vertexBindingDescriptionCount = 0;
            vertex_input_info.pVertexAttributeDescriptions = nullptr;
            vertex_input_info.pVertexBindingDescriptions = nullptr;
        }

        VkPipelineViewportStateCreateInfo viewport_info{};
        {
            viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

            viewport_info.viewportCount = 1;
            viewport_info.pViewports = &config_info.viewport;

            viewport_info.scissorCount = 1;
            viewport_info.pScissors = &config_info.scissor;
        }

        VkGraphicsPipelineCreateInfo pipeline_info{};
        {
            pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipeline_info.stageCount = 2;  // how many programmable stages pipeline will use (for now it's only vertex & fragment shaders)
            pipeline_info.pStages = shader_stages;
            pipeline_info.pVertexInputState = &vertex_input_info;
            pipeline_info.pInputAssemblyState = &config_info.input_assembly_info;
            pipeline_info.pViewportState = &viewport_info;
            pipeline_info.pRasterizationState = &config_info.rasterization_info;
            pipeline_info.pMultisampleState = &config_info.multisample_info;
            pipeline_info.pColorBlendState = &config_info.color_blend_info;
            pipeline_info.pDepthStencilState = &config_info.depth_stencil_info;
            pipeline_info.pDynamicState = nullptr;

            pipeline_info.layout = config_info.pipeline_layout;
            pipeline_info.renderPass = config_info.render_pass;
            pipeline_info.subpass = config_info.subpass;

            pipeline_info.basePipelineIndex = -1;
            pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
        }

        if (vkCreateGraphicsPipelines(lve_device_.device(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphics_pipeline_) != VK_SUCCESS)
        {
            throw std::runtime_error("LvePipeline::createGraphicsPipeline(); failed to create graphics pipeline");
        }
    }

    void LvePipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shader_module)
    {
        VkShaderModuleCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        info.codeSize = code.size();
        info.pCode = reinterpret_cast<const uint32_t*>(code.data());

        if (vkCreateShaderModule(lve_device_.device(), &info, nullptr, shader_module) != VK_SUCCESS)
        {
            throw std::runtime_error("LvePipeline::createShaderModule(); failed to create shader module");
        }
    }

    void LvePipeline::bind(VkCommandBuffer command_buffer)
    {
        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_);
    }

    PipelineConfigInfo LvePipeline::default_pipeline_config_info_(uint32_t width, uint32_t height)
    {
        PipelineConfigInfo info{};

        // triangle list as topology https://youtu.be/ecMcXW6MSYU?t=51
        {
            info.input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            info.input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            info.input_assembly_info.primitiveRestartEnable = VK_FALSE;
        }

        // viewport describes transformation between pipeline output (gl_Position values) and target image
        {
            info.viewport.x = 0.0f;
            info.viewport.y = 0.0f;
            info.viewport.width = static_cast<float>(width);
            info.viewport.height = static_cast<float>(height);
            info.viewport.minDepth = 0.0f;
            info.viewport.maxDepth = 1.0f;
        }

        // scissor (pixels outside the 'scissor' rectangle are discarded)
        {
            info.scissor.offset = {0, 0};
            info.scissor.extent = {width, height};
        }

        // rasterization https://youtu.be/ecMcXW6MSYU?t=278
        {
            info.rasterization_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            info.rasterization_info.depthClampEnable = VK_FALSE;   // force depth (z-component) to be between [0, 1]
            info.rasterization_info.rasterizerDiscardEnable = VK_FALSE;
            info.rasterization_info.polygonMode = VK_POLYGON_MODE_FILL;  // triangles filled in or just the sides
            info.rasterization_info.lineWidth = 1.0f;
            info.rasterization_info.cullMode = VK_CULL_MODE_NONE;
            info.rasterization_info.frontFace = VK_FRONT_FACE_CLOCKWISE;

            // depth bias
            {
                info.rasterization_info.depthBiasEnable = VK_FALSE;
                info.rasterization_info.depthBiasConstantFactor = 0.0f;  // Optional
                info.rasterization_info.depthBiasClamp = 0.0f;           // Optional
                info.rasterization_info.depthBiasSlopeFactor = 0.0f;     // Optional
            }
        }

        // multisampling (relates to how rasterizer handles the EDGES of geometry) https://youtu.be/ecMcXW6MSYU?t=463
        {
            info.multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            info.multisample_info.sampleShadingEnable = VK_FALSE;
            info.multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
            info.multisample_info.minSampleShading = 1.0f;           // Optional
            info.multisample_info.pSampleMask = nullptr;             // Optional
            info.multisample_info.alphaToCoverageEnable = VK_FALSE;  // Optional
            info.multisample_info.alphaToOneEnable = VK_FALSE;       // Optional
        }

        // color blending (how to blend current output with color value already in the frame buffer)
        {
            info.color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                                         VK_COLOR_COMPONENT_G_BIT |
                                                         VK_COLOR_COMPONENT_B_BIT |
                                                         VK_COLOR_COMPONENT_A_BIT;
            info.color_blend_attachment.blendEnable = VK_FALSE;
            info.color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
            info.color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
            info.color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
            info.color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
            info.color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
            info.color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

            info.color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            info.color_blend_info.logicOpEnable = VK_FALSE;
            info.color_blend_info.logicOp = VK_LOGIC_OP_COPY;  // Optional
            info.color_blend_info.attachmentCount = 1;
            info.color_blend_info.pAttachments = &info.color_blend_attachment;
            info.color_blend_info.blendConstants[0] = 0.0f;  // Optional
            info.color_blend_info.blendConstants[1] = 0.0f;  // Optional
            info.color_blend_info.blendConstants[2] = 0.0f;  // Optional
            info.color_blend_info.blendConstants[3] = 0.0f;  // Optional
        }

        // depth buffer, an additional attachment to frame buffer https://youtu.be/ecMcXW6MSYU?t=525
        {
            info.depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            info.depth_stencil_info.depthTestEnable = VK_TRUE;
            info.depth_stencil_info.depthWriteEnable = VK_TRUE;
            info.depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS;
            info.depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
            info.depth_stencil_info.minDepthBounds = 0.0f;  // Optional
            info.depth_stencil_info.maxDepthBounds = 1.0f;  // Optional
            info.depth_stencil_info.stencilTestEnable = VK_FALSE;
            info.depth_stencil_info.front = {};  // Optional
            info.depth_stencil_info.back = {};   // Optional
        }

        // pipeline layout & render pass will be set elsewhere

        return info;
    }
}