#include "Vulkan.h"

#include <cglm/cglm.h>
#include <string.h>

#define VOLK_IMPLEMENTATION
#include <volk.h>

#include "Base.h"

const char* ckp_Vulkan__ERROR_MESSAGES[] = {
    "None\n",
    "volkInitialize() failed.\n",
    "vkCreateInstance() failed.\n",
    "vkEnumerateInstanceLayerProperties() failed to count. RequiredLayerCount: %u\n",
    "vkEnumerateInstanceLayerProperties() count was zero. RequiredLayerCount: %u\n",
    "vkEnumerateInstanceLayerProperties() failed to read. RequiredLayerCount: %u, "
    "SupportedLayerCount: %u\n",
    "vkEnumerateInstanceLayerProperties() missing required layers.\n",
    "vkEnumerateInstanceExtensionProperties() failed to count. RequiredLayerCount: %u\n",
    "vkEnumerateInstanceExtensionProperties() count was zero. RequiredLayerCount: %u\n",
    "vkEnumerateInstanceExtensionProperties() failed to read. RequiredLayerCount: %u, "
    "SupportedLayerCount: %u\n",
    "vkEnumerateInstanceExtensionProperties() missing required extensions.\n",
};

Vulkan__Error_t Vulkan__InitDriver(Vulkan_t* self) {
  self->m_requiredDriverExtensionCount = 0;
  self->m_requiredValidationLayerCount = 0;
  self->m_instance = NULL;

  VkResult r = volkInitialize();
  if (VK_SUCCESS != r) {
    return VULKAN_ERROR_VOLK_INITIALIZE_FAILED;
  }
  return VULKAN_ERROR_NONE;
}

static const char* debug_validation_layer1 = "VK_LAYER_KHRONOS_validation";

Vulkan__Error_t Vulkan__AssertDriverValidationLayersSupported(Vulkan_t* self) {
#ifdef DEBUG_VULKAN
  // SDK-provided layer conveniently bundles all useful standard validation
  self->m_requiredValidationLayers[self->m_requiredValidationLayerCount++] =
      debug_validation_layer1;
#endif

  if (self->m_requiredValidationLayerCount > 0) {
    u32 availableLayersCount = 0;
    ASSERT_ERROR(
        VK_SUCCESS == vkEnumerateInstanceLayerProperties(&availableLayersCount, NULL),
        VULKAN_ERROR_VK_EILP_COUNT_FAILED,
        ckp_Vulkan__ERROR_MESSAGES,
        self->m_requiredValidationLayerCount);

    ASSERT_ERROR(
        availableLayersCount > 0,
        VULKAN_ERROR_VK_EILP_COUNT_ZERO,
        ckp_Vulkan__ERROR_MESSAGES,
        self->m_requiredValidationLayerCount);

    VkLayerProperties availableLayers[availableLayersCount];
    ASSERT_ERROR(
        VK_SUCCESS == vkEnumerateInstanceLayerProperties(&availableLayersCount, availableLayers),
        VULKAN_ERROR_VK_EILP_READ_FAILED,
        ckp_Vulkan__ERROR_MESSAGES,
        self->m_requiredValidationLayerCount,
        availableLayersCount);

    // print list of validation layers to console
    bool found;
    LOG_INFOF("validation layers:\n");
    for (u8 i = 0; i < availableLayersCount; i++) {
      found = false;
      for (u8 i2 = 0; i2 < self->m_requiredValidationLayerCount; i2++) {
        if (0 == strcmp(self->m_requiredValidationLayers[i2], availableLayers[i].layerName)) {
          found = true;
          break;
        }
      }
      LOG_INFOF("  %s%s\n", availableLayers[i].layerName, found ? " (required)" : "");
    }
    // validate the required validation layers are all found
    for (u8 i3 = 0; i3 < self->m_requiredValidationLayerCount; i3++) {
      found = false;
      for (u8 i4 = 0; i4 < availableLayersCount; i4++) {
        if (0 == strcmp(self->m_requiredValidationLayers[i3], availableLayers[i4].layerName)) {
          found = true;
          break;
        }
      }
      if (!found) {
        LOG_INFOF("  missing %s", self->m_requiredValidationLayers[i3]);
        ASSERT_ERROR(
            found,
            VULKAN_ERROR_VK_EILP_MISSING_REQUIRED,
            ckp_Vulkan__ERROR_MESSAGES,
            NULL);
      }
    }
  }

  return VULKAN_ERROR_NONE;
}

