#ifndef VULKAN_H
#define VULKAN_H

#define VK_NO_PROTOTYPES
#include <volk.h>

#define DEBUG_VULKAN

typedef enum {
  VULKAN_ERROR_NONE = 0,
  VULKAN_ERROR_VOLK_INITIALIZE_FAILED = 1,
  VULKAN_ERROR_VK_CREATE_INSTANCE_FAILED = 2,
} Vulkan__Error_t;

extern const char* ckp_Vulkan__ERROR_MESSAGES[];

typedef struct {
  unsigned int m_requiredDriverExtensionCount;
  const char** m_requiredDriverExtensions;
  unsigned int m_requiredValidationLayerCount;
  const char** m_requiredValidationLayers;
  VkInstance* m_instance;
} Vulkan_t;

Vulkan__Error_t Vulkan__InitDriver(Vulkan_t* self);

// void Vulkan__RequireValidationLayer(const char* layerName);
// void Vulkan__RequireDriverExtension(const char* layerName);

Vulkan__Error_t Vulkan__CreateInstance(
    Vulkan_t* self,
    const char* name,
    const char* engineName,
    const unsigned int major,
    const unsigned int minor,
    const unsigned int hotfix);

#endif