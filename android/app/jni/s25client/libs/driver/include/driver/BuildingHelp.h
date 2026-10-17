// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
namespace vrHelp {
inline const char* website(int site) {
    switch(site) {
    case 1: return "https://settlers2.net/guides/guide-to-buildings/";
    case 2: return "https://settlers2.net/guides/datasheet-for-buildings/";
    case 3: return "https://www.siedlercommunity.de/siedler2/gebaeude/";
    case 4: return "https://jh-isch.de/s2/tipps/t_gebaude.php";
    default: return "";
    }
}
}
