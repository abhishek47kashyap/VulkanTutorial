#include "first_app.hpp"
#include "simple_render_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>

namespace lve
{
    FirstApp::FirstApp()
    {
        loadGameObjects();
    }

    FirstApp::~FirstApp() {}

    void FirstApp::run()
    {
        SimpleRenderSystem simple_render_system{lve_device_, lve_renderer_.getSwapChainRenderPass()};
        while (!lve_window_.shouldClose())
        {
            glfwPollEvents();

            if (auto command_buffer = lve_renderer_.beginFrame())
            {
                lve_renderer_.beginSwapChainRenderPass(command_buffer);
                simple_render_system.renderGameObjects(command_buffer, game_objects_);
                lve_renderer_.endSwapChainRenderPass(command_buffer);
                lve_renderer_.endFrame();
            }
        }

        vkDeviceWaitIdle(lve_device_.device());
    }

    void FirstApp::loadGameObjects()
    {
        std::vector<LveModel::Vertex> vertices
        {
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
        };

        auto lve_model = std::make_shared<LveModel>(lve_device_, vertices);

        auto triangle = LveGameObject::createGameObject();
        triangle.model_ = lve_model;
        triangle.color_ = {0.1f, 0.8f, 0.1f};  // green
        triangle.transform_2d_.translation.x = 0.2f;
        triangle.transform_2d_.scale = {2.0f, 0.5f};
        triangle.transform_2d_.rotation = 90.0f * (glm::pi<float>() / 180.0);

        game_objects_.push_back(std::move(triangle));
    }
}