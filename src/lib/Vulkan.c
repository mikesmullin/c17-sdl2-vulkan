#include "Vulkan.h"

#include <cglm/cglm.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <string.h>

#define VOLK_IMPLEMENTATION
#include <volk.h>

#include "Base.h"
#include "Shader.h"

void Vulkan__InitDriver1(Vulkan_t* self) {
  self->m_requiredDriverExtensionsCount = 0;
  self->m_requiredValidationLayersCount = 0;
  self->m_requiredPhysicalDeviceExtensionsCount = 0;
  self->m_physicalDevice = VK_NULL_HANDLE;

  self->m_aspectRatio = ASPECT_SQUARE;
  self->m_windowWidth = 0;
  self->m_windowHeight = 0;
  self->m_viewportX = 0;
  self->m_viewportY = 0;
  self->m_viewportWidth = 0;
  self->m_viewportHeight = 0;
  u32 bufferWidth = 0;
  u32 bufferHeight = 0;
  self->m_framebufferResized = false;
  self->m_minimized = false;
  self->m_maximized = false;

  self->m_SwapChain__formats_count = 0;
  self->m_SwapChain__presentModes_count = 0;

  self->m_SwapChain__queues.same = false;
  self->m_SwapChain__queues.graphics_found = false;
  self->m_SwapChain__queues.graphics__index = 0;
  self->m_SwapChain__queues.graphics__queue = NULL;
  self->m_SwapChain__queues.present_found = false;
  self->m_SwapChain__queues.present__index = 0;
  self->m_SwapChain__queues.present__queue = NULL;

  ASSERT(VK_SUCCESS == volkInitialize())
}

static const char* debug_validation_layer1 = "VK_LAYER_KHRONOS_validation";

void Vulkan__AssertDriverValidationLayersSupported(Vulkan_t* self) {
#ifdef DEBUG_VULKAN
  // SDK-provided layer conveniently bundles all useful standard validation
  ASSERT(self->m_requiredValidationLayersCount < VULKAN_REQUIRED_VALIDATION_LAYERS_CAP)
  self->m_requiredValidationLayers[self->m_requiredValidationLayersCount++] =
      debug_validation_layer1;
#endif

  if (self->m_requiredValidationLayersCount > 0) {
    u32 availableLayersCount = 0;
    ASSERT(VK_SUCCESS == vkEnumerateInstanceLayerProperties(&availableLayersCount, NULL))
    ASSERT(availableLayersCount > 0)
    LOG_INFOF("validation layers count: %u", availableLayersCount)
    VkLayerProperties availableLayers[availableLayersCount];
    ASSERT(VK_SUCCESS == vkEnumerateInstanceLayerProperties(&availableLayersCount, availableLayers))

    // print list of validation layers to console
    bool found;
    LOG_INFOF("validation layers:")
    for (u8 i = 0; i < availableLayersCount; i++) {
      found = false;
      for (u8 i2 = 0; i2 < self->m_requiredValidationLayersCount; i2++) {
        if (0 == strcmp(self->m_requiredValidationLayers[i2], availableLayers[i].layerName)) {
          found = true;
          break;
        }
      }
      LOG_INFOF("  %s%s", availableLayers[i].layerName, found ? " (required)" : "")
    }
    // validate the required validation layers are all found
    for (u8 i3 = 0; i3 < self->m_requiredValidationLayersCount; i3++) {
      found = false;
      for (u8 i4 = 0; i4 < availableLayersCount; i4++) {
        if (0 == strcmp(self->m_requiredValidationLayers[i3], availableLayers[i4].layerName)) {
          found = true;
          break;
        }
      }

      ASSERT_CONTEXT(
          found,
          "A required validation layer is missing. Layer: %s",
          self->m_requiredValidationLayers[i3])
    }
  }
}

void Vulkan__AssertDriverExtensionsSupported(Vulkan_t* self) {
  // list the extensions supported by this driver
  u32 availableExtensionCount = 0;
  ASSERT(VK_SUCCESS == vkEnumerateInstanceExtensionProperties(NULL, &availableExtensionCount, NULL))
  ASSERT(availableExtensionCount > 0)
  LOG_INFOF("driver extensions count: %u", availableExtensionCount)
  VkExtensionProperties availableExtensions[availableExtensionCount];
  ASSERT(
      VK_SUCCESS ==
      vkEnumerateInstanceExtensionProperties(NULL, &availableExtensionCount, availableExtensions))

  // print list of extensions to console
  bool found;
  LOG_INFOF("driver extensions:")
  for (u8 i = 0; i < availableExtensionCount; i++) {
    found = false;
    for (u8 i2 = 0; i2 < self->m_requiredDriverExtensionsCount; i2++) {
      if (0 == strcmp(self->m_requiredDriverExtensions[i2], availableExtensions[i].extensionName)) {
        found = true;
        break;
      }
    }
    LOG_INFOF("  %s%s", availableExtensions[i].extensionName, found ? " (required)" : "");
  }

  // validate the required extensions are all found
  for (u8 i3 = 0; i3 < self->m_requiredDriverExtensionsCount; i3++) {
    found = false;
    for (u8 i4 = 0; i4 < availableExtensionCount; i4++) {
      if (0 ==
          strcmp(self->m_requiredDriverExtensions[i3], availableExtensions[i4].extensionName)) {
        found = true;
        break;
      }
    }

    ASSERT_CONTEXT(
        found,
        "A required driver extension is missing. Extension: %s",
        self->m_requiredDriverExtensions[i3])
  }
}

