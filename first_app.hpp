#pragma once

#include "lve_window.hpp"
#include "lve_pipeline.hpp"
#include "lve_device.hpp"
#include "lve_swap_chain.hpp"

#include <memory>
#include <vector>

namespace lve
{
    class FirstApp
    {
        public:
            static constexpr int WIDTH = 800;
            static constexpr int HEIGHT = 600;

            FirstApp();
            ~FirstApp();

            // deleting copy operator and copy constructor
            FirstApp(const FirstApp&) = delete;
            FirstApp &operator=(const FirstApp&) = delete;

            void run();

        private:
            void createPipelineLayout();
            void createPipeline();
            void createCommandBuffers();
            void drawFrame();

            LveWindow lve_window_{WIDTH, HEIGHT, "Little Vulkan Engine (lve) project"};
            LveDevice lve_device_{lve_window_};
            LveSwapChain lve_swap_chain_{lve_device_, lve_window_.getExtent()};
            std::unique_ptr<LvePipeline> lve_pipeline_;
            VkPipelineLayout pipeline_layout_;
            std::vector<VkCommandBuffer> command_buffers_;
    };
}