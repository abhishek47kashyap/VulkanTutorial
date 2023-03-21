#include "first_app.hpp"

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
        glm::mat2 transform{1.0f};   // default initialized to identity matrix
        glm::vec2 offset;
        alignas(16) glm::vec3 color;   // alignas(16) needed because of https://youtu.be/wlLGLWI9Fdc?t=498
    };

    FirstApp::FirstApp()
    {
        loadGameObjects();
        createPipelineLayout();
        createPipeline();
    }

    FirstApp::~FirstApp()
    {
        vkDestroyPipelineLayout(lve_device_.device(), pipeline_layout_, nullptr);
    }

    void FirstApp::run()
    {
        while (!lve_window_.shouldClose())
        {
            glfwPollEvents();

            if (auto command_buffer = lve_renderer_.beginFrame())
            {
                lve_renderer_.beginSwapChainRenderPass(command_buffer);
                renderGameObjects(command_buffer);
                lve_renderer_.endSwapChainRenderPass(command_buffer);
                lve_renderer_.endFrame();
            }
        }

        vkDeviceWaitIdle(lve_device_.device());
    }

    void FirstApp::loadGameObjects()
    {
        std::vector<LveModel::Vertex> vertices
        {
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
        };

        auto lve_model = std::make_shared<LveModel>(lve_device_, vertices);

        auto triangle = LveGameObject::createGameObject();
        triangle.model_ = lve_model;
        triangle.color_ = {0.1f, 0.8f, 0.1f};  // green
        triangle.transform_2d_.translation.x = 0.2f;
        triangle.transform_2d_.scale = {2.0f, 0.5f};
        triangle.transform_2d_.rotation = 90.0f * (glm::pi<float>() / 180.0);

        game_objects_.push_back(std::move(triangle));
    }

    void FirstApp::createPipelineLayout()
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
            throw std::runtime_error("FirstApp::createPipelineLayout(); could not create pipeline layout");
        }
    }

    void FirstApp::createPipeline()
    {
        assert(pipeline_layout_ != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipeline_config_info{};
        LvePipeline::default_pipeline_config_info_(pipeline_config_info);
        pipeline_config_info.render_pass = lve_renderer_.getSwapChainRenderPass();
        pipeline_config_info.pipeline_layout = pipeline_layout_;
        lve_pipeline_ = std::make_unique<LvePipeline>(lve_device_, "shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv", pipeline_config_info);
    }

    void FirstApp::renderGameObjects(VkCommandBuffer command_buffer)
    {
        lve_pipeline_->bind(command_buffer);

        for (auto& game_obj : game_objects_)
        {
            game_obj.transform_2d_.rotation = glm::mod(game_obj.transform_2d_.rotation + 0.01f, glm::two_pi<float>());

            SimplePushConstantData push
            {
                .transform = game_obj.transform_2d_.mat2(),
                .offset = game_obj.transform_2d_.translation,
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