#ifndef VULKAN_H
#define VULKAN_H

#define VK_NO_PROTOTYPES
#include <volk.h>

#include "Base.h"

#define DEBUG_VULKAN

typedef enum {
  VULKAN_ERROR_NONE = 0,
  VULKAN_ERROR_VOLK_INITIALIZE_FAILED = 1,
  VULKAN_ERROR_VK_CREATE_INSTANCE_FAILED = 2,
  VULKAN_ERROR_VK_EILP_COUNT_FAILED = 3,
  VULKAN_ERROR_VK_EILP_COUNT_ZERO = 4,
  VULKAN_ERROR_VK_EILP_READ_FAILED = 5,
  VULKAN_ERROR_VK_EILP_MISSING_REQUIRED = 6,
  VULKAN_ERROR_VK_EIEP_COUNT_FAILED = 7,
  VULKAN_ERROR_VK_EIEP_COUNT_ZERO = 8,
  VULKAN_ERROR_VK_EIEP_READ_FAILED = 9,
  VULKAN_ERROR_VK_EIEP_MISSING_REQUIRED = 10,
  VULKAN_ERROR_VK_EPD_COUNT_FAILED = 11,
  VULKAN_ERROR_VK_EPD_COUNT_ZERO = 12,
  VULKAN_ERROR_VK_EPD_READ_FAILED = 13,
  VULKAN_ERROR_INVALID_DEVICE_IDX = 14,
} Vulkan__Error_t;

extern const char* ckp_Vulkan__ERROR_MESSAGES[];

#define ASPECT_SQUARE 1.0f / 1

#define VULKAN_REQUIRED_VALIDATION_LAYERS_CAP 10
#define VULKAN_REQUIRED_DRIVER_EXTENSIONS_CAP 10

typedef struct {
  unsigned int m_requiredDriverExtensionCount;
  const char* m_requiredDriverExtensions[VULKAN_REQUIRED_DRIVER_EXTENSIONS_CAP];
  unsigned int m_requiredValidationLayerCount;
  const char* m_requiredValidationLayers[VULKAN_REQUIRED_VALIDATION_LAYERS_CAP];
  VkInstance m_instance;
  VkPhysicalDevice m_physicalDevice;
  VkSurfaceKHR m_surface;

  f32 m_aspectRatio;
  // window size may differ (ie. viewport may have fixed aspect
  // ratio, while window has letterbox/pillarbox)
  u32 m_windowWidth;
  u32 m_windowHeight;
  // viewport will upsample/downsample buffer to a particular screen size
  u32 m_viewportX;
  u32 m_viewportY;
  u32 m_viewportWidth;
  u32 m_viewportHeight;
  // buffers may be larger or smaller than they appear on
  // screen (especially HDPI retina)
  u32 m_bufferWidth;
  u32 m_bufferHeight;
  bool m_framebufferResized;
  bool m_minimized;
  bool m_maximized;
} Vulkan_t;

Vulkan__Error_t Vulkan__InitDriver1(Vulkan_t* self);

Vulkan__Error_t Vulkan__AssertDriverValidationLayersSupported(Vulkan_t* self);
Vulkan__Error_t Vulkan__AssertDriverExtensionsSupported(Vulkan_t* self);

Vulkan__Error_t Vulkan__CreateInstance(
    Vulkan_t* self,
    const char* name,
    const char* engineName,
    const unsigned int major,
    const unsigned int minor,
    const unsigned int hotfix);

void Vulkan__InitDriver2(Vulkan_t* self);

void Vulkan__UsePhysicalDevice(Vulkan_t* self, const u8 requiredDeviceIndex);

#endif