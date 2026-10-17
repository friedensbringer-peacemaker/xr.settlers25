// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "KeyEvent.h"
#include "VrSettings.h"
#include "MouseCoords.h"
#include <boost/config.hpp>

class BOOST_SYMBOL_VISIBLE VideoDriverLoaderInterface
{
public:
    virtual ~VideoDriverLoaderInterface() = default;

    virtual void Msg_LeftDown(MouseCoords mc) = 0;
    virtual void Msg_LeftUp(MouseCoords mc) = 0;
    virtual void Msg_RightDown(const MouseCoords& mc) = 0;
    virtual void Msg_RightUp(const MouseCoords& mc) = 0;
    virtual void Msg_MiddleDown(const MouseCoords& mc) = 0;
    virtual void Msg_MiddleUp(const MouseCoords& mc) = 0;
    virtual void Msg_WheelUp(const MouseCoords& mc) = 0;
    virtual void Msg_WheelDown(const MouseCoords& mc) = 0;
    virtual void Msg_MouseMove(const MouseCoords& mc) = 0;

    virtual void Msg_KeyDown(const KeyEvent& ke) = 0;

    virtual void WindowResized() = 0;
    virtual bool VrScroll(const MouseCoords&, bool, bool = true) { return false; }
    virtual void SaveVrPanel(int, int, int, int, int, int, int) {}
    virtual void VrPan(int, int) {}
    /// True if a click at this position would reach the open game map (no window, control or modal dialog).
    virtual bool VrMapAt(const MouseCoords&) const { return false; }
    /// Grab-drag the map so the map point under `from` moves to `to` (view coordinates).
    virtual void VrDrag(const MouseCoords&, const MouseCoords&) {}
    virtual void VrFocus(bool) {}
    virtual VrSettings GetVrSettings() const { return {}; }
};
