#pragma once

#include "lve_device.hpp"
#include "lve_game_object.hpp"
#include "lve_pipeline.hpp"

#include <memory>
#include <vector>

namespace lve
{
    class SimpleRenderSystem
    {
        public:
            SimpleRenderSystem(LveDevice& device, VkRenderPass render_pass);
            ~SimpleRenderSystem();

            // deleting copy operator and copy constructor
            SimpleRenderSystem(const SimpleRenderSystem&) = delete;
            SimpleRenderSystem &operator=(const SimpleRenderSystem&) = delete;

            void renderGameObjects(VkCommandBuffer command_buffer, std::vector<LveGameObject>& game_objects);

        private:
            void createPipelineLayout();
            void createPipeline(VkRenderPass render_pass);

            LveDevice& lve_device_;

            std::unique_ptr<LvePipeline> lve_pipeline_;
            VkPipelineLayout pipeline_layout_;
    };
}