#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define GLAD_VULKAN_IMPLEMENTATION
#include "lib/glad-vulkan1.4/include/glad/vulkan.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "window.h"



void create_window(struct windowinfo *window, char *APP_LONG_NAME) {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window->window = glfwCreateWindow(window->width,
                                    window->height,
                                    APP_LONG_NAME,
                                    NULL,
                                    NULL);
    if (!window->window) {
        // It didn't work, so try to give a useful error:
        printf("Cannot create a window in which to draw!\n");
        fflush(stdout);
        exit(1);
    }

    //glfwSetWindowUserPointer(demo->window, demo);
    //glfwSetWindowRefreshCallback(demo->window, demo_refresh_callback);
    //glfwSetFramebufferSizeCallback(demo->window, demo_resize_callback);
    //glfwSetKeyCallback(demo->window, demo_key_callback);
}

