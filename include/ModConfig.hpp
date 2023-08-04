#pragma once

#include "config-utils/shared/config-utils.hpp"

// Declare the mod config as "ModConfiguration" and declare all its values and functions.
DECLARE_CONFIG(ModConfig,
    CONFIG_VALUE(VariableTitleText, std::string, "Title Text", "Beat Saber");
    CONFIG_VALUE(VariableTitleSize, float, "Title Size", 80);
    CONFIG_VALUE(VariableBlockAds, bool, "Block Ads", true);
)