void Vulkan__CreateInstance(
    Vulkan_t* self,
    const char* name,
    const char* engineName,
    const unsigned int major,
    const unsigned int minor,
    const unsigned int hotfix) {
  VkInstanceCreateInfo createInfo;
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pNext = NULL;
#if OS_MAC == 1
  createInfo.flags =
      // enable MoltenVK support for MacOS cross-platform support
      VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#else
  createInfo.flags = 0;  // default
#endif
  VkApplicationInfo appInfo;
  createInfo.pApplicationInfo = &appInfo;
  {
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = NULL;
    appInfo.pApplicationName = name;
    appInfo.applicationVersion = VK_MAKE_VERSION(major, minor, hotfix);
    appInfo.pEngineName = engineName;
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;
  }

  createInfo.enabledLayerCount = self->m_requiredValidationLayersCount;
  createInfo.ppEnabledLayerNames = self->m_requiredValidationLayers;
  createInfo.enabledExtensionCount = self->m_requiredDriverExtensionsCount;
  createInfo.ppEnabledExtensionNames = self->m_requiredDriverExtensions;

  ASSERT(VK_SUCCESS == vkCreateInstance(&createInfo, NULL, &self->m_instance))
}

void Vulkan__InitDriver2(Vulkan_t* self) {
  volkLoadInstance(self->m_instance);
}

void Vulkan__UsePhysicalDevice(Vulkan_t* self, const u8 requiredDeviceIndex) {
  // list GPUs
  u32 deviceCount = 0;
  ASSERT(VK_SUCCESS == vkEnumeratePhysicalDevices(self->m_instance, &deviceCount, NULL))
  ASSERT(deviceCount > 0)
  LOG_INFOF("device count: %u", deviceCount)
  VkPhysicalDevice devices[deviceCount];
  ASSERT(VK_SUCCESS == vkEnumeratePhysicalDevices(self->m_instance, &deviceCount, devices))

  // print all GPUs found
  LOG_INFOF("devices:")
  for (u8 i = 0; i < deviceCount; i++) {
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(devices[i], &deviceFeatures);

    // assumption: we only want the type of GPU device used to render video games
    const bool discrete = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    const bool geometry = deviceFeatures.geometryShader;

    // list each GPU device found
    LOG_INFOF(
        "  %u: %s%s%s%s",
        i,
        deviceProperties.deviceName,
        i == requiredDeviceIndex ? " (selected)" : "",
        discrete ? " DISCRETE" : "",
        geometry ? " GEOMETRY_SHADER" : "");

    // select one GPU to be the active/default/current for all subsequent Vulkan methods;
    // it must meet certain minimum requirements
    if (i == requiredDeviceIndex /*&& discrete*/ /*&& geometry*/) {
      self->m_physicalDevice = devices[i];
      return;
    }
  }

  ASSERT_CONTEXT(
      false,
      "The requested device was not available. deviceIdx: %u",
      requiredDeviceIndex)
}

static const char* specialPhysicalExtension1 = "VK_KHR_portability_subset";

void Vulkan__AssertSwapChainSupported(Vulkan_t* self) {
  ASSERT(
      self->m_requiredPhysicalDeviceExtensionsCount <=
      VULKAN_REQUIRED_PHYSICAL_DEVICE_EXTENSIONS_CAP)
  self->m_requiredPhysicalDeviceExtensions[self->m_requiredPhysicalDeviceExtensionsCount++] =
      VK_KHR_SWAPCHAIN_EXTENSION_NAME;

  // list the extensions supported by this physical device
  u32 availablePhysicalExtensionCount = 0;
  ASSERT(
      VK_SUCCESS == vkEnumerateDeviceExtensionProperties(
                        self->m_physicalDevice,
                        NULL,
                        &availablePhysicalExtensionCount,
                        NULL))
  ASSERT(availablePhysicalExtensionCount > 0)
  LOG_INFOF("physical device extensions count: %u", availablePhysicalExtensionCount)
  VkExtensionProperties availablePhysicalExtensions[availablePhysicalExtensionCount];
  ASSERT(
      VK_SUCCESS == vkEnumerateDeviceExtensionProperties(
                        self->m_physicalDevice,
                        NULL,
                        &availablePhysicalExtensionCount,
                        availablePhysicalExtensions))

  // quirk: vulkan spec says if this is supported, we must request it
  for (u8 i = 0; i < self->m_requiredPhysicalDeviceExtensionsCount; i++) {
    if (0 == strcmp(specialPhysicalExtension1, self->m_requiredPhysicalDeviceExtensions[i])) {
      self->m_requiredPhysicalDeviceExtensions[self->m_requiredPhysicalDeviceExtensionsCount++] =
          specialPhysicalExtension1;
    }
  }

  // print list of extensions to console
  LOG_INFOF("required device extensions:")
  // validate the required extensions are all found
  bool found = false;
  for (u8 i2 = 0; i2 < self->m_requiredPhysicalDeviceExtensionsCount; i2++) {
    found = false;
    for (u8 i3 = 0; i3 < availablePhysicalExtensionCount; i3++) {
      if (0 == strcmp(
                   self->m_requiredPhysicalDeviceExtensions[i2],
                   availablePhysicalExtensions[i3].extensionName)) {
        found = true;
        break;
      }
    }

    LOG_INFOF(
        "  %s%s",
        self->m_requiredPhysicalDeviceExtensions[i2],
        found ? " (required)" : " (missing)")

    ASSERT_CONTEXT(
        found,
        "A required physical device extension is missing. Extension: %s",
        self->m_requiredPhysicalDeviceExtensions[i2])
  }

  ASSERT(
      VK_SUCCESS == vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                        self->m_physicalDevice,
                        self->m_surface,
                        &self->m_SwapChain__capabilities))

  self->m_SwapChain__formats_count = 0;
  ASSERT(
      VK_SUCCESS == vkGetPhysicalDeviceSurfaceFormatsKHR(
                        self->m_physicalDevice,
                        self->m_surface,
                        &self->m_SwapChain__formats_count,
                        NULL))
  ASSERT(self->m_SwapChain__formats_count > 0)
  ASSERT(self->m_SwapChain__formats_count <= VULKAN_SWAPCHAIN_FORMATS_CAP)
  LOG_INFOF("physical device surface formats count: %u", self->m_SwapChain__formats_count);
  ASSERT(
      VK_SUCCESS == vkGetPhysicalDeviceSurfaceFormatsKHR(
                        self->m_physicalDevice,
                        self->m_surface,
                        &self->m_SwapChain__formats_count,
                        self->m_SwapChain__formats))

  self->m_SwapChain__presentModes_count = 0;
  ASSERT(
      VK_SUCCESS == vkGetPhysicalDeviceSurfacePresentModesKHR(
                        self->m_physicalDevice,
                        self->m_surface,
                        &self->m_SwapChain__presentModes_count,
                        NULL))
  ASSERT(self->m_SwapChain__presentModes_count > 0)
  ASSERT(self->m_SwapChain__presentModes_count <= VULKAN_SWAPCHAIN_PRESENT_MODES_CAP)
  LOG_INFOF(
      "physical device surface present modes count: %u",
      self->m_SwapChain__presentModes_count);
  ASSERT(
      VK_SUCCESS == vkGetPhysicalDeviceSurfacePresentModesKHR(
                        self->m_physicalDevice,
                        self->m_surface,
                        &self->m_SwapChain__presentModes_count,
                        self->m_SwapChain__presentModes))
}

