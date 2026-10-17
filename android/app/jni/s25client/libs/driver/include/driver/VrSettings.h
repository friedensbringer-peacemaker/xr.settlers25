// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <array>

/// Shared between the game and the OpenXR driver. Defaults match the selected Quest preset; 100 % brightness/contrast preserve the original image.
struct VrSettings
{
    bool smartScroll = true, clickStabilization = true, hoverHighlight = true;
    int smoothing = 1; // off / light / strong
    bool haptics = true, leftHanded = false, autoPause = true, rememberPanel = true;
    int panelWidthMm = 2000, panelXmm = 0, panelYmm = 0, panelZmm = 0, panelYawMilli = 0;
    unsigned recenterRequest = 0; // transient command, not stored
    // Actions: off, zoom, build, menu, speed, save, pause, map, HQ, build aid,
    // inventory, messages, chat, names, productivity, recenter, quick menu, view toggle.
    std::array<int, 4> buttons{{1, 4, 2, 3}}; // X Y A B
    std::array<int, 4> longButtons{{0, 0, 0, 0}};
    std::array<int, 6> favorites{{5, 9, 10, 3, 7, 8}};
    bool invertRightY = false, startCentered = true, rememberDistance = true;
    int panelControl = 2; // always / legacy guarded (disabled) / disabled
    int mapSpeed = 1, mapBoost = 1; // 1x/2x/4x; off / stick click / double tap / both
    int screenDistanceMm = 0, tableDistanceMm = 0; // zero: use selected preset
    bool automaticSaveName = true;
    int buildingHelpSite=0; // off / guide / datasheet / community / JH-ISCH
    bool headquarters3d=false; // Beta, external HQ asset on the tabletop only
    bool frameMove=true;
    bool hideLoadingPointer=true; // transient, safe until an interactive desktop exists
    bool passthrough=false;
    int passthroughPercent=100;
    bool frameMoveAvailable=false; // transient: only the game desktop with no open windows
    bool invertLeftX=false, invertLeftY=false, invertRightX=false, swapRightStickAxes=false;
    bool triggerGrab=false; // hold trigger on the map: hand movement pans, forward/back zooms; a tap still clicks
    int gripScrollSpeed=0; // right-grip map drag: 0..3 = 1x (map follows the pointer 1:1) .. 4x
    bool screenshotMode=false; // transient, not stored: no ray/marker/cursor while no window is open, no focus pause
    bool recordVideo=false; // transient, not stored: record the flat game image and game sound to RTTR/videos
    bool realTable=false; // table view on the real table from the Quest room scan (VR-022), falls back to the simulated table
    float restoredDistance() const
    {
        const int mm = presentation == 1 ? tableDistanceMm : screenDistanceMm;
        return rememberDistance && mm > 0 ? mm / 1000.f : viewStartDistance();
    }
    bool swapLeftStickAxes = true;
    int rayWidth = 1;
    int rayColor = 0;
    int markerShape = 1;
    int markerSize = 0;
    int cursorSize = 0;
    int brightness = 80;
    int contrast = 120;
    int sharpness = 0; // -2/-1: soften, 0: neutral, 1/2: sharpen
    int startDistance = 1; // 0: near (1.2 m), 1: medium (1.8 m), 2: far (2.8 m)
    float panelStartDistance() const { return startDistance == 0 ? 1.2f : startDistance == 2 ? 2.8f : 1.8f; }
    int presentation = 0; // 0: upright screen, 1: tilted tabletop (still the original 2D game)
    int tableTilt = 1; // 30 / 45 / 60 / 75 degrees away from upright
    int tableLoweringCm = 40; // center below the centered head position, 0..90 cm
    int tableDistance = 1; // 0.8 / 1.2 / 1.8 m
    float viewStartDistance() const
    { return presentation == 1 ? (tableDistance == 0 ? .8f : tableDistance == 2 ? 1.8f : 1.2f) : panelStartDistance(); }
    float viewPitch() const { return presentation == 1 ? -(30.f + 15.f * tableTilt) * 0.01745329252f : 0.f; }
    float viewHeight() const { return presentation == 1 ? -tableLoweringCm / 100.f : 0.f; }
    int viewCurvature() const { return presentation == 1 ? 0 : curvature; }
    int curvature = 1; // 0: flat, 1: gentle (45 degrees), 2: curved (75 degrees)
    bool rayVisible = true, markerVisible = true; // "Strahl: Aus" / "Zielpunkt: Aus"; an open menu always shows both
    bool menuOpen = false; // transient: a window or a non-game desktop is shown (WindowManager::GetVrSettings)
    // Pointer offset X/Y ("Zeiger waagerecht/senkrecht") in quarter percent of the image size, -60..60 (= ±15 %),
    // positive = right/down; applied to the laser hit before mapping it to the game, not inside VR menus.
    int ptrOffsetX = 0, ptrOffsetY = 0;
    unsigned hapticConfirmRequest = 0; // transient command, not stored: double pulse when a button arms
    bool vrMenuFocus = false; // transient: a VR menu window is on top, stick/A/B navigate it (P2)
};
