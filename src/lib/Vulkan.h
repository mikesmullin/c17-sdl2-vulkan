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
#define VULKAN_SHADER_FILE_BUFFER_BYTES_CAP 50 * 1024  // KB

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
  VkFramebuffer m_SwapChain__framebuffers[VULKAN_SWAPCHAIN_IMAGES_CAP];

  // pipeline
  VkRenderPass m_renderPass;
  VkDescriptorSetLayout m_descriptorSetLayout;
  VkBuffer m_vertexBuffers[VULKAN_SWAPCHAIN_IMAGES_CAP];
  VkDeviceMemory m_vertexBufferMemories[VULKAN_SWAPCHAIN_IMAGES_CAP];
  VkPipelineLayout m_pipelineLayout;
  VkPipeline m_graphicsPipeline;
  VkCommandPool m_commandPool;
  VkImage m_textureImage;
  VkDeviceMemory m_textureImageMemory;
  VkImageView m_textureImageView;
  VkSampler m_textureSampler;
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
void Vulkan__CreateDescriptorSetLayout(Vulkan_t* self);
void Vulkan__CreateShaderModule(
    Vulkan_t* self, const u64 size, const char* code, VkShaderModule* shaderModule);
void Vulkan__DestroyShaderModule(Vulkan_t* self, const VkShaderModule* shaderModule);
void Vulkan__CreateGraphicsPipeline(
    Vulkan_t* self,
    const char* frag_shader,
    const char* vert_shader,
    u32 vertexSize,
    u32 instanceSize,
    u8 attrCount,
    u32 bindings[],
    u32 locations[],
    u32 formats[],
    u32 offsets[]);
void Vulkan__CreateFrameBuffers(Vulkan_t* self);
void Vulkan__CreateCommandPool(Vulkan_t* self);
void Vulkan__CreateBuffer(
    Vulkan_t* self,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer* buffer,
    VkDeviceMemory* bufferMemory);
void Vulkan__CreateTextureImage(Vulkan_t* self, const char* file);
u32 Vulkan__FindMemoryType(Vulkan_t* self, u32 typeFilter, VkMemoryPropertyFlags properties);
void Vulkan__BeginSingleTimeCommands(Vulkan_t* self, VkCommandBuffer* commandBuffer);
void Vulkan__EndSingleTimeCommands(Vulkan_t* self, VkCommandBuffer* commandBuffer);
void Vulkan__TransitionImageLayout(
    Vulkan_t* self,
    VkImage* image,
    VkFormat format,
    VkImageLayout oldLayout,
    VkImageLayout newLayout);
void Vulkan__CopyBufferToImage(
    Vulkan_t* self, VkBuffer* buffer, VkImage* image, u32 width, u32 height);
void Vulkan__CreateImage(
    Vulkan_t* self,
    uint32_t width,
    uint32_t height,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkImage* image,
    VkDeviceMemory* imageMemory);
void Vulkan__CreateImageView(
    Vulkan_t* self, VkImage* image, VkFormat format, VkImageView* imageView);
void Vulkan__CreateTextureImageView(Vulkan_t* self);
void Vulkan__CreateTextureSampler(Vulkan_t* self);
void Vulkan__CreateVertexBuffer(Vulkan_t* self, u8 idx, u64 size, const void* indata);

#endif