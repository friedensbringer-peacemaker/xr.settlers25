// SPDX-License-Identifier: GPL-2.0-or-later
#include "RealTable.h"
#include <cassert>
#include <cmath>
#include <iostream>

namespace {
bool near(float a, float b, float eps = 1e-3f) { return std::fabs(a - b) <= eps; }
// Scene table plane: surface in local X/Y, normal (local Z) pointing up => rotate -90 degrees about X.
const vrTable::Quat faceUp{-0.70710678f, 0.f, 0.f, 0.70710678f};
} // namespace

int main()
{
    // Normal pointing up
    const auto n = vrTable::rotate(faceUp, {0.f, 0.f, 1.f});
    assert(near(n.x, 0.f) && near(n.y, 1.f) && near(n.z, 0.f));

    // 1.6 m x 0.8 m desk, centred 1 m in front of the viewer (-Z), surface at 0.75 m, viewer head at 1.2 m.
    const auto p = vrTable::fit({-0.8f, 0.75f, -1.f + 0.4f}, faceUp, 0.f, 0.f, 1.6f, 0.8f, {0.f, 1.2f, 0.f}, 1.6f, 3.f);
    assert(near(p.center.x, 0.f) && near(p.center.y, 0.76f) && near(p.center.z, -1.f));
    assert(near(p.area, 1.28f));
    // Far side of the board points away from the viewer (towards -Z)
    const auto d = vrTable::depthAxis(p.yaw);
    assert(d.z < -0.99f);
    // Width limited by depth: 0.8 m * aspect 1.6 = 1.28 m, 95 % margin
    assert(near(p.width, 0.95f * 1.28f));

    // Viewer on the other side of the table: board turns around
    const auto q = vrTable::fit({-0.8f, 0.75f, -0.6f}, faceUp, 0.f, 0.f, 1.6f, 0.8f, {0.f, 1.2f, -2.f}, 1.6f, 3.f);
    assert(vrTable::depthAxis(q.yaw).z > 0.99f);

    // maxWidth caps large tables
    const auto big = vrTable::fit({-2.f, 0.7f, -3.f}, faceUp, 0.f, 0.f, 4.f, 4.f, {0.f, 1.2f, 0.f}, 1.6f, 2.4f);
    assert(near(big.width, 2.4f));

    // Origin puts the panel centre exactly on the table centre for any panel distance
    for(float panelZ : {-0.8f, -1.2f, -2.5f})
    {
        const auto o = vrTable::originFor(p.center, p.yaw, panelZ);
        const vrPanel::Pose pose{o, p.yaw, panelZ, -1.5707963f, 0.f};
        const auto c = pose.toWorld({0.f, 0.f, panelZ});
        assert(near(c.x, p.center.x) && near(c.y, p.center.y) && near(c.z, p.center.z));
        // Top of the image lies on the far side, flat on the table
        const auto top = pose.toWorld({0.f, 0.3f, panelZ});
        assert(near(top.y, p.center.y) && top.z < p.center.z);
    }
    std::cout << "real table placement: orientation, fit, cap and pose passed\n";
}
