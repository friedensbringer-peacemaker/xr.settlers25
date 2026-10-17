// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "PanelGeometry.h"
#include <algorithm>
#include <cmath>

/// Places the flat game board on a real table found by the Quest scene model (VR-022).
/// A scene plane has its surface in local X/Y and its normal along local Z; bounds are an
/// offset/extent rectangle in that local plane.
namespace vrTable {
using vrPanel::Vec3;
struct Quat { float x, y, z, w; };

inline Vec3 rotate(Quat q, Vec3 v)
{
    // v' = v + 2w(q x v) + 2 q x (q x v)
    const Vec3 u{q.x, q.y, q.z};
    const Vec3 t{2.f * (u.y * v.z - u.z * v.y), 2.f * (u.z * v.x - u.x * v.z), 2.f * (u.x * v.y - u.y * v.x)};
    return {v.x + q.w * t.x + (u.y * t.z - u.z * t.y), v.y + q.w * t.y + (u.z * t.x - u.x * t.z),
            v.z + q.w * t.z + (u.x * t.y - u.y * t.x)};
}
inline float dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

/// Panel width axis and "away from the viewer" axis for a yaw, matching vrPanel::rotateYaw.
inline Vec3 widthAxis(float yaw) { return {std::cos(yaw), 0.f, -std::sin(yaw)}; }
inline Vec3 depthAxis(float yaw) { return {-std::sin(yaw), 0.f, -std::cos(yaw)}; }

struct Placement
{
    Vec3 center;   ///< surface centre, slightly lifted
    float yaw;     ///< board yaw: width along a table edge, top of the map pointing away from the viewer
    float width;   ///< board width that fits on the table
    float area;    ///< table area in m^2, used to pick the best table
};

/// aspect = image width / height. The board edge is aligned with a table edge; of the four
/// possibilities the one whose far side points away from the viewer is used.
inline Placement fit(Vec3 planePos, Quat q, float offX, float offY, float extW, float extH, Vec3 head, float aspect,
                     float maxWidth, float lift = 0.01f)
{
    const Vec3 a = rotate(q, {1.f, 0.f, 0.f}), b = rotate(q, {0.f, 1.f, 0.f});
    const float cx = offX + extW * 0.5f, cy = offY + extH * 0.5f;
    Placement p{};
    p.center = {planePos.x + a.x * cx + b.x * cy, planePos.y + a.y * cx + b.y * cy + lift,
                planePos.z + a.z * cx + b.z * cy};
    p.area = extW * extH;
    const float yaw0 = std::atan2(-a.z, a.x); // width axis parallel to the table's local X edge
    Vec3 toTable{p.center.x - head.x, 0.f, p.center.z - head.z};
    const float len = std::sqrt(toTable.x * toTable.x + toTable.z * toTable.z);
    if(len > 1e-4f) { toTable.x /= len; toTable.z /= len; }
    float best = -2.f;
    for(int k = 0; k < 4; ++k)
    {
        const float yaw = yaw0 + k * 1.5707963f;
        const float score = dot(depthAxis(yaw), toTable);
        if(score > best) { best = score; p.yaw = yaw; }
    }
    // Table extent along the board's width and depth axes (edges are aligned, so this is exact up to rounding).
    const Vec3 w = widthAxis(p.yaw), d = depthAxis(p.yaw);
    const float spanW = std::fabs(dot(w, a)) * extW + std::fabs(dot(w, b)) * extH;
    const float spanD = std::fabs(dot(d, a)) * extW + std::fabs(dot(d, b)) * extH;
    p.width = std::max(0.3f, std::min(maxWidth, 0.95f * std::min(spanW, spanD * aspect)));
    return p;
}

/// Pose origin so that a panel with the given yaw and centre distance (panelZ, negative) lies at `center`.
inline Vec3 originFor(Vec3 center, float yaw, float panelZ)
{
    const Vec3 offset = vrPanel::rotateYaw({0.f, 0.f, panelZ}, yaw);
    return {center.x - offset.x, center.y - offset.y, center.z - offset.z};
}
} // namespace vrTable
