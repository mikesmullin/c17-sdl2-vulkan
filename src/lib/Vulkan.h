#ifndef VULKAN_H
#define VULKAN_H

#define VK_NO_PROTOTYPES
#include <volk.h>

#include "Base.h"

#define DEBUG_VULKAN

#define ASPECT_SQUARE 1.0f / 1

#define VULKAN_REQUIRED_VALIDATION_LAYERS_CAP 5
#define VULKAN_REQUIRED_DRIVER_EXTENSIONS_CAP 5
#define VULKAN_REQUIRED_PHYSICAL_DEVICE_EXTENSIONS_CAP 5
#define VULKAN_SWAPCHAIN_FORMATS_CAP 10
#define VULKAN_SWAPCHAIN_PRESENT_MODES_CAP 10

typedef struct {
  unsigned int m_requiredDriverExtensionsCount;
  const char* m_requiredDriverExtensions[VULKAN_REQUIRED_DRIVER_EXTENSIONS_CAP];
  unsigned int m_requiredValidationLayersCount;
  const char* m_requiredValidationLayers[VULKAN_REQUIRED_VALIDATION_LAYERS_CAP];
  unsigned int m_requiredPhysicalDeviceExtensionsCount;
  const char* m_requiredPhysicalDeviceExtensions[VULKAN_REQUIRED_PHYSICAL_DEVICE_EXTENSIONS_CAP];

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

  VkSurfaceCapabilitiesKHR m_SwapChain__capabilities;
  VkSurfaceFormatKHR m_SwapChain__formats[VULKAN_SWAPCHAIN_FORMATS_CAP];
  VkPresentModeKHR m_SwapChain__presentModes[VULKAN_SWAPCHAIN_PRESENT_MODES_CAP];
} Vulkan_t;

void Vulkan__InitDriver1(Vulkan_t* self);

void Vulkan__AssertDriverValidationLayersSupported(Vulkan_t* self);
void Vulkan__AssertDriverExtensionsSupported(Vulkan_t* self);

void Vulkan__CreateInstance(
    Vulkan_t* self,
    const char* name,
    const char* engineName,
    const unsigned int major,
    const unsigned int minor,
    const unsigned int hotfix);

void Vulkan__InitDriver2(Vulkan_t* self);

void Vulkan__UsePhysicalDevice(Vulkan_t* self, const u8 requiredDeviceIndex);

void Vulkan__AssertSwapChainSupported(Vulkan_t* self);
void Vulkan__CreateLogicalDeviceAndQueues(Vulkan_t* self);
void Vulkan__CreateSwapChain(Vulkan_t* self);

#endif