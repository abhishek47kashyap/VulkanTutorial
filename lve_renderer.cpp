#include "lve_renderer.hpp"

#include <stdexcept>
#include <array>

namespace lve
{
    LveRenderer::LveRenderer(LveWindow& window, LveDevice& device): lve_window_(window), lve_device_(device), is_frame_started_(false)
    {
        recreateSwapChain();
        createCommandBuffers();
    }

    LveRenderer::~LveRenderer()
    {
        freeCommandBuffers();
    }

    void LveRenderer::recreateSwapChain()
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

    }

    void LveRenderer::createCommandBuffers()
    {
        command_buffers_.resize(lve_swap_chain_->imageCount());

        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;    // https://youtu.be/_VOR6q3edig?t=160
        alloc_info.commandPool = lve_device_.getCommandPool();
        alloc_info.commandBufferCount = static_cast<uint32_t>(command_buffers_.size());

        if (vkAllocateCommandBuffers(lve_device_.device(), &alloc_info, command_buffers_.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("LveRenderer::createCommandBuffers(): could not allocate command buffers");
        }
    }

    void LveRenderer::freeCommandBuffers()
    {
        vkFreeCommandBuffers(lve_device_.device(), lve_device_.getCommandPool(), static_cast<uint32_t>(command_buffers_.size()), command_buffers_.data());
        command_buffers_.clear();
    }

    VkCommandBuffer LveRenderer::beginFrame()
    {
        assert(!is_frame_started_ && "Cannot call beginFrame() while already in progress");

        auto result = lve_swap_chain_->acquireNextImage(&current_image_index_);
        if (result == VK_ERROR_OUT_OF_DATE_KHR)  // can occur after window has been resized
        {
            recreateSwapChain();
            return nullptr;
        }
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("LveRenderer::beginFrame(): failed to acquire swap chain image");
        }

        is_frame_started_ = true;

        auto command_buffer = getCurrentCommandBuffer();
        VkCommandBufferBeginInfo cmd_buffer_begin_info{};
        cmd_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        if (vkBeginCommandBuffer(command_buffer, &cmd_buffer_begin_info) != VK_SUCCESS)
        {
            throw std::runtime_error("LveRenderer::beginFrame(): failed to begin recording command buffer");
        }

        return command_buffer;
    }

    void LveRenderer::endFrame()
    {
        assert(is_frame_started_ && "Cannot call endFrame() while already in progress");

        auto command_buffer = getCurrentCommandBuffer();
        if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
        {
            throw std::runtime_error("LveRenderer::endFrame(): failed to record command buffer");
        }

        auto result = lve_swap_chain_->submitCommandBuffers(&command_buffer, &current_image_index_);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || lve_window_.wasWindowResized())
        {
            lve_window_.resetWindowResizedFlag();
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("LveRenderer::endFrame(): failed to present swap chain image");
        }

        is_frame_started_ = false;
    }

    void LveRenderer::beginSwapChainRenderPass(VkCommandBuffer command_buffer)
    {
        assert(is_frame_started_ && "Cannot call beginSwapChainRenderPass() while already in progress");
        assert(command_buffer == getCurrentCommandBuffer() && "Cannot begin render pass on command buffer from a different frame");

        // render pass stuff
        VkRenderPassBeginInfo render_pass_begin_info{};
        render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin_info.renderPass = lve_swap_chain_->getRenderPass();
        render_pass_begin_info.framebuffer = lve_swap_chain_->getFrameBuffer(current_image_index_);
        render_pass_begin_info.renderArea.offset = {0, 0};
        render_pass_begin_info.renderArea.extent = lve_swap_chain_->getSwapChainExtent();

        // clear values
        std::array<VkClearValue, 2> clear_values{};
        clear_values[0].color = {0.01f, 0.01f, 0.01f, 1.0f};   // RGBA
        // clear_values[0].depthStencil = ...   https://youtu.be/_VOR6q3edig?t=414 (also VkClearValue is a union so requires EITHER color OR depth)
        clear_values[1].depthStencil = {1.0f, 0};  // for depth buffer, farthest away value is 1, closest is 0
        render_pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
        render_pass_begin_info.pClearValues = clear_values.data();

        // record to command buffer to begin render pass https://youtu.be/_VOR6q3edig?t=458
        vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(lve_swap_chain_->getSwapChainExtent().width);
        viewport.height = static_cast<float>(lve_swap_chain_->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, lve_swap_chain_->getSwapChainExtent()};
        vkCmdSetViewport(command_buffer, 0, 1, &viewport);
        vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    }

    void LveRenderer::endSwapChainRenderPass(VkCommandBuffer command_buffer)
    {
        assert(is_frame_started_ && "Cannot call endSwapChainRenderPass() while already in progress");
        assert(command_buffer == getCurrentCommandBuffer() && "Cannot end render pass on command buffer from a different frame");

        vkCmdEndRenderPass(command_buffer);
    }
}