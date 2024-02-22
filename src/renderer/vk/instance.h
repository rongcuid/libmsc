#pragma once

#include <volk.h>

bool instanceCreate(VkInstance *pInstance, VkDebugUtilsMessengerEXT *pMessenger,
                    bool validate);
