#include "first_app.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <stdexcept>
#include <array>

namespace lve
{
    struct SimplePushConstantData
    {
        glm::vec2 offset;
        alignas(16) glm::vec3 color;   // alignas(16) needed because of https://youtu.be/wlLGLWI9Fdc?t=498
    };

    FirstApp::FirstApp()
    {
        loadModels();
        createPipelineLayout();
        recreateSwapChain();
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

    void FirstApp::loadModels()
    {
        std::vector<LveModel::Vertex> vertices
        {
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
        };

        lve_model_ = std::make_unique<LveModel>(lve_device_, vertices);
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
        assert(lve_swap_chain_ != nullptr && "Cannot create pipeline before swap chain");
        assert(pipeline_layout_ != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipeline_config_info{};
        LvePipeline::default_pipeline_config_info_(pipeline_config_info);
        pipeline_config_info.render_pass = lve_swap_chain_->getRenderPass();
        pipeline_config_info.pipeline_layout = pipeline_layout_;
        lve_pipeline_ = std::make_unique<LvePipeline>(lve_device_, "shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv", pipeline_config_info);
    }

    void FirstApp::recreateSwapChain()
    {
        // pause and wait if any dimension is sizeless (https://youtu.be/0IIqvi3Z0ng?t=352)
        auto extent = lve_window_.getExtent();
        while (extent.width == 0 || extent.height == 0)
        {
            extent = lve_window_.getExtent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(lve_device_.device());

        if (lve_swap_chain_ == nullptr)
        {
            lve_swap_chain_ = std::make_unique<LveSwapChain>(lve_device_, extent);
        }
        else
        {
            lve_swap_chain_ = std::make_unique<LveSwapChain>(lve_device_, extent, std::move(lve_swap_chain_));
            if (lve_swap_chain_->imageCount() != command_buffers_.size())
            {
                freeCommandBuffers();
                createCommandBuffers();
            }
        }

        createPipeline();
    }

    void FirstApp::createCommandBuffers()
    {
        command_buffers_.resize(lve_swap_chain_->imageCount());

        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;    // https://youtu.be/_VOR6q3edig?t=160
        alloc_info.commandPool = lve_device_.getCommandPool();
        alloc_info.commandBufferCount = static_cast<uint32_t>(command_buffers_.size());

        if (vkAllocateCommandBuffers(lve_device_.device(), &alloc_info, command_buffers_.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("FirstApp::createCommandBuffers(): could not allocate command buffers");
        }
    }

    void FirstApp::freeCommandBuffers()
    {
        vkFreeCommandBuffers(lve_device_.device(), lve_device_.getCommandPool(), static_cast<uint32_t>(command_buffers_.size()), command_buffers_.data());
        command_buffers_.clear();
    }

    void FirstApp::recordCommandBuffer(const int image_index)
    {
        // animation looping every thousandth frame
        static int frame = 0;
        frame = (frame + 1) % 1000;

        VkCommandBufferBeginInfo cmd_buffer_begin_info{};
        cmd_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        if (vkBeginCommandBuffer(command_buffers_[image_index], &cmd_buffer_begin_info) != VK_SUCCESS)
        {
            throw std::runtime_error("FirstApp::createCommandBuffers(): failed to begin recording command buffer");
        }

        // clear values
        std::array<VkClearValue, 2> clear_values{};
        clear_values[0].color = {0.01f, 0.01f, 0.01f, 1.0f};   // RGBA
        // clear_values[0].depthStencil = ...   https://youtu.be/_VOR6q3edig?t=414 (also VkClearValue is a union so requires EITHER color OR depth)
        clear_values[1].depthStencil = {1.0f, 0};  // for depth buffer, farthest away value is 1, closest is 0

        // render pass stuff
        VkRenderPassBeginInfo render_pass_begin_info{};
        render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin_info.renderPass = lve_swap_chain_->getRenderPass();
        render_pass_begin_info.framebuffer = lve_swap_chain_->getFrameBuffer(image_index);
        render_pass_begin_info.renderArea.offset = {0, 0};
        render_pass_begin_info.renderArea.extent = lve_swap_chain_->getSwapChainExtent();
        render_pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
        render_pass_begin_info.pClearValues = clear_values.data();

        // record to command buffer to begin render pass https://youtu.be/_VOR6q3edig?t=458
        vkCmdBeginRenderPass(command_buffers_[image_index], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(lve_swap_chain_->getSwapChainExtent().width);
        viewport.height = static_cast<float>(lve_swap_chain_->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, lve_swap_chain_->getSwapChainExtent()};
        vkCmdSetViewport(command_buffers_[image_index], 0, 1, &viewport);
        vkCmdSetScissor(command_buffers_[image_index], 0, 1, &scissor);

        lve_pipeline_->bind(command_buffers_[image_index]);
        lve_model_->bind(command_buffers_[image_index]);

        // https://youtu.be/wlLGLWI9Fdc?t=264
        constexpr int num_copies = 4;
        for (int j = 0; j < num_copies; j++)
        {
            SimplePushConstantData push
            {
                .offset = {(-0.5f + (frame * 0.002f)), -0.4f + (j * 0.25f)},
                .color = {0.0f, 0.0f, 0.2f + (j * 0.2f)}
            };

            vkCmdPushConstants(
                command_buffers_[image_index],
                pipeline_layout_,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(SimplePushConstantData),
                &push
            );

            lve_model_->draw(command_buffers_[image_index]);
        }

        vkCmdEndRenderPass(command_buffers_[image_index]);
        if (vkEndCommandBuffer(command_buffers_[image_index]) != VK_SUCCESS)
        {
            throw std::runtime_error("FirstApp::createCommandBuffers(): failed to record command buffer");
        }
    }

    void FirstApp::drawFrame()
    {
        uint32_t image_index;
        auto result = lve_swap_chain_->acquireNextImage(&image_index);
        if (result == VK_ERROR_OUT_OF_DATE_KHR)  // can occur after window has been resized
        {
            recreateSwapChain();
            return;
        }
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("FirstApp::drawFrame(): failed to acquire swap chain image");
        }

        recordCommandBuffer(image_index);
        result = lve_swap_chain_->submitCommandBuffers(&command_buffers_[image_index], &image_index);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || lve_window_.wasWindowResized())
        {
            lve_window_.resetWindowResizedFlag();
            recreateSwapChain();
            return;
        }
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("FirstApp::drawFrame(): failed to present swap chain image");
        }
    }
}