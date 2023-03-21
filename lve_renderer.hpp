#pragma once

#include "lve_window.hpp"
#include "lve_device.hpp"
#include "lve_swap_chain.hpp"
#include "lve_model.hpp"

#include <cassert>
#include <memory>
#include <vector>

namespace lve
{
    class LveRenderer
    {
        public:
            LveRenderer(LveWindow& window, LveDevice& device);
            ~LveRenderer();

            // deleting copy operator and copy constructor
            LveRenderer(const LveRenderer&) = delete;
            LveRenderer &operator=(const LveRenderer&) = delete;

            VkRenderPass getSwapChainRenderPass() const
            {
                return lve_swap_chain_->getRenderPass();
            }

            bool isFrameInProgress() const
            {
                return is_frame_started_;
            }

            VkCommandBuffer getCurrentCommandBuffer() const
            {
                assert(is_frame_started_ && "Cannot get command buffer if frame not in progress");
                return command_buffers_[current_image_index_];
            }

            VkCommandBuffer beginFrame();
            void endFrame();

            void beginSwapChainRenderPass(VkCommandBuffer command_buffer);
            void endSwapChainRenderPass(VkCommandBuffer command_buffer);

        private:
            void createCommandBuffers();
            void freeCommandBuffers();
            void recreateSwapChain();

            LveWindow& lve_window_;
            LveDevice& lve_device_;
            std::unique_ptr<LveSwapChain> lve_swap_chain_;
            std::vector<VkCommandBuffer> command_buffers_;

            uint32_t current_image_index_;
            bool is_frame_started_;
    };
}