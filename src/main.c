#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <signal.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define GLAD_VULKAN_IMPLEMENTATION
#include "lib/glad-vulkan1.4/include/glad/vulkan.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "window.h"
#include "render.h"

#define APP_SHORT_NAME "vkrender"
#define APP_LONG_NAME "Vulkan Render"



int main(const int argc, const char *argv[]) {
    struct windowinfo window;
    struct renderinfo render;
   

    init_render(&window,&render,APP_SHORT_NAME,argc,argv);
    create_window(&window,APP_LONG_NAME);


    //demo_init_vk_swapchain(&demo);

    //demo_prepare(&demo);
    //demo_run(&demo);

    //demo_cleanup(&demo);

    //return validation_error;
}