void Vulkan__CreateLogicalDeviceAndQueues(Vulkan_t* self) {
  ASSERT(VK_NULL_HANDLE != self->m_physicalDevice)

  // enumerate the queue families on current physical device
  ASSERT(self->m_surface)

  u32 queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(self->m_physicalDevice, &queueFamilyCount, NULL);
  VkQueueFamilyProperties queueFamilies[queueFamilyCount];
  vkGetPhysicalDeviceQueueFamilyProperties(
      self->m_physicalDevice,
      &queueFamilyCount,
      queueFamilies);

  // list all queue families found on the current physical device
  LOG_DEBUGF("device queue families:");
  for (u8 i = 0; i < queueFamilyCount; i++) {
    const bool graphics = queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;
    const bool compute = queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT;
    const bool transfer = queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT;
    const bool sparse = queueFamilies[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT;
    const bool protect = queueFamilies[i].queueFlags & VK_QUEUE_PROTECTED_BIT;
    // if VK_KHR_video_decode_queue extension:
    // const bool video_decode = queueFamilies[i].queueFlags & VK_QUEUE_VIDEO_DECODE_BIT;
    // ifdef VK_ENABLE_BETA_EXTENSIONS:
    // const bool video_encode = queueFamilies[i].queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT;
    const bool optical = queueFamilies[i].queueFlags & VK_QUEUE_OPTICAL_FLOW_BIT_NV;

    // Query if presentation is supported
    VkBool32 present = false;
    ASSERT_CONTEXT(
        VK_SUCCESS == vkGetPhysicalDeviceSurfaceSupportKHR(
                          self->m_physicalDevice,
                          i,
                          self->m_surface,
                          &present),
        "queueFamilyIndex: %u",
        i);
    // ASSERT_CONTEXT(present, "queue not present. queueFamilyIndex: %u", i);

    // strategy: select fewest family indices where required queues are present
    if (!self->m_SwapChain__queues.same) {
      if (graphics && present) {
        // prioritize any queue with both
        self->m_SwapChain__queues.same = true;
        self->m_SwapChain__queues.graphics_found = true;
        self->m_SwapChain__queues.present_found = true;
        self->m_SwapChain__queues.graphics__index = i;
        self->m_SwapChain__queues.present__index = i;
      } else if (graphics) {
        self->m_SwapChain__queues.same = false;
        self->m_SwapChain__queues.graphics_found = true;
        self->m_SwapChain__queues.graphics__index = i;
      } else if (present) {
        self->m_SwapChain__queues.same = false;
        self->m_SwapChain__queues.present_found = true;
        self->m_SwapChain__queues.present__index = i;
      }
    }

    LOG_INFOF(
        "  %u: flags:%s%s%s%s%s%s%s",
        i,
        present ? " PRESENT" : "",
        graphics ? " GRAPHICS" : "",
        compute ? " COMPUTE" : "",
        transfer ? " TRANSFER" : "",
        sparse ? " SPARSE" : "",
        protect ? " PROTECT" : "",
        optical ? " OPTICAL" : "");
  }

  ASSERT_CONTEXT(
      self->m_SwapChain__queues.graphics_found,
      "GRAPHICS queue not found within queue families of physical device.")

  ASSERT_CONTEXT(
      self->m_SwapChain__queues.present_found,
      "PRESENT queue not found within queue families of physical device.")

  if (self->m_SwapChain__queues.same) {
    LOG_INFOF(
        "will choose queue %u because it has both GRAPHICS and PRESENT families",
        self->m_SwapChain__queues.graphics__index)
  } else {
    LOG_INFOF(
        "will choose queue %u because it has GRAPHICS family",
        self->m_SwapChain__queues.graphics__index)
    LOG_INFOF(
        "will choose queue %u because it has PRESENT family",
        self->m_SwapChain__queues.present__index)
  }

  // for each unique queue family index,
  // construct a request for pointer to its VkQueue
  const float queuePriority = 1.0f;
  u32 queueCreateInfosCount = self->m_SwapChain__queues.same ? 1 : 2;
  VkDeviceQueueCreateInfo queueCreateInfos[queueCreateInfosCount];
  {
    VkDeviceQueueCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    createInfo.queueFamilyIndex = self->m_SwapChain__queues.graphics__index,
    createInfo.queueCount = 1;
    createInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos[0] = createInfo;
  }
  if (!self->m_SwapChain__queues.same) {
    VkDeviceQueueCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    createInfo.queueFamilyIndex = self->m_SwapChain__queues.present__index,
    createInfo.queueCount = 1;
    createInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos[2] = createInfo;
  }

  VkDeviceCreateInfo createInfo;
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.queueCreateInfoCount = queueCreateInfosCount;
  createInfo.pQueueCreateInfos = queueCreateInfos;

  // per-device validation layers
  // deprecated, because we also defined it on the VkInstance,
  // but recommended to also keep here for backward-compatibility.
  createInfo.enabledLayerCount = (u32)self->m_requiredValidationLayersCount;
  createInfo.ppEnabledLayerNames = self->m_requiredValidationLayers;

  createInfo.enabledExtensionCount = (u32)self->m_requiredPhysicalDeviceExtensionsCount;
  createInfo.ppEnabledExtensionNames = self->m_requiredPhysicalDeviceExtensions;

  VkPhysicalDeviceFeatures deviceFeatures;
  createInfo.pEnabledFeatures = &deviceFeatures;
  deviceFeatures.robustBufferAccess = VK_FALSE;
  deviceFeatures.fullDrawIndexUint32 = VK_FALSE;
  deviceFeatures.imageCubeArray = VK_FALSE;
  deviceFeatures.independentBlend = VK_FALSE;
  deviceFeatures.geometryShader = VK_FALSE;
  deviceFeatures.tessellationShader = VK_FALSE;
  deviceFeatures.sampleRateShading = VK_FALSE;
  deviceFeatures.dualSrcBlend = VK_FALSE;
  deviceFeatures.logicOp = VK_FALSE;
  deviceFeatures.multiDrawIndirect = VK_FALSE;
  deviceFeatures.drawIndirectFirstInstance = VK_FALSE;
  deviceFeatures.depthClamp = VK_FALSE;
  deviceFeatures.depthBiasClamp = VK_FALSE;
  deviceFeatures.fillModeNonSolid = VK_FALSE;
  deviceFeatures.depthBounds = VK_FALSE;
  deviceFeatures.wideLines = VK_FALSE;
  deviceFeatures.largePoints = VK_FALSE;
  deviceFeatures.alphaToOne = VK_FALSE;
  deviceFeatures.multiViewport = VK_FALSE;
  deviceFeatures.samplerAnisotropy = VK_TRUE;
  deviceFeatures.textureCompressionETC2 = VK_FALSE;
  deviceFeatures.textureCompressionASTC_LDR = VK_FALSE;
  deviceFeatures.textureCompressionBC = VK_FALSE;
  deviceFeatures.occlusionQueryPrecise = VK_FALSE;
  deviceFeatures.pipelineStatisticsQuery = VK_FALSE;
  deviceFeatures.vertexPipelineStoresAndAtomics = VK_FALSE;
  deviceFeatures.fragmentStoresAndAtomics = VK_FALSE;
  deviceFeatures.shaderTessellationAndGeometryPointSize = VK_FALSE;
  deviceFeatures.shaderImageGatherExtended = VK_FALSE;
  deviceFeatures.shaderStorageImageExtendedFormats = VK_FALSE;
  deviceFeatures.shaderStorageImageMultisample = VK_FALSE;
  deviceFeatures.shaderStorageImageReadWithoutFormat = VK_FALSE;
  deviceFeatures.shaderStorageImageWriteWithoutFormat = VK_FALSE;
  deviceFeatures.shaderUniformBufferArrayDynamicIndexing = VK_FALSE;
  deviceFeatures.shaderSampledImageArrayDynamicIndexing = VK_FALSE;
  deviceFeatures.shaderStorageBufferArrayDynamicIndexing = VK_FALSE;
  deviceFeatures.shaderStorageImageArrayDynamicIndexing = VK_FALSE;
  deviceFeatures.shaderClipDistance = VK_FALSE;
  deviceFeatures.shaderCullDistance = VK_FALSE;
  deviceFeatures.shaderFloat64 = VK_FALSE;
  deviceFeatures.shaderInt64 = VK_FALSE;
  deviceFeatures.shaderInt16 = VK_FALSE;
  deviceFeatures.shaderResourceResidency = VK_FALSE;
  deviceFeatures.shaderResourceMinLod = VK_FALSE;
  deviceFeatures.sparseBinding = VK_FALSE;
  deviceFeatures.sparseResidencyBuffer = VK_FALSE;
  deviceFeatures.sparseResidencyImage2D = VK_FALSE;
  deviceFeatures.sparseResidencyImage3D = VK_FALSE;
  deviceFeatures.sparseResidency2Samples = VK_FALSE;
  deviceFeatures.sparseResidency4Samples = VK_FALSE;
  deviceFeatures.sparseResidency8Samples = VK_FALSE;
  deviceFeatures.sparseResidency16Samples = VK_FALSE;
  deviceFeatures.sparseResidencyAliased = VK_FALSE;
  deviceFeatures.variableMultisampleRate = VK_FALSE;
  deviceFeatures.inheritedQueries = VK_FALSE;

  ASSERT(
      VK_SUCCESS ==
      vkCreateDevice(self->m_physicalDevice, &createInfo, NULL, &self->m_logicalDevice))

  LOG_INFOF("created logical device")

  vkGetDeviceQueue(
      self->m_logicalDevice,
      self->m_SwapChain__queues.graphics__index,
      0,
      &self->m_SwapChain__queues.graphics__queue);

  if (!self->m_SwapChain__queues.same) {
    vkGetDeviceQueue(
        self->m_logicalDevice,
        self->m_SwapChain__queues.present__index,
        0,
        &self->m_SwapChain__queues.present__queue);
  }
}

void Vulkan__CreateSwapChain(Vulkan_t* self, VkSwapchainKHR priorSwapChain) {
  bool found1 = false;
  VkSurfaceFormatKHR format;

  for (u8 i = 0; i < self->m_SwapChain__formats_count; i++) {
    if (self->m_SwapChain__formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
        self->m_SwapChain__formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      format = self->m_SwapChain__formats[i];
      found1 = true;
      break;
    }
  }
  ASSERT_CONTEXT(
      found1,
      "Couldn't locate physical device support for the swap chain format we wanted. "
      "format: VK_FORMAT_B8G8R8A8_SRGB, "
      "colorSpace: VK_COLOR_SPACE_SRGB_NONLINEAR_KHR")

  bool found2 = false;
  VkPresentModeKHR mode;
  for (u8 i = 0; i < self->m_SwapChain__presentModes_count; i++) {
    if (self->m_SwapChain__presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
      mode = self->m_SwapChain__presentModes[i];
      found2 = true;
      break;
    }
  }
  if (!found2) {
    mode = VK_PRESENT_MODE_FIFO_KHR;
  }

  VkExtent2D extent;
  extent.width = self->m_bufferWidth;
  extent.height = self->m_bufferHeight;

  self->m_SwapChain__images_count = MATH_CLAMP(
      self->m_SwapChain__capabilities.minImageCount + 1,
      self->m_SwapChain__capabilities.minImageCount,
      self->m_SwapChain__capabilities.maxImageCount);

  VkSwapchainCreateInfoKHR createInfo;
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = self->m_surface;
  createInfo.minImageCount = self->m_SwapChain__images_count;
  createInfo.imageFormat = format.format;
  createInfo.imageColorSpace = format.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  if (self->m_SwapChain__queues.same) {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;   // default
    createInfo.pQueueFamilyIndices = NULL;  // default
  } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    const u32 indicies[] = {
        self->m_SwapChain__queues.graphics__index,
        self->m_SwapChain__queues.present__index,
    };
    createInfo.pQueueFamilyIndices = indicies;
  }
  createInfo.preTransform = self->m_SwapChain__capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = mode;
  createInfo.clipped = VK_TRUE;
  createInfo.oldSwapchain = (NULL == priorSwapChain ? VK_NULL_HANDLE : priorSwapChain);

  ASSERT(
      VK_SUCCESS ==
      vkCreateSwapchainKHR(self->m_logicalDevice, &createInfo, NULL, &self->m_swapChain))

  u32 receivedImageCount = 0;
  ASSERT(
      VK_SUCCESS ==
      vkGetSwapchainImagesKHR(self->m_logicalDevice, self->m_swapChain, &receivedImageCount, NULL))
  ASSERT_CONTEXT(
      receivedImageCount <= VULKAN_SWAPCHAIN_IMAGES_CAP,
      "Mismatch in swap chain image count. available: %u, capacity: %u",
      receivedImageCount,
      VULKAN_SWAPCHAIN_IMAGES_CAP)
  ASSERT_CONTEXT(
      VK_SUCCESS == vkGetSwapchainImagesKHR(
                        self->m_logicalDevice,
                        self->m_swapChain,
                        &receivedImageCount,
                        self->m_SwapChain__images),
      "imageCount: %u",
      receivedImageCount)

  self->m_SwapChain__imageFormat = format.format;
  self->m_SwapChain__extent = extent;

  LOG_INFOF(
      "swap chain %screated. width %u height %u imageCount %u",
      (NULL == priorSwapChain ? "" : "re"),
      extent.width,
      extent.height,
      receivedImageCount);
}

