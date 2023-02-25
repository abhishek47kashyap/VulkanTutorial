#pragma once

#include "lve_window.hpp"
#include "lve_pipeline.hpp"
#include "lve_device.hpp"

namespace lve
{
    class FirstApp
    {
        public:
            static constexpr int WIDTH = 800;
            static constexpr int HEIGHT = 600;

            void run();

        private:
            LveWindow lve_window_{WIDTH, HEIGHT, "Little Vulkan Engine (lve) project"};
            LveDevice lve_device_{lve_window_};
            LvePipeline lve_pipeline{lve_device_, "shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv", LvePipeline::default_pipeline_config_info_(WIDTH, HEIGHT)};
    };
}