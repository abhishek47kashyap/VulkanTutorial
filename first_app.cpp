#include "first_app.hpp"

#include <stdexcept>

namespace lve
{
    FirstApp::FirstApp()
    {
        createPipelineLayout();
        createPipeline();
        createCommandBuffers();
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
        }
    }

    void FirstApp::createPipelineLayout()
    {
        VkPipelineLayoutCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        info.setLayoutCount = 0;
        info.pSetLayouts = nullptr;
        info.pushConstantRangeCount = 0;
        info.pPushConstantRanges = nullptr;

        if (vkCreatePipelineLayout(lve_device_.device(), &info, nullptr, &pipeline_layout_) != VK_SUCCESS)
        {
            throw std::runtime_error("FirstApp::createPipelineLayout(); could not create pipeline layout");
        }
    }

    void FirstApp::createPipeline()
    {
        auto config = LvePipeline::default_pipeline_config_info_(lve_swap_chain_.width(), lve_swap_chain_.height());
        config.render_pass = lve_swap_chain_.getRenderPass();
        config.pipeline_layout = pipeline_layout_;
        lve_pipeline_ = std::make_unique<LvePipeline>(lve_device_, "shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv", config);
    }

    void FirstApp::createCommandBuffers()
    {

    }

    void FirstApp::drawFrame()
    {

    }
}