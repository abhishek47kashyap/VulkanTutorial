#include "simple_render_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>

namespace lve
{
    struct SimplePushConstantData
    {
        glm::mat4 transform{1.0f};   // default initialized to identity matrix
        alignas(16) glm::vec3 color;   // alignas(16) needed because of https://youtu.be/wlLGLWI9Fdc?t=498
    };

    SimpleRenderSystem::SimpleRenderSystem(LveDevice& device, VkRenderPass render_pass) : lve_device_(device)
    {
        createPipelineLayout();
        createPipeline(render_pass);
    }

    SimpleRenderSystem::~SimpleRenderSystem()
    {
        vkDestroyPipelineLayout(lve_device_.device(), pipeline_layout_, nullptr);
    }

    void SimpleRenderSystem::createPipelineLayout()
    {
        VkPushConstantRange push_constant_range
        {
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            .offset = 0,
            .size = sizeof(SimplePushConstantData)
        };

        VkPipelineLayoutCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        info.setLayoutCount = 0;
        info.pSetLayouts = nullptr;
        info.pushConstantRangeCount = 1;
        info.pPushConstantRanges = &push_constant_range;

        if (vkCreatePipelineLayout(lve_device_.device(), &info, nullptr, &pipeline_layout_) != VK_SUCCESS)
        {
            throw std::runtime_error("SimpleRenderSystem::createPipelineLayout(); could not create pipeline layout");
        }
    }

    void SimpleRenderSystem::createPipeline(VkRenderPass render_pass)
    {
        assert(pipeline_layout_ != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipeline_config_info{};
        LvePipeline::default_pipeline_config_info_(pipeline_config_info);
        pipeline_config_info.render_pass = render_pass;
        pipeline_config_info.pipeline_layout = pipeline_layout_;
        lve_pipeline_ = std::make_unique<LvePipeline>(lve_device_, "shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv", pipeline_config_info);
    }

    void SimpleRenderSystem::renderGameObjects(VkCommandBuffer command_buffer, std::vector<LveGameObject>& game_objects)
    {
        lve_pipeline_->bind(command_buffer);

        for (auto& game_obj : game_objects)
        {
            game_obj.transform_.rotation.y = glm::mod(game_obj.transform_.rotation.y + 0.01f, glm::two_pi<float>());
            game_obj.transform_.rotation.x = glm::mod(game_obj.transform_.rotation.x + 0.005f, glm::two_pi<float>());

            SimplePushConstantData push
            {
                .transform = game_obj.transform_.mat4(),
                .color = game_obj.color_
            };

            vkCmdPushConstants(
                command_buffer,
                pipeline_layout_,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(SimplePushConstantData),
                &push
            );

            game_obj.model_->bind(command_buffer);
            game_obj.model_->draw(command_buffer);
        }
    }
}