#include "lve_pipeline.hpp"
#include "lve_model.hpp"

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

        auto binding_descriptions = LveModel::Vertex::getBindingDescriptions();
        auto attribute_descriptions = LveModel::Vertex::getAttributeDescriptions();
        VkPipelineVertexInputStateCreateInfo vertex_input_info{};
        {
            vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
            vertex_input_info.vertexBindingDescriptionCount = static_cast<uint32_t>(binding_descriptions.size());
            vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();
            vertex_input_info.pVertexBindingDescriptions = binding_descriptions.data();
        }

        VkGraphicsPipelineCreateInfo pipeline_info{};
        {
            pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipeline_info.stageCount = 2;  // how many programmable stages pipeline will use (for now it's only vertex & fragment shaders)
            pipeline_info.pStages = shader_stages;
            pipeline_info.pVertexInputState = &vertex_input_info;
            pipeline_info.pInputAssemblyState = &config_info.input_assembly_info;
            pipeline_info.pViewportState = &config_info.viewport_info;
            pipeline_info.pRasterizationState = &config_info.rasterization_info;
            pipeline_info.pMultisampleState = &config_info.multisample_info;
            pipeline_info.pColorBlendState = &config_info.color_blend_info;
            pipeline_info.pDepthStencilState = &config_info.depth_stencil_info;
            pipeline_info.pDynamicState = &config_info.dynamic_state_info;

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

    void LvePipeline::default_pipeline_config_info_(PipelineConfigInfo& config_info)
    {
        // triangle list as topology https://youtu.be/ecMcXW6MSYU?t=51
        {
            config_info.input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            config_info.input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            config_info.input_assembly_info.primitiveRestartEnable = VK_FALSE;
        }

        // viewport describes transformation between pipeline output (gl_Position values) and target image
        {
            config_info.viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            config_info.viewport_info.viewportCount = 1;
            config_info.viewport_info.pViewports = nullptr;
            config_info.viewport_info.scissorCount = 1;
            config_info.viewport_info.pScissors = nullptr;
        }

        // rasterization https://youtu.be/ecMcXW6MSYU?t=278
        {
            config_info.rasterization_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            config_info.rasterization_info.depthClampEnable = VK_FALSE;   // force depth (z-component) to be between [0, 1]
            config_info.rasterization_info.rasterizerDiscardEnable = VK_FALSE;
            config_info.rasterization_info.polygonMode = VK_POLYGON_MODE_FILL;  // triangles filled in or just the sides
            config_info.rasterization_info.lineWidth = 1.0f;
            config_info.rasterization_info.cullMode = VK_CULL_MODE_NONE;
            config_info.rasterization_info.frontFace = VK_FRONT_FACE_CLOCKWISE;

            // depth bias
            {
                config_info.rasterization_info.depthBiasEnable = VK_FALSE;
                config_info.rasterization_info.depthBiasConstantFactor = 0.0f;  // Optional
                config_info.rasterization_info.depthBiasClamp = 0.0f;           // Optional
                config_info.rasterization_info.depthBiasSlopeFactor = 0.0f;     // Optional
            }
        }

        // multisampling (relates to how rasterizer handles the EDGES of geometry) https://youtu.be/ecMcXW6MSYU?t=463
        {
            config_info.multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            config_info.multisample_info.sampleShadingEnable = VK_FALSE;
            config_info.multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
            config_info.multisample_info.minSampleShading = 1.0f;           // Optional
            config_info.multisample_info.pSampleMask = nullptr;             // Optional
            config_info.multisample_info.alphaToCoverageEnable = VK_FALSE;  // Optional
            config_info.multisample_info.alphaToOneEnable = VK_FALSE;       // Optional
        }

        // color blending (how to blend current output with color value already in the frame buffer)
        {
            config_info.color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                                                VK_COLOR_COMPONENT_G_BIT |
                                                                VK_COLOR_COMPONENT_B_BIT |
                                                                VK_COLOR_COMPONENT_A_BIT;
            config_info.color_blend_attachment.blendEnable = VK_FALSE;
            config_info.color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
            config_info.color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
            config_info.color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
            config_info.color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
            config_info.color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
            config_info.color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

            config_info.color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            config_info.color_blend_info.logicOpEnable = VK_FALSE;
            config_info.color_blend_info.logicOp = VK_LOGIC_OP_COPY;  // Optional
            config_info.color_blend_info.attachmentCount = 1;
            config_info.color_blend_info.pAttachments = &config_info.color_blend_attachment;
            config_info.color_blend_info.blendConstants[0] = 0.0f;  // Optional
            config_info.color_blend_info.blendConstants[1] = 0.0f;  // Optional
            config_info.color_blend_info.blendConstants[2] = 0.0f;  // Optional
            config_info.color_blend_info.blendConstants[3] = 0.0f;  // Optional
        }

        // depth buffer, an additional attachment to frame buffer https://youtu.be/ecMcXW6MSYU?t=525
        {
            config_info.depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            config_info.depth_stencil_info.depthTestEnable = VK_TRUE;
            config_info.depth_stencil_info.depthWriteEnable = VK_TRUE;
            config_info.depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS;
            config_info.depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
            config_info.depth_stencil_info.minDepthBounds = 0.0f;  // Optional
            config_info.depth_stencil_info.maxDepthBounds = 1.0f;  // Optional
            config_info.depth_stencil_info.stencilTestEnable = VK_FALSE;
            config_info.depth_stencil_info.front = {};  // Optional
            config_info.depth_stencil_info.back = {};   // Optional
        }

        // pipeline layout & render pass will be set elsewhere

        // dynamic state enables and dynamic state info fields
        {
            config_info.dynamic_state_enables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
            config_info.dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            config_info.dynamic_state_info.pDynamicStates = config_info.dynamic_state_enables.data();
            config_info.dynamic_state_info.dynamicStateCount = static_cast<uint32_t>(config_info.dynamic_state_enables.size());
            config_info.dynamic_state_info.flags = 0;
        }
    }
}