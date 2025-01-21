#ifndef RENDER_H
#define RENDER_H


struct renderinfo {
   
    int32_t frameCount;
    int32_t curFrame;

    bool validate;
    bool use_break;
    bool use_staging_buffer;


    VkInstance inst;
    VkPhysicalDevice gpu;
    VkDevice device;
    VkQueue queue;
    VkPhysicalDeviceProperties gpu_props;
    VkPhysicalDeviceFeatures gpu_features;
    VkQueueFamilyProperties *queue_props;
    uint32_t graphics_queue_node_index;

    uint32_t enabled_extension_count;
    uint32_t enabled_layer_count;
    const char *extension_names[64];
    const char *enabled_layers[64];
    
    VkDebugReportCallbackEXT msg_callback;

    uint32_t current_buffer;
    uint32_t queue_count;
    
    
    
};

VKAPI_ATTR VkBool32 VKAPI_CALL BreakCallback(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType,
              uint64_t srcObject, size_t location, int32_t msgCode,
              const char *pLayerPrefix, const char *pMsg,
              void *pUserData);

VKAPI_ATTR VkBool32 VKAPI_CALL
dbgFunc(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType,
    uint64_t srcObject, size_t location, int32_t msgCode,
    const char *pLayerPrefix, const char *pMsg, void *pUserData);

void error_callback(int error, const char* description);

void init_connection(struct windowinfo *window);

VkBool32 check_layers(uint32_t check_count, const char **check_names,
                                  uint32_t layer_count,
                                  VkLayerProperties *layers);

void init_vulkan(struct windowinfo *window, struct renderinfo *render, char *APP_SHORT_NAME);

void init_render(struct windowinfo *window, struct renderinfo *render, char *APP_SHORT_NAME, const int argc, const char *argv[]);


#endif