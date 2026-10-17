// Copyright (C) 2005 - 2025 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <array>

// XR port: extra 4x step for close-up viewing in the headset (pixelated, but readable from a distance).
constexpr std::array<float, 8> SUPPRESS_UNUSED ZOOM_FACTORS = {{0.5f, 0.75f, 1.f, 1.25f, 1.5f, 2.f, 3.f, 4.f}};
constexpr size_t ZOOM_DEFAULT_INDEX = 2;
constexpr float ZOOM_ACCELERATION = 0.001f;
constexpr float ZOOM_WHEEL_INCREMENT = 0.03f;