Vulkan__Error_t Vulkan__AssertDriverExtensionsSupported(Vulkan_t* self) {
  // list the extensions supported by this driver
  u32 availableExtensionCount = 0;
  ASSERT_ERROR(
      VK_SUCCESS == vkEnumerateInstanceExtensionProperties(NULL, &availableExtensionCount, NULL),
      VULKAN_ERROR_VK_EIEP_COUNT_FAILED,
      ckp_Vulkan__ERROR_MESSAGES,
      NULL);

  ASSERT_ERROR(
      availableExtensionCount > 0,
      VULKAN_ERROR_VK_EIEP_COUNT_ZERO,
      ckp_Vulkan__ERROR_MESSAGES,
      self->m_requiredDriverExtensionCount);

  VkExtensionProperties availableExtensions[availableExtensionCount];
  ASSERT_ERROR(
      VK_SUCCESS == vkEnumerateInstanceExtensionProperties(
                        NULL,
                        &availableExtensionCount,
                        availableExtensions),
      VULKAN_ERROR_VK_EIEP_READ_FAILED,
      ckp_Vulkan__ERROR_MESSAGES,
      self->m_requiredDriverExtensionCount,
      availableExtensionCount);

  // print list of extensions to console
  bool found;
  LOG_INFOF("driver extensions:\n");
  for (u8 i = 0; i < availableExtensionCount; i++) {
    found = false;
    for (u8 i2 = 0; i2 < self->m_requiredDriverExtensionCount; i2++) {
      if (0 == strcmp(self->m_requiredDriverExtensions[i2], availableExtensions[i].extensionName)) {
        found = true;
        break;
      }
    }
    LOG_INFOF("  %s%s\n", availableExtensions[i].extensionName, found ? " (required)" : "");
  }

  // validate the required extensions are all found
  for (u8 i3 = 0; i3 < self->m_requiredDriverExtensionCount; i3++) {
    found = false;
    for (u8 i4 = 0; i4 < availableExtensionCount; i4++) {
      if (0 ==
          strcmp(self->m_requiredDriverExtensions[i3], availableExtensions[i4].extensionName)) {
        found = true;
        break;
      }
    }
    if (!found) {
      LOG_INFOF("  missing %s", self->m_requiredDriverExtensions[i3]);
      ASSERT_ERROR(found, VULKAN_ERROR_VK_EIEP_MISSING_REQUIRED, ckp_Vulkan__ERROR_MESSAGES, NULL);
    }
  }

  return VULKAN_ERROR_NONE;
}

Vulkan__Error_t Vulkan__CreateInstance(
    Vulkan_t* self,
    const char* name,
    const char* engineName,
    const unsigned int major,
    const unsigned int minor,
    const unsigned int hotfix) {
  VkApplicationInfo appInfo;
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pNext = NULL;
  appInfo.pApplicationName = name;
  appInfo.applicationVersion = VK_MAKE_VERSION(major, minor, hotfix);
  appInfo.pEngineName = engineName;
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_3;

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
  createInfo.pApplicationInfo = &appInfo;

#ifdef DEBUG_VULKAN
  // the number of global layers to enable.
  createInfo.enabledLayerCount = self->m_requiredValidationLayerCount;
  createInfo.ppEnabledLayerNames = self->m_requiredValidationLayers;
#else
  createInfo.enabledLayerCount = 0;
  createInfo.ppEnabledLayerNames = NULL;
#endif
  createInfo.enabledExtensionCount = self->m_requiredDriverExtensionCount;
  createInfo.ppEnabledExtensionNames = self->m_requiredDriverExtensions;

  if (vkCreateInstance(&createInfo, NULL, self->m_instance) != VK_SUCCESS) {
    return VULKAN_ERROR_VK_CREATE_INSTANCE_FAILED;
  }

  return VULKAN_ERROR_NONE;
}