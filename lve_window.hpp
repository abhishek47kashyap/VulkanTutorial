#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace lve
{
    class LveWindow
    {
        public:
            LveWindow(const int width, const int height, const std::string& name);
            ~LveWindow();

            bool shouldClose();
            VkExtent2D getExtent();
            void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

            // deleting copy operator and copy constructor (https://youtu.be/lr93-_cC8v4?t=601)
            LveWindow(const LveWindow&) = delete;
            LveWindow &operator=(const LveWindow&) = delete;

        private:
            void initWindow();

            const int width_;
            const int height_;

            std::string windowName_;

            GLFWwindow *window_;
    };
}