void Vulkan__CreateImageViews(Vulkan_t* self) {
  for (u8 i = 0; i < self->m_SwapChain__images_count; i++) {
    VkImageViewCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = self->m_SwapChain__images[i];
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = self->m_SwapChain__imageFormat;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    ASSERT(
        VK_SUCCESS == vkCreateImageView(
                          self->m_logicalDevice,
                          &createInfo,
                          NULL,
                          &self->m_SwapChain__imageViews[i]));
  }
}

void Vulkan__CreateRenderPass(Vulkan_t* self) {
  VkAttachmentDescription colorAttachment;
  {
    VkAttachmentDescriptionFlags flags = VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT;
    colorAttachment.flags = flags;
    colorAttachment.format = self->m_SwapChain__imageFormat;
    // TODO: we're not doing anything with multisampling yet, so we'll stick to 1 sample.
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  }

  VkAttachmentReference colorAttachmentRef[1];
  colorAttachmentRef[0].attachment = 0;
  colorAttachmentRef[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass;
  {
    VkSubpassDescriptionFlags flags;
    subpass.flags = flags;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = NULL;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = colorAttachmentRef;
    subpass.pResolveAttachments = NULL;
    subpass.pDepthStencilAttachment = NULL;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = NULL;
  }

  VkSubpassDependency dependency;
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkRenderPassCreateInfo renderPassInfo;
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.pNext = NULL;
  VkRenderPassCreateFlags flags = VK_RENDER_PASS_CREATE_TRANSFORM_BIT_QCOM;
  renderPassInfo.flags = flags;
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments = &colorAttachment;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  ASSERT(
      VK_SUCCESS ==
      vkCreateRenderPass(self->m_logicalDevice, &renderPassInfo, NULL, &self->m_renderPass))
}

void Vulkan__CreateDescriptorSetLayout(Vulkan_t* self) {
  VkDescriptorSetLayoutBinding uboLayoutBinding;
  uboLayoutBinding.binding = 0;
  uboLayoutBinding.descriptorCount = 1;
  uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uboLayoutBinding.pImmutableSamplers = NULL;
  uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  VkDescriptorSetLayoutBinding samplerLayoutBinding;
  samplerLayoutBinding.binding = 1;
  samplerLayoutBinding.descriptorCount = 1;
  samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  samplerLayoutBinding.pImmutableSamplers = NULL;
  samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  u32 bindingsCount = 2;
  VkDescriptorSetLayoutBinding bindings[bindingsCount];
  bindings[0] = uboLayoutBinding;
  bindings[1] = samplerLayoutBinding;

  VkDescriptorSetLayoutCreateInfo layoutInfo;
  VkDescriptorSetLayoutCreateFlags flags;
  layoutInfo.flags = flags;
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = bindingsCount;
  layoutInfo.pBindings = bindings;

  ASSERT(
      VK_SUCCESS == vkCreateDescriptorSetLayout(
                        self->m_logicalDevice,
                        &layoutInfo,
                        NULL,
                        &self->m_descriptorSetLayout))
}

void Vulkan__CreateShaderModule(
    Vulkan_t* self, const u64 size, const char* code, VkShaderModule* shaderModule) {
  VkShaderModuleCreateInfo createInfo;
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = size;
  createInfo.pCode = (u32*)code;
  ASSERT(VK_SUCCESS == vkCreateShaderModule(self->m_logicalDevice, &createInfo, NULL, shaderModule))
}

void Vulkan__DestroyShaderModule(Vulkan_t* self, const VkShaderModule* shaderModule) {
  vkDestroyShaderModule(self->m_logicalDevice, *shaderModule, NULL);
}

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
    u32 offsets[]) {
  const char shader1[VULKAN_SHADER_FILE_BUFFER_BYTES_CAP];
  u64 len1 = Shader__ReadFile((char*)&shader1, frag_shader);
  const char shader2[VULKAN_SHADER_FILE_BUFFER_BYTES_CAP];
  u64 len2 = Shader__ReadFile((char*)&shader2, vert_shader);

  VkShaderModule vertShaderModule, fragShaderModule;
  Vulkan__CreateShaderModule(self, len1, shader1, &fragShaderModule);
  Vulkan__CreateShaderModule(self, len2, shader2, &vertShaderModule);

  VkPipelineShaderStageCreateInfo vertShaderStageInfo;
  vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vertShaderModule;
  vertShaderStageInfo.pName = "main";
  // TODO: provide constants at shader stage init via .pSpecializationInfo

  VkPipelineShaderStageCreateInfo fragShaderStageInfo;
  fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragShaderModule;
  fragShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

  // utilize dynamic viewport and scissor state, so resize can be specified at draw time
  u32 dynamicStatesCount = 2;
  VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
  VkPipelineDynamicStateCreateInfo dynamicState;
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = dynamicStatesCount;
  dynamicState.pDynamicStates = dynamicStates;

  u32 bindingDescriptionsCount = 2;
  VkVertexInputBindingDescription bindingDescriptions[] = {
      {
          .binding = 0,
          .stride = vertexSize,
          .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
      },
      {
          .binding = 1,
          .stride = instanceSize,
          .inputRate = VK_VERTEX_INPUT_RATE_INSTANCE,
      },
  };

  VkVertexInputAttributeDescription attributeDescriptions[attrCount];
  for (u8 i = 0; i < attrCount; i++) {
    attributeDescriptions[i].binding = bindings[i];
    attributeDescriptions[i].location = locations[i];
    attributeDescriptions[i].format = (VkFormat)(formats[i]);
    // VK_FORMAT_R32G32_SFLOAT;  // 103
    // VK_FORMAT_R32G32B32_SFLOAT;  // 106
    // VK_FORMAT_R32G32B32A32_SFLOAT;  // 109
    // VK_FORMAT_R32_UINT; // 98
    // VK_FORMAT_R32G32B32_SFLOAT; // 106
    // VK_FORMAT_R32_SFLOAT; // 100
    attributeDescriptions[i].offset = offsets[i];
  }

  VkPipelineVertexInputStateCreateInfo vertexInputInfo;
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount = bindingDescriptionsCount;
  vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions;
  vertexInputInfo.vertexAttributeDescriptionCount = attrCount;
  vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;

  // specify what kind of geometry will be drawn from the vertices, and
  // if primitive restart should be enabled.
  VkPipelineInputAssemblyStateCreateInfo inputAssembly;
  inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  VkPipelineViewportStateCreateInfo viewportState;
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.scissorCount = 1;

  // Rasterizer
  VkPipelineRasterizationStateCreateInfo rasterizer;
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  // TODO: which of these do I want to keep?
  // rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;
  rasterizer.depthBiasConstantFactor = 0.0f;
  rasterizer.depthBiasClamp = 0.0f;
  rasterizer.depthBiasSlopeFactor = 0.0f;

  VkPipelineMultisampleStateCreateInfo multisampling;
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling.minSampleShading = 1.0f;
  multisampling.pSampleMask = NULL;
  multisampling.alphaToCoverageEnable = VK_FALSE;
  multisampling.alphaToOneEnable = VK_FALSE;

  // TODO: configure the depth and stencil tests using VkPipelineDepthStencilStateCreateInfo

  VkPipelineColorBlendAttachmentState colorBlendAttachment;
  colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_TRUE;
  colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
  colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

  VkPipelineColorBlendStateCreateInfo colorBlending;
  colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 1.0f;
  colorBlending.blendConstants[1] = 1.0f;
  colorBlending.blendConstants[2] = 1.0f;
  colorBlending.blendConstants[3] = 1.0f;

  VkPipelineLayoutCreateInfo pipelineLayoutInfo;
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 1;
  pipelineLayoutInfo.pSetLayouts = &self->m_descriptorSetLayout;

  ASSERT(
      VK_SUCCESS == vkCreatePipelineLayout(
                        self->m_logicalDevice,
                        &pipelineLayoutInfo,
                        NULL,
                        &self->m_pipelineLayout))

  VkGraphicsPipelineCreateInfo pipelineInfo;
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pDepthStencilState = NULL;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = &dynamicState;
  pipelineInfo.layout = self->m_pipelineLayout;
  pipelineInfo.renderPass = self->m_renderPass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
  pipelineInfo.basePipelineIndex = -1;

  ASSERT(
      VK_SUCCESS == vkCreateGraphicsPipelines(
                        self->m_logicalDevice,
                        VK_NULL_HANDLE,
                        1,
                        &pipelineInfo,
                        NULL,
                        &self->m_graphicsPipeline))

  Vulkan__DestroyShaderModule(self, &vertShaderModule);
  Vulkan__DestroyShaderModule(self, &fragShaderModule);
}

