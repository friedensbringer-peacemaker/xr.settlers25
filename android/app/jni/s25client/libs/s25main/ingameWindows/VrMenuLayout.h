// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
namespace vrMenu {
constexpr int firstRow=115, rowStep=40, controlHeight=32;
constexpr int labelX=20, fieldX=300, fieldWidth=315;
constexpr int row(unsigned index) { return firstRow+static_cast<int>(index)*rowStep; }
}
