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
#define VULKAN_SWAPCHAIN_IMAGES_CAP 3

typedef struct {
  bool same;
  bool graphics_found;
  u32 graphics__index;
  VkQueue graphics__queue;
  bool present_found;
  u32 present__index;
  VkQueue present__queue;
} Vulkan__PhysicalDeviceQueue_t;

typedef struct {
  unsigned int m_requiredDriverExtensionsCount;
  const char* m_requiredDriverExtensions[VULKAN_REQUIRED_DRIVER_EXTENSIONS_CAP];
  unsigned int m_requiredValidationLayersCount;
  const char* m_requiredValidationLayers[VULKAN_REQUIRED_VALIDATION_LAYERS_CAP];
  unsigned int m_requiredPhysicalDeviceExtensionsCount;
  const char* m_requiredPhysicalDeviceExtensions[VULKAN_REQUIRED_PHYSICAL_DEVICE_EXTENSIONS_CAP];

  // instance
  VkInstance m_instance;
  VkPhysicalDevice m_physicalDevice;
  VkSurfaceKHR m_surface;
  VkDevice m_logicalDevice;

  // window
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

  // swapchain
  VkSwapchainKHR m_swapChain;
  VkSurfaceCapabilitiesKHR m_SwapChain__capabilities;
  u32 m_SwapChain__images_count;
  VkImage m_SwapChain__images[VULKAN_SWAPCHAIN_IMAGES_CAP];
  VkImageView m_SwapChain__imageViews[VULKAN_SWAPCHAIN_IMAGES_CAP];
  VkFormat m_SwapChain__imageFormat;
  VkExtent2D m_SwapChain__extent;
  u32 m_SwapChain__formats_count;
  VkSurfaceFormatKHR m_SwapChain__formats[VULKAN_SWAPCHAIN_FORMATS_CAP];
  u32 m_SwapChain__presentModes_count;
  VkPresentModeKHR m_SwapChain__presentModes[VULKAN_SWAPCHAIN_PRESENT_MODES_CAP];
  Vulkan__PhysicalDeviceQueue_t m_SwapChain__queues;

  // pipeline
  VkRenderPass m_renderPass;
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
void Vulkan__CreateSwapChain(Vulkan_t* self, VkSwapchainKHR oldSwapChain);
void Vulkan__CreateImageViews(Vulkan_t* self);
void Vulkan__CreateRenderPass(Vulkan_t* self);

#endif