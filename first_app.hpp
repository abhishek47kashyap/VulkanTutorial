#pragma once

#include "lve_device.hpp"
#include "lve_game_object.hpp"
#include "lve_pipeline.hpp"
#include "lve_window.hpp"
#include "lve_renderer.hpp"

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
            void renderGameObjects(VkCommandBuffer command_buffer);

            LveWindow lve_window_{WIDTH, HEIGHT, "Little Vulkan Engine (lve) project"};
            LveDevice lve_device_{lve_window_};
            LveRenderer lve_renderer_{lve_window_, lve_device_};

            std::unique_ptr<LvePipeline> lve_pipeline_;
            VkPipelineLayout pipeline_layout_;
            std::vector<LveGameObject> game_objects_;
    };
}