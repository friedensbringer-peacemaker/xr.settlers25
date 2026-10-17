// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "driver/VideoDriver.h"
#include <memory>

/// Vorwärtsdeklaration, damit dieser Header auch ohne OpenXR-Header kompiliert.
struct XrInstance_T;
struct XrSession_T;
struct XrDebugUtilsMessengerEXT_T;

/**
 * OpenXR-Video-Treiber für Meta Quest 3 (Tabletop-/Paneldarstellung).
 *
 * Strategie (MVP):
 *  - RTTR zeichnet wie gewohnt in einen Offscreen-FBO (Ersatz-Backbuffer),
 *    der in MessageLoop() gebunden wird.
 *  - SwapBuffers() blittet diesen Backbuffer auf die Bilder der Stereo-
 *    Swapchain und reicht sie per xrEndFrame an die Laufzeit.
 *  - Der rechte Controller-Strahl wird auf die Panel-Ebene projiziert und
 *    liefert absolute Mauskoordinaten; Trigger/Ablegen liefern Tasten.
 *
 * Der Treiber ist als Shared Library nach dem Muster von videoSDL2 gebaut
 * und wird vom DriverWrapper zur Laufzeit geladen.
 *
 * Paneldarstellung und rechter Controller auf Quest getestet.
 * Linker Stick: Panelbreite und Abstand; rechter Stick: Kartenverschiebung.
 */
class VideoOpenXR final : public VideoDriver
{
public:
    explicit VideoOpenXR(VideoDriverLoaderInterface* CallBack);
    ~VideoOpenXR() override;

    const char* GetName() const override;
    bool Initialize() override;
    bool CreateScreen(const std::string& title, VideoMode newSize, DisplayMode displayMode) override;
    bool ResizeScreen(VideoMode newSize, DisplayMode displayMode) override;
    void DestroyScreen() override;
    bool SwapBuffers() override;
    bool OpenBuildingHelp(int site) override;
    bool DrawVrHeadquarters(Position point,unsigned color) override;
    int VrTableStatus() const override;
    int VrRecordStatus() const override;
    bool VrPassthroughAvailable() const override;
    bool MessageLoop() override;
    unsigned long GetTickCount() const override;
    OpenGL_Loader_Proc GetLoaderFunction() const override;
    std::vector<VideoMode> ListVideoModes() const override;
    GuiScaleRange getGuiScaleRange() const override;

    void SetMousePos(Position pos) override;
    KeyEvent GetModKeyState() const override;
    void* GetMapPointer() const override;
    void ShowErrorMessage(const std::string& title, const std::string& message) override;

private:
    void StartRecording(int panelWidth, int panelHeight);
    void StopRecording();
    void EncodeRecordingFrame(unsigned nativeTexture);
    struct XrState;
    std::unique_ptr<XrState> xr_;

    bool CreateOffscreenBackbuffer(Extent size);
    void DestroyOffscreenBackbuffer();
    bool BeginFrame();
    void UpdatePassthrough(bool enabled,int percent);

    unsigned backbufferFbo_ = 0;      ///< Offscreen-Ziel für RTTR-Zeichnung
    unsigned backbufferColor_ = 0;    ///< Textur hinter backbufferFbo_
    bool frameActive_ = false;
    bool sessionRunning_ = false;     ///< Spiegelt xr_->sessionRunning (Stub-Modus)
};

// CreateVideoInstance/FreeVideoInstance/GetDriverName werden ueber driver/Interface.h
// bzw. driver/VideoInterface.h (RTTR_DECL, C-Linkage) deklariert.