void Vulkan__CreateFrameBuffers(Vulkan_t* self) {
  for (size_t i = 0; i < self->m_SwapChain__images_count; i++) {
    VkImageView attachments[] = {self->m_SwapChain__imageViews[i]};

    VkFramebufferCreateInfo framebufferInfo;
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.pNext = NULL;
    framebufferInfo.renderPass = self->m_renderPass;
    // TODO: couldn't/shouldn't we have one framebuffer, with two attachments?
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = self->m_SwapChain__extent.width;
    framebufferInfo.height = self->m_SwapChain__extent.height;
    framebufferInfo.layers = 1;

    ASSERT(
        VK_SUCCESS == vkCreateFramebuffer(
                          self->m_logicalDevice,
                          &framebufferInfo,
                          NULL,
                          &self->m_SwapChain__framebuffers[i]))
  }
}

void Vulkan__CreateCommandPool(Vulkan_t* self) {
  VkCommandPoolCreateInfo poolInfo;
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.pNext = NULL;
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  poolInfo.queueFamilyIndex = self->m_SwapChain__queues.graphics__index;

  ASSERT(
      VK_SUCCESS ==
      vkCreateCommandPool(self->m_logicalDevice, &poolInfo, NULL, &self->m_commandPool))
}

u32 Vulkan__FindMemoryType(Vulkan_t* self, u32 typeFilter, VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(self->m_physicalDevice, &memProperties);

  for (u32 i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) &&
        (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }

  ASSERT_CONTEXT(false, "failed to find suitable memory type!");
}

