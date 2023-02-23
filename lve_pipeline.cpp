#include "lve_pipeline.hpp"

#include <fstream>
#include <iostream>

namespace lve
{
    LvePipeline::LvePipeline(const std::string& vertex_shader_filepath, const std::string& frag_shader_filepath)
    {
        createGraphicsPipeline(vertex_shader_filepath, frag_shader_filepath);
    }

    std::vector<char> LvePipeline::readFile(const std::string& filepath)
    {
        std::ifstream file(filepath, std::ios::ate | std::ios::binary);
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open file " + filepath);
        }

        std::size_t file_size = static_cast<std::size_t>(file.tellg());
        std::vector<char> buffer(file_size);

        file.seekg(0);
        file.read(buffer.data(), file_size);

        file.close();
        return buffer;
    }

    void LvePipeline::createGraphicsPipeline(const std::string& vertex_shader_filepath, const std::string& frag_shader_filepath)
    {
        auto vertex_code = readFile(vertex_shader_filepath);
        auto frag_code = readFile(frag_shader_filepath);

        std::cout << "Sizes:\n\tvertex shader = " << vertex_code.size() << "\n\tfragment shader = " << frag_code.size() << std::endl;
    }
}