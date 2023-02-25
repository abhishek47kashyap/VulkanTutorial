#pragma once

#include "lve_device.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>

namespace lve
{
    class LveModel
    {
        public:
            struct Vertex
            {
                glm::vec2 position;
                static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
                static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
            };

            LveModel(LveDevice& device, const std::vector<Vertex>& vertices);
            ~LveModel();

            // deleting copy operator and copy constructor
            LveModel(const LveModel&) = delete;
            LveModel &operator=(const LveModel&) = delete;

            void bind(VkCommandBuffer command_buffer);
            void draw(VkCommandBuffer command_buffer);

        private:
            void createVertexBuffers(const std::vector<Vertex>& vertices);

            LveDevice& lve_device_;
            VkBuffer vertex_buffer_;
            VkDeviceMemory verterx_buffer_memory_;
            uint32_t vertex_count_;
    };
}
