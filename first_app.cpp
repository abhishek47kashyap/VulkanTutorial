#include "first_app.hpp"

#include <stdexcept>
#include <array>

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
            drawFrame();
        }

        vkDeviceWaitIdle(lve_device_.device());
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
        command_buffers_.resize(lve_swap_chain_.imageCount());

        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;    // https://youtu.be/_VOR6q3edig?t=160
        alloc_info.commandPool = lve_device_.getCommandPool();
        alloc_info.commandBufferCount = static_cast<uint32_t>(command_buffers_.size());

        if (vkAllocateCommandBuffers(lve_device_.device(), &alloc_info, command_buffers_.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("FirstApp::createCommandBuffers(): could not allocate command buffers");
        }

        // record draw commands to each buffer
        for (int i = 0; i < command_buffers_.size(); ++i)
        {
            // command buffer stuff
            VkCommandBufferBeginInfo cmd_buffer_begin_info{};
            cmd_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            if (vkBeginCommandBuffer(command_buffers_[i], &cmd_buffer_begin_info) != VK_SUCCESS)
            {
                throw std::runtime_error("FirstApp::createCommandBuffers(): failed to begin recording command buffer");
            }

            // clear values
            std::array<VkClearValue, 2> clear_values{};
            clear_values[0].color = {0.1f, 0.1f, 0.1f, 1.0f};   // RGBA
            // clear_values[0].depthStencil = ...   https://youtu.be/_VOR6q3edig?t=414 (also VkClearValue is a union so requires EITHER color OR depth)
            clear_values[1].depthStencil = {1.0f, 0};  // for depth buffer, farthest away value is 1, closest is 0

            // render pass stuff
            VkRenderPassBeginInfo render_pass_begin_info{};
            render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            render_pass_begin_info.renderPass = lve_swap_chain_.getRenderPass();
            render_pass_begin_info.framebuffer = lve_swap_chain_.getFrameBuffer(i);
            render_pass_begin_info.renderArea.offset = {0, 0};
            render_pass_begin_info.renderArea.extent = lve_swap_chain_.getSwapChainExtent();
            render_pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
            render_pass_begin_info.pClearValues = clear_values.data();

            // record to command buffer to begin render pass https://youtu.be/_VOR6q3edig?t=458
            vkCmdBeginRenderPass(command_buffers_[i], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

            lve_pipeline_->bind(command_buffers_[i]);

            constexpr uint32_t vertex_count    = 3;
            constexpr uint32_t instance_count  = 1;
            constexpr uint32_t first_vertex    = 0;
            constexpr uint32_t first_instance  = 0;
            vkCmdDraw(command_buffers_[i], vertex_count, instance_count, first_vertex, first_instance);

            vkCmdEndRenderPass(command_buffers_[i]);
            if (vkEndCommandBuffer(command_buffers_[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("FirstApp::createCommandBuffers(): failed to record command buffer");
            }
        }
    }

    void FirstApp::drawFrame()
    {
        uint32_t image_index;
        auto result = lve_swap_chain_.acquireNextImage(&image_index);
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("FirstApp::drawFrame(): failed to acquire swap chain image");
        }

        result = lve_swap_chain_.submitCommandBuffers(&command_buffers_[image_index], &image_index);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("FirstApp::drawFrame(): failed to present swap chain image");
        }
    }
}