#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <assert.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define GLAD_VULKAN_IMPLEMENTATION
#include "lib/glad-vulkan1.4/include/glad/vulkan.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "window.h"
#include "render.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define ERR_EXIT(err_msg, err_class)                                           \
    do {                                                                       \
        printf(err_msg);                                                       \
        fflush(stdout);                                                        \
        exit(1);                                                               \
    } while (0)

int validation_error = 0;

VKAPI_ATTR VkBool32 VKAPI_CALL
BreakCallback(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType,
              uint64_t srcObject, size_t location, int32_t msgCode,
              const char *pLayerPrefix, const char *pMsg,
              void *pUserData) {
#ifdef _WIN32
    DebugBreak();
#else
    raise(SIGTRAP);
#endif

    return false;
}

VKAPI_ATTR VkBool32 VKAPI_CALL
dbgFunc(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType,
    uint64_t srcObject, size_t location, int32_t msgCode,
    const char *pLayerPrefix, const char *pMsg, void *pUserData) {
    char *message = (char *)malloc(strlen(pMsg) + 100);

    assert(message);

    validation_error = 1;

    if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
        sprintf(message, "ERROR: [%s] Code %d : %s", pLayerPrefix, msgCode,
            pMsg);
    } else if (msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
        sprintf(message, "WARNING: [%s] Code %d : %s", pLayerPrefix, msgCode,
            pMsg);
    } else {
        return false;
    }

    printf("%s\n", message);
    fflush(stdout);
    free(message);

    /*
    * false indicates that layer should not bail-out of an
    * API call that had validation failures. This may mean that the
    * app dies inside the driver due to invalid parameter(s).
    * That's what would happen without validation layers, so we'll
    * keep that behavior here.
    */
    return false;
}


void error_callback(int error, const char* description) {
    printf("GLFW error: %s\n", description);
    fflush(stdout);
}

void init_connection(struct windowinfo *window) {
    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        printf("Cannot initialize GLFW.\nExiting ...\n");
        fflush(stdout);
        exit(1);
    }

    if (!glfwVulkanSupported()) {
        printf("GLFW failed to find the Vulkan loader.\nExiting ...\n");
        fflush(stdout);
        exit(1);
    }

    gladLoadVulkanUserPtr(NULL, (GLADuserptrloadfunc) glfwGetInstanceProcAddress, NULL);
}

VkBool32 check_layers(uint32_t check_count, const char **check_names,
                                  uint32_t layer_count,
                                  VkLayerProperties *layers) {
    uint32_t i, j;
    for (i = 0; i < check_count; i++) {
        VkBool32 found = 0;
        for (j = 0; j < layer_count; j++) {
            if (!strcmp(check_names[i], layers[j].layerName)) {
                found = 1;
                break;
            }
        }
        if (!found) {
            fprintf(stderr, "Cannot find layer: %s\n", check_names[i]);
            return 0;
        }
    }
    return 1;
}

