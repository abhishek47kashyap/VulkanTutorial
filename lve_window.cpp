#include "lve_window.hpp"

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

    LveWindow::~LveWindow()
    {
        glfwDestroyWindow(window_);
        glfwTerminate();
    }
}