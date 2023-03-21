#pragma once

#include "lve_window.hpp"
#include "lve_pipeline.hpp"
#include "lve_device.hpp"
#include "lve_swap_chain.hpp"
#include "lve_model.hpp"
#include "lve_game_object.hpp"

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
            void loadGameObjects();
            void createPipelineLayout();
            void createPipeline();
            void createCommandBuffers();
            void freeCommandBuffers();
            void drawFrame();
            void recreateSwapChain();
            void recordCommandBuffer(const int image_index);
            void renderGameObjects(VkCommandBuffer command_buffer);

            LveWindow lve_window_{WIDTH, HEIGHT, "Little Vulkan Engine (lve) project"};
            LveDevice lve_device_{lve_window_};
            std::unique_ptr<LveSwapChain> lve_swap_chain_;
            std::unique_ptr<LvePipeline> lve_pipeline_;
            VkPipelineLayout pipeline_layout_;
            std::vector<VkCommandBuffer> command_buffers_;
            std::vector<LveGameObject> game_objects_;
    };
}