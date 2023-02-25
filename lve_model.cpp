#include "lve_model.hpp"

#include <cassert>
#include <cstring>

namespace lve
{
    std::vector<VkVertexInputBindingDescription> LveModel::Vertex::getBindingDescriptions()
    {
        std::vector<VkVertexInputBindingDescription> binding_descriptions(1);
        binding_descriptions[0].binding = 0;
        binding_descriptions[0].stride = sizeof(Vertex);
        binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return binding_descriptions;
    }

    std::vector<VkVertexInputAttributeDescription> LveModel::Vertex::getAttributeDescriptions()
    {
        std::vector<VkVertexInputAttributeDescription> attribute_descriptions(2);
        attribute_descriptions[0].binding = 0;
        attribute_descriptions[0].location = 0;
        attribute_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attribute_descriptions[0].offset = offsetof(Vertex, position);
        attribute_descriptions[1].binding = 0;
        attribute_descriptions[1].location = 1;
        attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_descriptions[1].offset = offsetof(Vertex, color);
        return attribute_descriptions;
    }

    LveModel::LveModel(LveDevice& device, const std::vector<Vertex>& vertices): lve_device_{device}
    {
        createVertexBuffers(vertices);
    }

    LveModel::~LveModel()
    {
        vkDestroyBuffer(lve_device_.device(), vertex_buffer_, nullptr);
        vkFreeMemory(lve_device_.device(), verterx_buffer_memory_, nullptr);
    }

    void LveModel::bind(VkCommandBuffer command_buffer)
    {
        VkBuffer buffers[] = {vertex_buffer_};
        VkDeviceSize offsets[] = {0};
        constexpr uint32_t first_binding = 0;
        constexpr uint32_t binding_count = 1;
        vkCmdBindVertexBuffers(command_buffer, first_binding, binding_count, buffers, offsets);
    }

    void LveModel::draw(VkCommandBuffer command_buffer)
    {
        constexpr uint32_t instance_count = 1;
        constexpr uint32_t first_vertex = 0;
        constexpr uint32_t first_instance = 0;
        vkCmdDraw(command_buffer, vertex_count_, instance_count, first_vertex, first_instance);
    }

    void LveModel::createVertexBuffers(const std::vector<Vertex>& vertices)
    {
        vertex_count_ = static_cast<uint32_t>(vertices.size());
        assert(vertex_count_ >= 3 && "LveModel::createVertexBuffers() expects a minimum of 3 vertices");

        VkDeviceSize buffer_size = sizeof(vertices[0]) * vertex_count_;   // number of bytes
        lve_device_.createBuffer(
            buffer_size,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,   // host is CPU, device is GPU
            vertex_buffer_,
            verterx_buffer_memory_
        );

        void *data;
        constexpr VkDeviceSize offset = 0;
        constexpr VkMemoryMapFlags mem_map_flags = 0;
        vkMapMemory(lve_device_.device(), verterx_buffer_memory_, offset, buffer_size, mem_map_flags, &data);  // maps host memory (CPU) to device memory (GPU)
        memcpy(data, vertices.data(), static_cast<size_t>(buffer_size));
        vkUnmapMemory(lve_device_.device(), verterx_buffer_memory_);
    }
}