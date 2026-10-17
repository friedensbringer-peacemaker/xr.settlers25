// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "iwHelp.h"
#include "Loader.h"
#include "Settings.h"
#include "WindowManager.h"
#include "drivers/VideoDriverWrapper.h"
#include "iwMsgbox.h"
#include <algorithm>
#include "controls/ctrlMultiline.h"
#include "gameData/const_gui_ids.h"

/// Breite des Fensters
const unsigned short HELP_WINDOW_WIDTH = 280;

/// Maximale Anzahl von Zeilen, bis Scrollbar eingesetzt wird
const unsigned MAX_LINES = 15;

iwHelp::iwHelp(const std::string& content, bool buildingInfo)
    : IngameWindow(CGI_HELP, IngameWindow::posAtMouse, Extent(HELP_WINDOW_WIDTH, 480), _("What is this?"),
                   LOADER.GetImageN("io", 1))
{
    // Größe des Fensters und des Controls nach der Anzahl der Zeilen
    ctrlMultiline* text = AddMultiline(2, DrawPoint(contentOffset), GetIwSize(), TextureColor::Green1, NormalFont);
    text->SetNumVisibleLines(MAX_LINES);
    text->ShowBackground(false);
    text->AddString(content, COLOR_YELLOW, false);
    // Shrink to content
    text->Resize(text->GetContentSize());
    SetIwSize(text->GetSize());
    website_=buildingInfo ? SETTINGS.video.vr.buildingHelpSite : 0;
    if(website_>0 && website_<=4) {
        const auto size=GetIwSize();
        SetIwSize(Extent(std::max(280u,size.x),size.y+44));
        AddTextButton(3,DrawPoint(contentOffset)+DrawPoint(0,size.y+6),
                      Extent(GetIwSize().x,32),TextureColor::Green2,_("Online-Gebäudehilfe öffnen"),NormalFont);
    }
    MoveNextToMouse();
}

void iwHelp::Msg_ButtonClick(unsigned id)
{
    if(id==3 && website_>0 && !VIDEODRIVER.GetDriver()->OpenBuildingHelp(website_))
        WINDOWMANAGER.Show(std::make_unique<iwMsgbox>(_("Gebäudehilfe"),
            _("Die Website konnte nicht im Browser geöffnet werden."),nullptr,MsgboxButton::Ok,MsgboxIcon::ExclamationGreen));
}
