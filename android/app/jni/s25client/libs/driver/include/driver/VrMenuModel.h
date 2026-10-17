// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
// VR menu duties (skill xr-vr-menu, section 0): quit with a visible two-click confirmation and hideable
// ray/marker (P3), stick/A/B focus navigation with hold-to-repeat (P2). Logic only, no drawing, no file format.
// TODO xrc/menu.h nach Tag: replace by xr-common (xrc::ConfirmAction, xrc::PointerVisibility) once tagged;
// this file follows its specification and is kept dependency free for the host test.
#include <cmath>
#include <cstdlib>
namespace vrMenuModel {

/// Two-click confirmation for a dangerous action ("Beenden"). The first click arms (button visibly red,
/// "Sicher? Nochmal klicken", double haptic pulse), a second click within `timeout` seconds confirms.
/// Without a second click the button falls back; update() reports that so the window can redraw.
class ConfirmAction {
public:
    enum Result { None, Armed, Confirmed };
    static constexpr float timeout = 3.f;
    Result click() {
        if(armed_) { armed_ = false; left_ = 0.f; return Confirmed; }
        armed_ = true; left_ = timeout; return Armed;
    }
    /// true = fell back from armed to idle in this call (redraw).
    bool update(float dt) {
        if(!armed_) return false;
        if(!(dt > 0.f) || !std::isfinite(dt)) dt = 0.f;
        left_ -= dt;
        if(left_ > 0.f) return false;
        armed_ = false; left_ = 0.f;
        return true;
    }
    /// Window closed, other action chosen: never carry an armed state into a new context.
    void cancel() { armed_ = false; left_ = 0.f; }
    bool armed() const { return armed_; }
    float remaining() const { return armed_ && left_ > 0.f ? left_ : 0.f; }
private:
    float left_ = 0.f;
    bool armed_ = false;
};

/// Stored switches for ray and target marker. While a menu is open both stay visible, otherwise
/// the menu could not be used.
struct PointerVisibility {
    bool ray = true, marker = true;
    bool showRay(bool menuOpen) const { return menuOpen || ray; }
    bool showMarker(bool menuOpen) const { return menuOpen || marker; }
};

/// Double haptic pulse on request: the game bumps a counter, the driver polls once per frame.
/// poll() returns 1 for the first pulse, 2 for the second one `gapMs` later, otherwise 0.
struct DoublePulse {
    unsigned seen = 0;
    unsigned long secondAt = 0;
    bool secondPending = false;
    int poll(unsigned request, unsigned long nowMs, unsigned long gapMs = 120) {
        if(request != seen) { seen = request; secondPending = true; secondAt = nowMs + gapMs; return 1; }
        if(secondPending && nowMs >= secondAt) { secondPending = false; return 2; }
        return 0;
    }
};
/// Pointer offset X/Y ("Zeiger waagerecht/senkrecht", duty P3): percent of the image width/height, 0.25 % grid,
/// ±15 %, positive = right/down. Applied to the laser UV before every mapping to game coordinates, never inside
/// a VR menu. Model: XRShell pointer_map.h (ptroff).
/// TODO xrc/menu.h: replace by xrc::ptroff once xr-common is tagged (same specification).
namespace ptroff {
constexpr float step = 0.25f;
constexpr float maxPercent = 15.f;
/// Round to the grid and clamp; NaN/Inf -> 0.
inline float clampPercent(float p)
{
    if(!std::isfinite(p)) return 0.f;
    p = std::round(p / step) * step;
    if(p == 0.f) p = 0.f; // no "-0"
    return p < -maxPercent ? -maxPercent : p > maxPercent ? maxPercent : p;
}
/// "‹" (dir = -1) or "›" (dir = +1).
inline float stepPercent(float p, int dir) { return clampPercent(p + step * static_cast<float>(dir)); }
/// Laser UV (0..1) -> shifted UV for the game, clamped at the image border.
inline float apply(float uv, float percent)
{
    const float v = uv + clampPercent(percent) / 100.f;
    return v < 0.f ? 0.f : v > 1.f ? 1.f : v;
}
/// Stored as quarter percent steps (exact in INI files): -60..60.
inline int clampQuarters(int q) { return q < -60 ? -60 : q > 60 ? 60 : q; }
inline float fromQuarters(int q) { return clampQuarters(q) * step; }
} // namespace ptroff

/// Menu navigation by stick (P2). Directions as sent to the game in KeyEvent::c.
enum Direction { DirNone = 0, DirUp, DirDown, DirLeft, DirRight };

/// Right stick to one focus step: the first deflection steps at once, holding repeats (450 ms, then every 200 ms).
/// x/y are raw thumbstick axes, y > 0 = up. The dominant axis wins; below 0.55 counts as neutral.
struct StickRepeat {
    int held = DirNone;
    unsigned long next = 0;
    int sample(float x, float y, unsigned long nowMs) {
        int dir = DirNone;
        if(std::fabs(x) >= .55f || std::fabs(y) >= .55f)
            dir = std::fabs(y) >= std::fabs(x) ? (y > 0 ? DirUp : DirDown) : (x > 0 ? DirRight : DirLeft);
        if(dir == DirNone) { held = DirNone; next = 0; return DirNone; }
        if(dir != held) { held = dir; next = nowMs + 450; return dir; }
        if(nowMs >= next) { next = nowMs + 200; return dir; }
        return DirNone;
    }
};

/// Screen rectangle of a focusable control (y grows downwards, as in the game GUI).
struct Box { int left, top, right, bottom; };

/// Next focus in `dir` from `current` (index into boxes, -1 = none yet). Without a current focus the top-left
/// control is chosen. Only controls whose centre lies in that direction count; the nearest one along the
/// direction wins, sideways offset weighs double. No candidate: focus stays (no wrap-around).
inline int nextFocus(const Box* boxes, int count, int current, int dir)
{
    if(count <= 0) return -1;
    const auto cx = [&](int i) { return (boxes[i].left + boxes[i].right) / 2; };
    const auto cy = [&](int i) { return (boxes[i].top + boxes[i].bottom) / 2; };
    if(current < 0 || current >= count)
    {
        int best = 0;
        for(int i = 1; i < count; ++i)
            if(cy(i) < cy(best) - 4 || (std::abs(cy(i) - cy(best)) <= 4 && cx(i) < cx(best))) best = i;
        return best;
    }
    int best = current;
    long bestScore = 0;
    for(int i = 0; i < count; ++i)
    {
        if(i == current) continue;
        const int dx = cx(i) - cx(current), dy = cy(i) - cy(current);
        int along = 0, side = 0;
        switch(dir)
        {
            case DirUp: along = -dy; side = dx; break;
            case DirDown: along = dy; side = dx; break;
            case DirLeft: along = -dx; side = dy; break;
            case DirRight: along = dx; side = dy; break;
            default: return current;
        }
        if(along <= 0) continue;
        const long score = along + 2L * std::abs(side);
        if(best == current || score < bestScore) { best = i; bestScore = score; }
    }
    return best;
}
} // namespace vrMenuModel