void Vulkan__CreateBuffer(
    Vulkan_t* self,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer* buffer,
    VkDeviceMemory* bufferMemory) {
  VkBufferCreateInfo bufferInfo;
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.pNext = NULL;
  bufferInfo.flags = 0;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  ASSERT(VK_SUCCESS == vkCreateBuffer(self->m_logicalDevice, &bufferInfo, NULL, buffer))

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(self->m_logicalDevice, *buffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo;
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.pNext = NULL;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex =
      Vulkan__FindMemoryType(self, memRequirements.memoryTypeBits, properties);

  // TODO: The maximum number of simultaneous memory allocations is limited by the
  // maxMemoryAllocationCount So when objects on screen become too numerous, create a custom
  // allocator
  ASSERT(VK_SUCCESS == vkAllocateMemory(self->m_logicalDevice, &allocInfo, NULL, bufferMemory))

  vkBindBufferMemory(self->m_logicalDevice, *buffer, *bufferMemory, 0);
}

void Vulkan__CreateImage(
    Vulkan_t* self,
    uint32_t width,
    uint32_t height,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkImage* image,
    VkDeviceMemory* imageMemory) {
  VkImageCreateInfo imageInfo;
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.flags = 0;
  imageInfo.pNext = NULL;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = width;
  imageInfo.extent.height = height;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = format;
  imageInfo.tiling = tiling;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = usage;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  ASSERT(VK_SUCCESS == vkCreateImage(self->m_logicalDevice, &imageInfo, NULL, image))

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(self->m_logicalDevice, *image, &memRequirements);

  VkMemoryAllocateInfo allocInfo;
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.pNext = NULL;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex =
      Vulkan__FindMemoryType(self, memRequirements.memoryTypeBits, properties);

  ASSERT(VK_SUCCESS == vkAllocateMemory(self->m_logicalDevice, &allocInfo, NULL, imageMemory))

  vkBindImageMemory(self->m_logicalDevice, *image, *imageMemory, 0);
}

void Vulkan__BeginSingleTimeCommands(Vulkan_t* self, VkCommandBuffer* commandBuffer) {
  VkCommandBufferAllocateInfo allocInfo;
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.pNext = NULL;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = self->m_commandPool;
  allocInfo.commandBufferCount = 1;

  vkAllocateCommandBuffers(self->m_logicalDevice, &allocInfo, commandBuffer);

  VkCommandBufferBeginInfo beginInfo;
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.pNext = NULL;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  ASSERT(VK_SUCCESS == vkBeginCommandBuffer(*commandBuffer, &beginInfo))
}

void Vulkan__EndSingleTimeCommands(Vulkan_t* self, VkCommandBuffer* commandBuffer) {
  vkEndCommandBuffer(*commandBuffer);

  VkSubmitInfo submitInfo;
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.pNext = NULL;
  submitInfo.waitSemaphoreCount = 0;
  submitInfo.pWaitSemaphores = VK_NULL_HANDLE;
  submitInfo.pWaitDstStageMask = 0;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = commandBuffer;
  submitInfo.signalSemaphoreCount = 0;
  submitInfo.pSignalSemaphores = VK_NULL_HANDLE;

  ASSERT(
      VK_SUCCESS ==
      vkQueueSubmit(self->m_SwapChain__queues.graphics__queue, 1, &submitInfo, VK_NULL_HANDLE))
  ASSERT(VK_SUCCESS == vkQueueWaitIdle(self->m_SwapChain__queues.graphics__queue))

  vkFreeCommandBuffers(self->m_logicalDevice, self->m_commandPool, 1, commandBuffer);
}

void Vulkan__TransitionImageLayout(
    Vulkan_t* self,
    VkImage* image,
    VkFormat format,
    VkImageLayout oldLayout,
    VkImageLayout newLayout) {
  VkCommandBuffer commandBuffer;
  Vulkan__BeginSingleTimeCommands(self, &commandBuffer);

  VkImageMemoryBarrier barrier[] = {{
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .oldLayout = oldLayout,
      .newLayout = newLayout,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = *image,
      .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .subresourceRange.baseMipLevel = 0,
      .subresourceRange.levelCount = 1,
      .subresourceRange.baseArrayLayer = 0,
      .subresourceRange.layerCount = 1,
  }};

  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;

  if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier[0].srcAccessMask = 0;
    barrier[0].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (
      oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
      newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else {
    ASSERT_CONTEXT(false, "unsupported layout transition!")
  }

  vkCmdPipelineBarrier(
      commandBuffer,
      sourceStage,
      destinationStage,
      0,
      0,
      NULL,
      0,
      NULL,
      1,
      barrier);

  Vulkan__EndSingleTimeCommands(self, &commandBuffer);
}

void Vulkan__CopyBufferToImage(
    Vulkan_t* self, VkBuffer* buffer, VkImage* image, u32 width, u32 height) {
  VkCommandBuffer commandBuffer;
  Vulkan__BeginSingleTimeCommands(self, &commandBuffer);

  VkBufferImageCopy region;
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;
  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;
  region.imageOffset = (VkOffset3D){0, 0, 0};
  region.imageExtent = (VkExtent3D){width, height, 1};

  vkCmdCopyBufferToImage(
      commandBuffer,
      *buffer,
      *image,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      1,
      &region);

  Vulkan__EndSingleTimeCommands(self, &commandBuffer);
}

/**
 * Load an image from disk. Queue it to Vulkan -> Buffer -> Image.
 */
void Vulkan__CreateTextureImage(Vulkan_t* self, const char* file) {
  int texWidth, texHeight, texChannels;
  stbi_uc* pixels = stbi_load(file, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
  VkDeviceSize imageSize = texWidth * texHeight * 4;

  ASSERT_CONTEXT(pixels, "failed to load texture image!")

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  Vulkan__CreateBuffer(
      self,
      imageSize,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      &stagingBuffer,
      &stagingBufferMemory);

  void* data;
  vkMapMemory(self->m_logicalDevice, stagingBufferMemory, 0, imageSize, 0, &data);
  memcpy(data, pixels, (size_t)(imageSize));
  vkUnmapMemory(self->m_logicalDevice, stagingBufferMemory);

  stbi_image_free(pixels);

  Vulkan__CreateImage(
      self,
      texWidth,
      texHeight,
      VK_FORMAT_R8G8B8A8_SRGB,
      VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      &self->m_textureImage,
      &self->m_textureImageMemory);

  Vulkan__TransitionImageLayout(
      self,
      &self->m_textureImage,
      VK_FORMAT_R8G8B8A8_SRGB,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  Vulkan__CopyBufferToImage(
      self,
      &stagingBuffer,
      &self->m_textureImage,
      (u32)(texWidth),
      (u32)(texHeight));
  Vulkan__TransitionImageLayout(
      self,
      &self->m_textureImage,
      VK_FORMAT_R8G8B8A8_SRGB,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  vkDestroyBuffer(self->m_logicalDevice, stagingBuffer, NULL);
  vkFreeMemory(self->m_logicalDevice, stagingBufferMemory, NULL);
}

void Vulkan__CreateImageView(
    Vulkan_t* self, VkImage* image, VkFormat format, VkImageView* imageView) {
  VkImageViewCreateInfo viewInfo;
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.pNext = NULL;
  viewInfo.flags = 0;
  viewInfo.image = *image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = format;
  viewInfo.components = (VkComponentMapping){0, 0, 0, 0};
  viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  ASSERT(VK_SUCCESS == vkCreateImageView(self->m_logicalDevice, &viewInfo, NULL, imageView))
}

void Vulkan__CreateTextureImageView(Vulkan_t* self) {
  Vulkan__CreateImageView(
      self,
      &self->m_textureImage,
      VK_FORMAT_R8G8B8A8_SRGB,
      &self->m_textureImageView);
}

void Vulkan__CreateTextureSampler(Vulkan_t* self) {
  VkPhysicalDeviceProperties properties;
  vkGetPhysicalDeviceProperties(self->m_physicalDevice, &properties);

  VkSamplerCreateInfo samplerInfo;
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.pNext = NULL;
  samplerInfo.flags = 0;
  // nearest is best for pixel art, but a pixel shader is even better
  samplerInfo.magFilter = VK_FILTER_NEAREST;                // VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_NEAREST;                // VK_FILTER_LINEAR;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;  // VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.mipLodBias = 0;
  samplerInfo.anisotropyEnable = VK_TRUE;
  samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  samplerInfo.minLod = 0;
  samplerInfo.maxLod = 0;
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = VK_FALSE;

  ASSERT(
      VK_SUCCESS ==
      vkCreateSampler(self->m_logicalDevice, &samplerInfo, NULL, &self->m_textureSampler))
}

void Vulkan__CopyBuffer(
    Vulkan_t* self, VkBuffer* srcBuffer, VkBuffer* dstBuffer, VkDeviceSize size) {
  VkCommandBuffer commandBuffer;
  Vulkan__BeginSingleTimeCommands(self, &commandBuffer);

  VkBufferCopy copyRegion;
  copyRegion.srcOffset = 0;
  copyRegion.dstOffset = 0;
  copyRegion.size = size;
  vkCmdCopyBuffer(commandBuffer, *srcBuffer, *dstBuffer, 1, &copyRegion);

  Vulkan__EndSingleTimeCommands(self, &commandBuffer);
}

void Vulkan__CreateVertexBuffer(Vulkan_t* self, u8 idx, u64 size, const void* indata) {
  VkDeviceSize bufferSize = size;

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  Vulkan__CreateBuffer(
      self,
      bufferSize,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      &stagingBuffer,
      &stagingBufferMemory);

  void* data;
  vkMapMemory(self->m_logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, indata, (size_t)bufferSize);
  vkUnmapMemory(self->m_logicalDevice, stagingBufferMemory);

  Vulkan__CreateBuffer(
      self,
      bufferSize,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      &self->m_vertexBuffers[idx],
      &self->m_vertexBufferMemories[idx]);

  Vulkan__CopyBuffer(self, &stagingBuffer, &self->m_vertexBuffers[idx], bufferSize);

  vkDestroyBuffer(self->m_logicalDevice, stagingBuffer, NULL);
  vkFreeMemory(self->m_logicalDevice, stagingBufferMemory, NULL);
}