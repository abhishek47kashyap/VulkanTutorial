#include "lve_window.hpp"

#include <stdexcept>

namespace lve
{
    LveWindow::LveWindow(const int width, const int height, const std::string& name):
    width_(width),
    height_(height),
    windowName_(name)
    {
        initWindow();
    }

    void LveWindow::initWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window_ = glfwCreateWindow(width_, height_, windowName_.c_str(), nullptr, nullptr);
    }

    bool LveWindow::shouldClose()
    {
        return glfwWindowShouldClose(window_);
    }

    VkExtent2D LveWindow::getExtent()
    {
        return {static_cast<uint32_t>(width_), static_cast<uint32_t>(height_)};
    }

    void LveWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
    {
        if (glfwCreateWindowSurface(instance, window_, nullptr, surface) != VK_SUCCESS)
        {
            throw std::runtime_error("LveWindow::createWindowSurface(); failed to create window");
        }
    }

    LveWindow::~LveWindow()
    {
        glfwDestroyWindow(window_);
        glfwTerminate();
    }
}