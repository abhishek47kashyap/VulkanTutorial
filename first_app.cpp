#include "first_app.hpp"

namespace lve
{
    void FirstApp::run()
    {
        while (!lve_window_.shouldClose())
        {
            glfwPollEvents();
        }
    }
}