#pragma once

#include <cstdlib> // IWYU pragma: keep
#include <volk.h>
#include <vulkan/vk_enum_string_helper.h>

#include "core/logger.hpp"

#define VK_CHECK(vk_function)                           \
	do {                                                \
		VkResult err = (vk_function);                   \
		if (err != VK_SUCCESS) {                        \
			RTGJ_LOG_CRITICAL(                          \
			    "vulkan call '{}' failed with {} ({})", \
			    #vk_function,                           \
			    string_VkResult(err),                   \
			    static_cast<int>(err));                 \
			abort();                                    \
		}                                               \
	} while (0)
