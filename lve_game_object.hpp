#pragma once

#include "lve_model.hpp"

// std
#include <memory>

namespace lve
{
    struct Transform2dComponent
    {
        glm::vec2 translation{};  // moving objects up/down/left/right
        glm::vec2 scale{1.0f, 1.0f};
        float rotation;

        glm::mat2 mat2()
        {
            const float s = glm::sin(rotation);
            const float c = glm::cos(rotation);
            glm::mat2 rotation_matrix{{c, s}, {-s, c}};

            glm::mat2 scale_mat{{scale.x, 0.0f}, {0.0f, scale.y}};   // each element is a column, NOT row (https://youtu.be/gxUcgc88tD4?t=646)

            return (rotation_matrix * scale_mat);
            // return (scale_mat * rotation_matrix);
        }
    };

    class LveGameObject
    {
        public:
            using id_t = unsigned int;

            static LveGameObject createGameObject()
            {
                static id_t current_id = 0;
                return LveGameObject{current_id++};
            }

            LveGameObject(const LveGameObject &) = delete;
            LveGameObject &operator=(const LveGameObject &) = delete;
            LveGameObject(LveGameObject &&) = default;
            LveGameObject &operator=(LveGameObject &&) = default;

            id_t getId() const
            {
                return id_;
            }

            std::shared_ptr<LveModel> model_;
            glm::vec3 color_{};
            Transform2dComponent transform_2d_{};

        private:
            LveGameObject(const id_t obj_id) : id_(obj_id) {}
            id_t id_;   // unique to all game objects

    };
}