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
            bool wasWindowResized() const { return frame_buffer_resized_; }
            void resetWindowResizedFlag() { frame_buffer_resized_ = false; }

            // deleting copy operator and copy constructor (https://youtu.be/lr93-_cC8v4?t=601)
            LveWindow(const LveWindow&) = delete;
            LveWindow &operator=(const LveWindow&) = delete;

        private:
            static void frameBufferResizeCallback(GLFWwindow* window, const int width, const int height);
            void initWindow();

            int width_;
            int height_;
            bool frame_buffer_resized_ = false;

            std::string windowName_;

            GLFWwindow *window_;
    };
}