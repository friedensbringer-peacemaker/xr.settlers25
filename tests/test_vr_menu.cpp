// SPDX-License-Identifier: GPL-2.0-or-later
// Host test for driver/VrMenuModel.h (VR menu duties P3: quit confirmation, ray/marker "Aus", double pulse).
#include "driver/VrMenuModel.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
using namespace vrMenuModel;
int main()
{
    unsigned checks = 0;
    // Quit confirmation: sweep frame rates and the moment of the second click.
    for(float hz : {36.f, 60.f, 72.f, 90.f, 120.f, 10.f /* 100 ms menu timer */})
    {
        const float dt = 1.f / hz;
        for(float secondAfter : {0.05f, 0.5f, 1.5f, 2.9f, 3.1f, 4.f, 10.f})
        {
            ConfirmAction quit;
            assert(!quit.armed() && quit.remaining() == 0.f);
            assert(quit.click() == ConfirmAction::Armed);
            assert(quit.armed() && quit.remaining() > 2.99f);
            float t = 0.f;
            int fallbacks = 0;
            while(t + dt <= secondAfter)
            {
                if(quit.update(dt)) ++fallbacks;
                t += dt;
            }
            assert(fallbacks <= 1);
            const bool expectArmed = t < ConfirmAction::timeout - 1e-3f;
            if(std::fabs(t - ConfirmAction::timeout) > 2 * dt)
            {
                assert(quit.armed() == expectArmed);
                assert(fallbacks == (expectArmed ? 0 : 1));
                // Second click: confirms only while armed, otherwise it arms again (never quits by accident).
                assert(quit.click() == (expectArmed ? ConfirmAction::Confirmed : ConfirmAction::Armed));
                ++checks;
            }
        }
    }
    {
        // Fallback is reported exactly once, then stays idle.
        ConfirmAction quit;
        quit.click();
        int reports = 0;
        for(int i = 0; i < 100; ++i) reports += quit.update(0.1f) ? 1 : 0;
        assert(reports == 1 && !quit.armed() && quit.remaining() == 0.f);
        assert(!quit.update(0.1f));
        ++checks;
    }
    {
        // Invalid frame times never confirm or disarm on their own.
        ConfirmAction quit;
        quit.click();
        for(float bad : {0.f, -1.f, std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity()})
            assert(!quit.update(bad) && quit.armed());
        assert(std::fabs(quit.remaining() - ConfirmAction::timeout) < 1e-6f);
        ++checks;
    }
    {
        // Cancel (other action, window closed) disarms; the next click only arms.
        ConfirmAction quit;
        quit.click();
        quit.update(1.f);
        quit.cancel();
        assert(!quit.armed() && quit.remaining() == 0.f && !quit.update(1.f));
        assert(quit.click() == ConfirmAction::Armed);
        ++checks;
    }
    // Pointer visibility: full truth table; an open menu always shows ray and marker.
    for(int ray = 0; ray < 2; ++ray)
        for(int marker = 0; marker < 2; ++marker)
            for(int menu = 0; menu < 2; ++menu)
            {
                const PointerVisibility v{ray != 0, marker != 0};
                assert(v.showRay(menu != 0) == (menu != 0 || ray != 0));
                assert(v.showMarker(menu != 0) == (menu != 0 || marker != 0));
                ++checks;
            }
    assert(PointerVisibility{}.showRay(false) && PointerVisibility{}.showMarker(false));
    // Double pulse: one request gives exactly two pulses, the second after the gap; sweep frame steps.
    for(unsigned long step : {1ul, 8ul, 11ul, 14ul, 28ul, 100ul})
        for(unsigned long gap : {60ul, 120ul, 200ul})
        {
            DoublePulse pulse;
            unsigned long now = 5000;
            assert(pulse.poll(0, now, gap) == 0);
            assert(pulse.poll(1, now, gap) == 1);
            const unsigned long first = now;
            int seconds = 0;
            unsigned long secondTime = 0;
            while(now < first + gap + 3 * step)
            {
                now += step;
                const int r = pulse.poll(1, now, gap);
                assert(r != 1);
                if(r == 2) { ++seconds; secondTime = now; }
            }
            assert(seconds == 1 && secondTime - first >= gap && secondTime - first < gap + step);
            // A new request restarts the pair; counter wrap-around also counts as a request.
            assert(pulse.poll(2, now, gap) == 1);
            ++checks;
        }
    {
        DoublePulse pulse;
        pulse.seen = 0xFFFFFFFFu;
        assert(pulse.poll(0u, 0, 120) == 1);
        ++checks;
    }
    // Stick repeat: first step at once, hold repeats after 450 ms then every 200 ms; neutral resets.
    for(unsigned long step : {8ul, 11ul, 14ul, 28ul})
        for(float x : {-1.f, -.6f, 0.f, .6f, 1.f})
            for(float y : {-1.f, -.6f, 0.f, .6f, 1.f})
            {
                StickRepeat stick;
                const bool neutral = std::fabs(x) < .55f && std::fabs(y) < .55f;
                const int expected = neutral ? DirNone
                    : std::fabs(y) >= std::fabs(x) ? (y > 0 ? DirUp : DirDown) : (x > 0 ? DirRight : DirLeft);
                unsigned long now = 1000;
                assert(stick.sample(x, y, now) == expected);
                int repeats = 0;
                for(unsigned long t = step; t <= 1050; t += step)
                {
                    const int r = stick.sample(x, y, now + t);
                    assert(r == DirNone || r == expected);
                    if(r) ++repeats;
                }
                // Within ~1.05 s of holding: repeats at >=450, >=650, >=850, >=1050 ms (frame quantised).
                assert(neutral ? repeats == 0 : (repeats >= 3 && repeats <= 4));
                assert(stick.sample(0.f, 0.f, now + 1100) == DirNone);
                assert(stick.sample(x, y, now + 1110) == expected); // after neutral: immediate again
                ++checks;
            }
    // Focus: 3 x 4 grid like the quick menu; moves stay in the grid, edges do not wrap.
    for(int cols : {1, 2, 3})
        for(int rows : {1, 2, 4})
        {
            Box boxes[12];
            const int n = cols * rows;
            for(int i = 0; i < n; ++i)
                boxes[i] = {20 + (i % cols) * 330, 115 + (i / cols) * 45, 340 + (i % cols) * 330, 153 + (i / cols) * 45};
            assert(nextFocus(boxes, n, -1, DirNone) == 0); // top-left first
            for(int i = 0; i < n; ++i)
            {
                const int c = i % cols, r = i / cols;
                assert(nextFocus(boxes, n, i, DirRight) == (c + 1 < cols ? i + 1 : i));
                assert(nextFocus(boxes, n, i, DirLeft) == (c > 0 ? i - 1 : i));
                assert(nextFocus(boxes, n, i, DirDown) == (r + 1 < rows ? i + cols : i));
                assert(nextFocus(boxes, n, i, DirUp) == (r > 0 ? i - cols : i));
                assert(nextFocus(boxes, n, i, DirNone) == i);
                ++checks;
            }
        }
    {
        // Wide button below two narrow ones: down from either reaches it, up returns to the nearer column.
        const Box boxes[] = {{20, 35, 230, 67}, {460, 35, 670, 67}, {20, 115, 670, 153}};
        assert(nextFocus(boxes, 3, 0, DirDown) == 2 && nextFocus(boxes, 3, 1, DirDown) == 2);
        assert(nextFocus(boxes, 3, 2, DirUp) == 0 || nextFocus(boxes, 3, 2, DirUp) == 1);
        assert(nextFocus(boxes, 0, -1, DirNone) == -1);
        ++checks;
    }
    // Pointer offset X/Y (P3): 0.25 % grid, ±15 %, NaN -> 0, no "-0", clamp at the image border.
    {
        using namespace ptroff;
        assert(clampPercent(std::numeric_limits<float>::quiet_NaN()) == 0.f);
        assert(clampPercent(std::numeric_limits<float>::infinity()) == 0.f);
        assert(clampPercent(0.1f) == 0.f && !std::signbit(clampPercent(-0.1f)));
        assert(clampPercent(0.13f) == 0.25f && clampPercent(-0.6f) == -0.5f);
        assert(clampPercent(99.f) == 15.f && clampPercent(-99.f) == -15.f);
        float p = 0.f;
        for(int i = 0; i < 100; ++i) p = stepPercent(p, +1);
        assert(p == 15.f);
        for(int i = 0; i < 200; ++i) p = stepPercent(p, -1);
        assert(p == -15.f);
        for(float uv = 0.f; uv <= 1.f; uv += 0.05f)
            for(float off : {-15.f, -1.25f, 0.f, 0.5f, 15.f})
            {
                const float r = apply(uv, off);
                assert(r >= 0.f && r <= 1.f);
                if(uv + off / 100.f >= 0.f && uv + off / 100.f <= 1.f) assert(std::fabs(r - (uv + off / 100.f)) < 1e-6f);
                ++checks;
            }
        assert(apply(0.5f, std::numeric_limits<float>::quiet_NaN()) == 0.5f);
        assert(clampQuarters(-61) == -60 && clampQuarters(61) == 60 && fromQuarters(5) == 1.25f && fromQuarters(-2) == -0.5f);
        ++checks;
    }
    std::cout << "test_vr_menu: " << checks << " checks passed\n";
    return 0;
}