void init_vulkan(struct windowinfo *window, struct renderinfo *render, char *APP_SHORT_NAME) {
    VkResult err;
    VkBool32 portability_enumeration = VK_FALSE;
    uint32_t i = 0;
    uint32_t required_extension_count = 0;
    uint32_t instance_extension_count = 0;
    uint32_t instance_layer_count = 0;
    uint32_t validation_layer_count = 0;
    const char **required_extensions = NULL;
    const char **instance_validation_layers = NULL;
    render->enabled_extension_count = 0;
    render->enabled_layer_count = 0;

    char *instance_validation_layers_alt1[] = {
        "VK_LAYER_LUNARG_standard_validation"
    };

    char *instance_validation_layers_alt2[] = {
        "VK_LAYER_GOOGLE_threading",       "VK_LAYER_LUNARG_parameter_validation",
        "VK_LAYER_LUNARG_object_tracker",  "VK_LAYER_LUNARG_image",
        "VK_LAYER_LUNARG_core_validation", "VK_LAYER_LUNARG_swapchain",
        "VK_LAYER_GOOGLE_unique_objects"
    };

    /* Look for validation layers */
    VkBool32 validation_found = 0;
    if (render->validate) {

        err = vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL);
        assert(!err);

        instance_validation_layers = (const char**) instance_validation_layers_alt1;
        if (instance_layer_count > 0) {
            VkLayerProperties *instance_layers =
                    malloc(sizeof (VkLayerProperties) * instance_layer_count);
            err = vkEnumerateInstanceLayerProperties(&instance_layer_count,
                    instance_layers);
            assert(!err);


            validation_found = check_layers(
                    ARRAY_SIZE(instance_validation_layers_alt1),
                    instance_validation_layers, instance_layer_count,
                    instance_layers);
            if (validation_found) {
                render->enabled_layer_count = ARRAY_SIZE(instance_validation_layers_alt1);
                render->enabled_layers[0] = "VK_LAYER_LUNARG_standard_validation";
                validation_layer_count = 1;
            } else {
                // use alternative set of validation layers
                instance_validation_layers =
                    (const char**) instance_validation_layers_alt2;
                render->enabled_layer_count = ARRAY_SIZE(instance_validation_layers_alt2);
                validation_found = check_layers(
                    ARRAY_SIZE(instance_validation_layers_alt2),
                    instance_validation_layers, instance_layer_count,
                    instance_layers);
                validation_layer_count =
                    ARRAY_SIZE(instance_validation_layers_alt2);
                for (i = 0; i < validation_layer_count; i++) {
                    render->enabled_layers[i] = instance_validation_layers[i];
                }
            }
            free(instance_layers);
        }

        if (!validation_found) {
            ERR_EXIT("vkEnumerateInstanceLayerProperties failed to find "
                    "required validation layer.\n\n"
                    "Please look at the Getting Started guide for additional "
                    "information.\n",
                    "vkCreateInstance Failure");
        }
    }

    /* Look for instance extensions */
    required_extensions = glfwGetRequiredInstanceExtensions(&required_extension_count);
    if (!required_extensions) {
        ERR_EXIT("glfwGetRequiredInstanceExtensions failed to find the "
                 "platform surface extensions.\n\nDo you have a compatible "
                 "Vulkan installable client driver (ICD) installed?\nPlease "
                 "look at the Getting Started guide for additional "
                 "information.\n",
                 "vkCreateInstance Failure");
    }

    for (i = 0; i < required_extension_count; i++) {
        render->extension_names[render->enabled_extension_count++] = required_extensions[i];
        assert(render->enabled_extension_count < 64);
    }

    err = vkEnumerateInstanceExtensionProperties(
        NULL, &instance_extension_count, NULL);
    assert(!err);

    if (instance_extension_count > 0) {
        VkExtensionProperties *instance_extensions =
            malloc(sizeof(VkExtensionProperties) * instance_extension_count);
        err = vkEnumerateInstanceExtensionProperties(
            NULL, &instance_extension_count, instance_extensions);
        assert(!err);
        for (i = 0; i < instance_extension_count; i++) {
            if (!strcmp(VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
                        instance_extensions[i].extensionName)) {
                if (render->validate) {
                    render->extension_names[render->enabled_extension_count++] =
                        VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
                }
            }
            assert(render->enabled_extension_count < 64);
            if (!strcmp(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
                        instance_extensions[i].extensionName)) {
                render->extension_names[render->enabled_extension_count++] =
                    VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
                portability_enumeration = VK_TRUE;
            }
            assert(render->enabled_extension_count < 64);
        }

        free(instance_extensions);
    }

    const VkApplicationInfo app = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = NULL,
        .pApplicationName = APP_SHORT_NAME,
        .applicationVersion = 0,
        .pEngineName = APP_SHORT_NAME,
        .engineVersion = 0,
        .apiVersion = VK_API_VERSION_1_0,
    };
    VkInstanceCreateInfo inst_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = NULL,
        .pApplicationInfo = &app,
        .enabledLayerCount = render->enabled_layer_count,
        .ppEnabledLayerNames = (const char *const *)instance_validation_layers,
        .enabledExtensionCount = render->enabled_extension_count,
        .ppEnabledExtensionNames = (const char *const *)render->extension_names,
    };

    if (portability_enumeration)
        inst_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    uint32_t gpu_count;

    err = vkCreateInstance(&inst_info, NULL, &render->inst);
    if (err == VK_ERROR_INCOMPATIBLE_DRIVER) {
        ERR_EXIT("Cannot find a compatible Vulkan installable client driver "
                 "(ICD).\n\nPlease look at the Getting Started guide for "
                 "additional information.\n",
                 "vkCreateInstance Failure");
    } else if (err == VK_ERROR_EXTENSION_NOT_PRESENT) {
        ERR_EXIT("Cannot find a specified extension library"
                 ".\nMake sure your layers path is set appropriately\n",
                 "vkCreateInstance Failure");
    } else if (err) {
        ERR_EXIT("vkCreateInstance failed.\n\nDo you have a compatible Vulkan "
                 "installable client driver (ICD) installed?\nPlease look at "
                 "the Getting Started guide for additional information.\n",
                 "vkCreateInstance Failure");
    }

    gladLoadVulkanUserPtr(NULL, (GLADuserptrloadfunc) glfwGetInstanceProcAddress, render->inst);

    /* Make initial call to query gpu_count, then second call for gpu info*/
    err = vkEnumeratePhysicalDevices(render->inst, &gpu_count, NULL);
    assert(!err && gpu_count > 0);

    if (gpu_count > 0) {
        VkPhysicalDevice *physical_devices =
            malloc(sizeof(VkPhysicalDevice) * gpu_count);
        err = vkEnumeratePhysicalDevices(render->inst, &gpu_count,
                                         physical_devices);
        assert(!err);
        /* For tri demo we just grab the first physical device */
        render->gpu = physical_devices[0];
        free(physical_devices);
    } else {
        ERR_EXIT("vkEnumeratePhysicalDevices reported zero accessible devices."
                 "\n\nDo you have a compatible Vulkan installable client"
                 " driver (ICD) installed?\nPlease look at the Getting Started"
                 " guide for additional information.\n",
                 "vkEnumeratePhysicalDevices Failure");
    }

    gladLoadVulkanUserPtr(render->gpu, (GLADuserptrloadfunc) glfwGetInstanceProcAddress, render->inst);

    /* Look for device extensions */
    uint32_t device_extension_count = 0;
    VkBool32 swapchainExtFound = 0;
    render->enabled_extension_count = 0;

    err = vkEnumerateDeviceExtensionProperties(render->gpu, NULL,
                                               &device_extension_count, NULL);
    assert(!err);

    if (device_extension_count > 0) {
        VkExtensionProperties *device_extensions =
                malloc(sizeof(VkExtensionProperties) * device_extension_count);
        err = vkEnumerateDeviceExtensionProperties(
            render->gpu, NULL, &device_extension_count, device_extensions);
        assert(!err);

        for (i = 0; i < device_extension_count; i++) {
            if (!strcmp(VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                        device_extensions[i].extensionName)) {
                swapchainExtFound = 1;
                render->extension_names[render->enabled_extension_count++] =
                    VK_KHR_SWAPCHAIN_EXTENSION_NAME;
            }
            assert(render->enabled_extension_count < 64);
        }

        free(device_extensions);
    }

    if (!swapchainExtFound) {
        ERR_EXIT("vkEnumerateDeviceExtensionProperties failed to find "
                 "the " VK_KHR_SWAPCHAIN_EXTENSION_NAME
                 " extension.\n\nDo you have a compatible "
                 "Vulkan installable client driver (ICD) installed?\nPlease "
                 "look at the Getting Started guide for additional "
                 "information.\n",
                 "vkCreateInstance Failure");
    }

    if (render->validate) {
        VkDebugReportCallbackCreateInfoEXT dbgCreateInfo;
        dbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
        dbgCreateInfo.flags =
            VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        dbgCreateInfo.pfnCallback = render->use_break ? BreakCallback : dbgFunc;
        dbgCreateInfo.pUserData = render;
        dbgCreateInfo.pNext = NULL;
        err = vkCreateDebugReportCallbackEXT(render->inst, &dbgCreateInfo, NULL,
                                             &render->msg_callback);
        switch (err) {
        case VK_SUCCESS:
            break;
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            ERR_EXIT("CreateDebugReportCallback: out of host memory\n",
                     "CreateDebugReportCallback Failure");
            break;
        default:
            ERR_EXIT("CreateDebugReportCallback: unknown failure\n",
                     "CreateDebugReportCallback Failure");
            break;
        }
    }

    vkGetPhysicalDeviceProperties(render->gpu, &render->gpu_props);

    // Query with NULL data to get count
    vkGetPhysicalDeviceQueueFamilyProperties(render->gpu, &render->queue_count,
                                             NULL);

    render->queue_props = (VkQueueFamilyProperties *)malloc(
        render->queue_count * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(render->gpu, &render->queue_count,
                                             render->queue_props);
    assert(render->queue_count >= 1);

    vkGetPhysicalDeviceFeatures(render->gpu, &render->gpu_features);

    // Graphics queue and MemMgr queue can be separate.
    // TODO: Add support for separate queues, including synchronization,
    //       and appropriate tracking for QueueSubmit
}


void init_render(struct windowinfo *window, struct renderinfo *render, char *APP_SHORT_NAME, const int argc, const char *argv[])
{
    int i;
    memset(window, 0, sizeof(*window));
    render->frameCount = INT32_MAX;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--use_staging") == 0) {
            render->use_staging_buffer = true;
            continue;
        }
        if (strcmp(argv[i], "--break") == 0) {
            render->use_break = true;
            continue;
        }
        if (strcmp(argv[i], "--validate") == 0) {
            render->validate = true;
            continue;
        }
        if (strcmp(argv[i], "--c") == 0 && render->frameCount == INT32_MAX &&
            i < argc - 1 && sscanf(argv[i + 1], "%d", &render->frameCount) == 1 &&
            render->frameCount >= 0) {
            i++;
            continue;
        }

        fprintf(stderr, "Usage:\n  %s [--use_staging] [--validate] [--break] "
                        "[--c <framecount>]\n",
                APP_SHORT_NAME);
        fflush(stderr);
        exit(1);
    }

    init_connection(window);
    init_vulkan(window, render, APP_SHORT_NAME);

    window->width = 500;
    window->height = 500;
    //demo->depthStencil = 1.0;
    //demo->depthIncrement = -0.01f;
}