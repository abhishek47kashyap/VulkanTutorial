#pragma once

#include <string>
#include <vector>

namespace lve
{
    class LvePipeline
    {
        public:
            LvePipeline(const std::string& vertex_shader_filepath, const std::string& frag_shader_filepath);

        private:
            static std::vector<char> readFile(const std::string& filepath);
            void createGraphicsPipeline(const std::string& vertex_shader_filepath, const std::string& frag_shader_filepath);
    };
}