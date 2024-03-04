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
  // LocateQueueFamilies();
  /*
    {
      ABORT_IF(!self->m_surface)

      u32 queueFamilyCount = 0;
      vkGetPhysicalDeviceQueueFamilyProperties(self->m_physicalDevice, &queueFamilyCount, NULL);

      VkQueueFamilyProperties queueFamilies[queueFamilyCount];
      vkGetPhysicalDeviceQueueFamilyProperties(
          self->m_physicalDevice,
          &queueFamilyCount,
          queueFamilies);

      // list all queue families found on the current physical device
      LOG_DEBUGF("device queue families:");
      bool same = false;
      for (uint32_t i = 0; i < queueFamilies.size(); i++) {
        const auto& queueFamily = queueFamilies[i];

        const bool graphics = queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT;
        const bool compute = queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT;
        const bool transfer = queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT;
        const bool sparse = queueFamily.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT;
        const bool protect = queueFamily.queueFlags & VK_QUEUE_PROTECTED_BIT;
        // if VK_KHR_video_decode_queue extension:
        // const bool video_decode = queueFamily.queueFlags & VK_QUEUE_VIDEO_DECODE_BIT;
        // ifdef VK_ENABLE_BETA_EXTENSIONS:
        // const bool video_encode = queueFamily.queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT;
        const bool optical = queueFamily.queueFlags & VK_QUEUE_OPTICAL_FLOW_BIT_NV;

        VkBool32 present = false;
        // Query if presentation is supported
        //
    https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkGetPhysicalDeviceSurfaceSupportKHR.html
        if (vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &present) !=
            VK_SUCCESS) {
          Logger::Debugf("vkGetPhysicalDeviceSurfaceSupportKHR() failed. queueFamilyIndex: %u", i);
        }

        // strategy: select fewest family indices where required queues are present
        if (!same) {
          if (graphics && present) {
            // prioritize any queue with both
            same = true;
            pdqs.graphics.index = i;
            pdqs.present.index = i;
          } else if (graphics) {
            pdqs.graphics.index = i;
          } else if (present) {
            pdqs.present.index = i;
          }
        }

        Logger::Debugf(
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

      Logger::Debugf(
          "  selected: graphics: %u, present: %u",
          pdqs.graphics.index,
          pdqs.present.index);
    }

    //   // for each unique queue family index,
    //   // construct a request for pointer to its VkQueue
    //   if (!pdqs.graphics.index.has_value()) {
    //     throw Logger::Errorf("GRAPHICS queue not found within queue families of physical
    device.");
    //   }
    //   if (!pdqs.present.index.has_value()) {
    //     throw Logger::Errorf("PRESENT queue not found within queue families of physical
    device.");
    //   }
    //   const bool same = pdqs.graphics.index.value() == pdqs.present.index.value();
    //   std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    //   {
    //     // Structure specifying parameters of a newly created device queue
    //     //
    //
    https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDeviceQueueCreateInfo.html
    //     VkDeviceQueueCreateInfo createInfo{};

    //     // the type of this structure.
    //     createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

    //     // NULL or a pointer to a structure extending this structure.
    //     // createInfo.pNext = NULL;

    //     // a bitmask indicating behavior of the queues.
    //     // createInfo.flags = 0;

    //     // an unsigned integer indicating the index of the queue family in which to create the
    //     queues on
    //     // this device. This index corresponds to the index of an element of the
    //     pQueueFamilyProperties
    //     // array, and that was returned by vkGetPhysicalDeviceQueueFamilyProperties.
    //     createInfo.queueFamilyIndex = pdqs.graphics.index.value();

    //     // an unsigned integer specifying the number of queues to create in the queue family
    //     // indicated by queueFamilyIndex, and with the behavior specified by flags.
    //     createInfo.queueCount = 1;

    //     // a pointer to an array of queueCount normalized floating point values, specifying
    //     priorities
    //     // of work that will be submitted to each created queue.
    //     const float queuePriority = 1.0f;
    //     createInfo.pQueuePriorities = &queuePriority;

    //     // one request per unique queue family index
    //     queueCreateInfos.push_back(createInfo);
    //   }
    //   if (!same) {
    //     VkDeviceQueueCreateInfo createInfo{};
    //     createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    //     createInfo.queueFamilyIndex = pdqs.present.index.value();
    //     createInfo.queueCount = 1;
    //     const float queuePriority = 1.0f;
    //     createInfo.pQueuePriorities = &queuePriority;
    //     queueCreateInfos.push_back(createInfo);
    //   }

    //   // Structure describing the fine-grained features that can be supported by an
    implementation
    //   //
    //
    https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPhysicalDeviceFeatures.html
    //   VkPhysicalDeviceFeatures deviceFeatures{};

    //   // used with texture images
    //   deviceFeatures.samplerAnisotropy = VK_TRUE;

    //   // Structure specifying parameters of a newly created [logical] device
    //   //
    https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDeviceCreateInfo.html
    //   VkDeviceCreateInfo createInfo{};
    //   // the type of this structure.
    //   createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    //   // NULL or a pointer to a structure extending this structure.
    //   createInfo.pNext = NULL;

    //   // reserved for future use.
    //   createInfo.flags = static_cast<uint32_t>(0);

    //   // unsigned integer size of the pQueueCreateInfos array.
    //   createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

    //   // pointer to an array of VkDeviceQueueCreateInfo structures describing the queues that are
    //   // requested to be created along with the logical device.
    //   createInfo.pQueueCreateInfos = queueCreateInfos.data();

    // // define validation layers to be used, per-device.
    // // deprecated, because we also defined it on the VkInstance,
    // // but recommended to also keep here for backward-compatibility.
    // #ifdef DEBUG_VULKAN
    //   createInfo.enabledLayerCount = static_cast<uint32_t>(requiredValidationLayers.size());
    //   createInfo.ppEnabledLayerNames = requiredValidationLayers.data();
    // #else
    //   createInfo.enabledLayerCount = 0;
    // #endif

    //   // number of device extensions to enable.
    //   createInfo.enabledExtensionCount =
    //   static_cast<uint32_t>(requiredPhysicalDeviceExtensions.size());

    //   // pointer to an array of enabledExtensionCount null-terminated UTF-8 strings containing
    the
    //   names
    //   // of extensions to enable for the created device.
    //   createInfo.ppEnabledExtensionNames = requiredPhysicalDeviceExtensions.data();

    //   // NULL or a pointer to a VkPhysicalDeviceFeatures structure containing boolean indicators
    of
    //   all
    //   // the features to be enabled.
    //   createInfo.pEnabledFeatures = &deviceFeatures;

    //   // Create a new [logical] device instance
    //   // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkCreateDevice.html
    //   if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS) {
    //     throw Logger::Errorf("vkCreateDevice() failed.");
    //   }

    //   // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkGetDeviceQueue.html
    //   vkGetDeviceQueue(logicalDevice, pdqs.graphics.index.value(), 0, &pdqs.graphics.queue);
    //   if (!same) {
    //     vkGetDeviceQueue(logicalDevice, pdqs.present.index.value(), 0, &pdqs.present.queue);
    //   }
    */
}

void Vulkan__CreateSwapChain(Vulkan_t* self) {
}