#include "Vulkan.h"

#include <cglm/cglm.h>
#include <string.h>

#define VOLK_IMPLEMENTATION
#include <volk.h>

#include "Base.h"

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

  u32 availableFormatsCount = 0;
  ASSERT(
      VK_SUCCESS == vkGetPhysicalDeviceSurfaceFormatsKHR(
                        self->m_physicalDevice,
                        self->m_surface,
                        &availableFormatsCount,
                        NULL))
  ASSERT(availableFormatsCount > 0)
  ASSERT(availableFormatsCount <= VULKAN_SWAPCHAIN_FORMATS_CAP)
  LOG_INFOF("physical device surface formats count: %u", availableFormatsCount);
  ASSERT(
      VK_SUCCESS == vkGetPhysicalDeviceSurfaceFormatsKHR(
                        self->m_physicalDevice,
                        self->m_surface,
                        &availableFormatsCount,
                        self->m_SwapChain__formats))

  u32 availablePresentModesCount = 0;
  ASSERT(
      VK_SUCCESS == vkGetPhysicalDeviceSurfacePresentModesKHR(
                        self->m_physicalDevice,
                        self->m_surface,
                        &availablePresentModesCount,
                        NULL))
  ASSERT(availablePresentModesCount > 0)
  ASSERT(availablePresentModesCount <= VULKAN_SWAPCHAIN_PRESENT_MODES_CAP)
  LOG_INFOF("physical device surface present modes count: %u", availablePresentModesCount);
  ASSERT(
      VK_SUCCESS == vkGetPhysicalDeviceSurfacePresentModesKHR(
                        self->m_physicalDevice,
                        self->m_surface,
                        &availablePresentModesCount,
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

void Vulkan__CreateSwapChain(Vulkan_t* self) {
}