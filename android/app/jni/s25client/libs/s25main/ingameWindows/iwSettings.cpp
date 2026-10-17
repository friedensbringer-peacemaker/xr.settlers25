// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "iwSettings.h"
#include "VrMenuLayout.h"
#include "controls/ctrlText.h"
#include "controls/ctrlTextButton.h"
#include "iwVrTools.h"
#include "Loader.h"
#include "Settings.h"
#include "WindowManager.h"
#include "controls/ctrlCheck.h"
#include "controls/ctrlComboBox.h"
#include "controls/ctrlProgress.h"
#include "driver/VrMenuModel.h"
#include "controls/ctrlOptionGroup.h"
#include "driver/VideoInterface.h"
#include "drivers/VideoDriverWrapper.h"
#include "enum_cast.hpp"
#include "helpers/containerUtils.h"
#include "helpers/format.hpp"
#include "iwMsgbox.h"
#include "gameData/const_gui_ids.h"
#include "s25util/colors.h"
#include <cstdlib>

namespace {
enum
{
    ID_txtDisplayMode,
    ID_cbDisplayMode,
    ID_cbLockWindowSize,
    ID_grpResolution,
    ID_txtResolution,
    ID_cbResolution,
    ID_grpWindowSize,
    ID_txtWindowSize,
    ID_cbWindowSize,
    ID_txtMapScrollMode,
    ID_cbMapScrollMode,
    ID_cbSmartCursor,
    ID_cbStatisticScale,
    ID_btApply,
    ID_btAbort,
    ID_btVr,
};
} // namespace

iwSettings::iwSettings()
    : IngameWindow(CGI_SETTINGS, IngameWindow::posLastOrCenter, Extent(370, VIDEODRIVER.GetName().find("OpenXR") != std::string::npos ? 294 : 254), _("Settings"),
                   LOADER.GetImageN("resource", 41), false)
{
    // Controls are in 2 columns, the left might be the label for the control on the right
    constexpr auto rowWidth = 350;
    constexpr auto leftColOffset = 17u;                           // X-position of the left column
    constexpr DrawPoint rightColOffset(170u - leftColOffset, -1); // Offset from left column to right column
    constexpr Extent ctrlSize(rowWidth - rightColOffset.x - leftColOffset, 22);
    constexpr Extent cbSize = Extent(rowWidth - leftColOffset, 26);
    // 2 buttons evenly spaced
    constexpr Extent btSize = Extent(rowWidth / 3, 22);
    constexpr auto btSpacing = btSize.x / 3;

    DrawPoint curPos(leftColOffset, 30);
    AddText(ID_txtDisplayMode, curPos, _("Mode:"), COLOR_YELLOW, FontStyle{}, NormalFont);
    ctrlComboBox* cbDisplayMode =
      AddComboBox(ID_cbDisplayMode, curPos + rightColOffset, ctrlSize, TextureColor::Grey, NormalFont, 100);
    cbDisplayMode->AddItem(_("Windowed"));
    cbDisplayMode->AddItem(_("Fullscreen"));
    cbDisplayMode->AddItem(_("Borderless window"));
    cbDisplayMode->SetSelection(rttr::enum_cast(SETTINGS.video.displayMode.type));
    curPos.y += ctrlSize.y + 5;

    auto* grpResolution = AddGroup(ID_grpResolution);
    grpResolution->AddText(ID_txtResolution, curPos, _("Fullscreen resolution:"), COLOR_YELLOW, FontStyle{},
                           NormalFont);
    auto* cbResolution = grpResolution->AddComboBox(ID_cbResolution, curPos + rightColOffset, ctrlSize,
                                                    TextureColor::Grey, NormalFont, 110);
    auto* grpWindowSize = AddGroup(ID_grpWindowSize);
    grpWindowSize->AddText(ID_txtWindowSize, curPos, _("Window size:"), COLOR_YELLOW, FontStyle{}, NormalFont);
    auto& cbWindowSize = *grpWindowSize->AddComboBox(ID_cbWindowSize, curPos + rightColOffset, ctrlSize,
                                                     TextureColor::Grey, NormalFont, 150);
    curPos.y += ctrlSize.y + 1;
    grpWindowSize
      ->AddCheckBox(ID_cbLockWindowSize, curPos, cbSize, TextureColor::Grey, _("Lock window size:"), NormalFont, false)
      ->setChecked(!SETTINGS.video.displayMode.resizeable);

    const auto supportedResolutions = VIDEODRIVER.ListVideoModes();
    supportedResolutions_.assign(supportedResolutions.begin(), supportedResolutions.end());
    for(const auto& videoMode : supportedResolutions_)
    {
        cbResolution->AddItem(helpers::format("%ux%u", videoMode.width, videoMode.height));
        if(videoMode == SETTINGS.video.fullscreenSize)
            cbResolution->SetSelection(cbResolution->GetNumItems() - 1);
    }
    if(!cbResolution->GetSelection() && !supportedResolutions_.empty())
    {
        auto current = std::find(supportedResolutions_.begin(), supportedResolutions_.end(), VIDEODRIVER.GetWindowSize());
        cbResolution->SetSelection(current == supportedResolutions_.end() ? 0 : static_cast<unsigned>(current - supportedResolutions_.begin()));
    }
    const auto windowSizes = VIDEODRIVER.GetDefaultWindowSizes();
    windowSizes_.assign(windowSizes.begin(), windowSizes.end());
    cbWindowSize.AddItem(""); // Placeholder for current window size
    for(const auto& size : windowSizes_)
        cbWindowSize.AddItem(helpers::format("%ux%u", size.width, size.height));

    curPos.y += cbSize.y + 15;

    AddText(ID_txtMapScrollMode, curPos, _("Map scroll mode:"), COLOR_YELLOW, FontStyle{}, NormalFont);
    ctrlComboBox* cbMapScrollMode =
      AddComboBox(ID_cbMapScrollMode, curPos + rightColOffset, ctrlSize, TextureColor::Grey, NormalFont, 100);
    cbMapScrollMode->AddItem(
      _("Scroll same (Map moves in the same direction the mouse is moved when scrolling/panning.)"));
    cbMapScrollMode->AddItem(
      _("Scroll opposite (Map moves in the opposite direction the mouse is moved when scrolling/panning.)"));
    cbMapScrollMode->AddItem(_("Grab and drag (Map moves with your cursor when scrolling/panning.)"));
    cbMapScrollMode->SetSelection(static_cast<int>(SETTINGS.interface.mapScrollMode));

    curPos.y += ctrlSize.y + 10;
    AddCheckBox(ID_cbSmartCursor, curPos, cbSize, TextureColor::Grey, _("Smart Cursor"), NormalFont, false)
      ->setChecked(SETTINGS.global.smartCursor)
      ->SetTooltip(_("Place cursor on default button for new dialogs / action windows (default)"));
    curPos.y += cbSize.y + 3;
    AddCheckBox(ID_cbStatisticScale, curPos, cbSize, TextureColor::Grey, _("Statistics Scale"), NormalFont, false)
      ->setChecked(SETTINGS.ingame.scaleStatistics);

    if(VIDEODRIVER.GetName().find("OpenXR") != std::string::npos)
        AddTextButton(ID_btVr, DrawPoint(leftColOffset, GetSize().y - 78), Extent(330, 30),
                      TextureColor::Green2, _("VR: Controller / Bild"), NormalFont);
    curPos = DrawPoint(leftColOffset + btSpacing, GetSize().y - 40);
    AddTextButton(ID_btApply, curPos, btSize, TextureColor::Green2, _("Apply"), NormalFont, _("Apply Changes"));
    curPos.x += btSize.x + btSpacing;
    AddTextButton(ID_btAbort, curPos, btSize, TextureColor::Red1, _("Abort"), NormalFont, _("Close Without Saving"));

    updateWindowSizeComboBox();
    updateResolutionGroups();
}

void iwSettings::Msg_ButtonClick(unsigned ctrl_id)
{
    switch(ctrl_id)
    {
        case ID_btApply:
        {
            auto& grpWindowSize = *GetCtrl<ctrlGroup>(ID_grpWindowSize);
            SETTINGS.global.smartCursor = GetCtrl<ctrlCheck>(ID_cbSmartCursor)->isChecked();
            VIDEODRIVER.SetMouseWarping(SETTINGS.global.smartCursor);
            SETTINGS.ingame.scaleStatistics = GetCtrl<ctrlCheck>(ID_cbStatisticScale)->isChecked();
            SETTINGS.video.displayMode = DisplayMode(GetCtrl<ctrlComboBox>(ID_cbDisplayMode)->GetSelection().value());
            SETTINGS.video.displayMode.resizeable = !grpWindowSize.GetCtrl<ctrlCheck>(ID_cbLockWindowSize)->isChecked();
            SETTINGS.interface.mapScrollMode =
              static_cast<MapScrollMode>(GetCtrl<ctrlComboBox>(ID_cbMapScrollMode)->GetSelection().value());
            SETTINGS.video.fullscreenSize = supportedResolutions_
              [GetCtrl<ctrlGroup>(ID_grpResolution)->GetCtrl<ctrlComboBox>(ID_cbResolution)->GetSelection().value()];
            const unsigned windowSizeSelection =
              grpWindowSize.GetCtrl<ctrlComboBox>(ID_cbWindowSize)->GetSelection().value();
            if(windowSizeSelection > 0)
                SETTINGS.video.windowedSize = windowSizes_[windowSizeSelection - 1];
            else if(VIDEODRIVER.GetDisplayMode() == DisplayMode::Windowed)
                SETTINGS.video.windowedSize = VIDEODRIVER.GetWindowSize();
            closeBehavior_ = CloseBehavior::Regular;

            const auto screenSize = SETTINGS.video.displayMode == DisplayMode::Fullscreen ?
                                      SETTINGS.video.fullscreenSize :
                                      SETTINGS.video.windowedSize;
            if((SETTINGS.video.displayMode != DisplayMode::BorderlessWindow
                && VIDEODRIVER.GetWindowSize() != screenSize)
               || SETTINGS.video.displayMode != VIDEODRIVER.GetDisplayMode())
            {
                if(!VIDEODRIVER.ResizeScreen(screenSize, SETTINGS.video.displayMode))
                {
                    WINDOWMANAGER.Show(std::make_unique<iwMsgbox>(
                      _("Sorry!"), _("You need to restart your game to change the screen resolution!"), this,
                      MsgboxButton::Ok, MsgboxIcon::ExclamationGreen, 1));
                }
            }
            SETTINGS.Save();
        }
        break;
        case ID_btVr: WINDOWMANAGER.Show(std::make_unique<iwVrSettings>()); break;
        case ID_btAbort: Close(); break;
        default: RTTR_Assert(false);
    }
}

void iwSettings::Msg_ComboSelectItem(const unsigned ctrl_id, const unsigned /*selection*/)
{
    // Handle on apply and just disable closing the window without the buttons (apply/discard)
    closeBehavior_ = CloseBehavior::Custom;
    if(ctrl_id == ID_cbDisplayMode)
        updateResolutionGroups();
}

void iwSettings::Msg_CheckboxChange(const unsigned /*ctrl_id*/, const bool /*checked*/)
{
    // Handle on apply and just disable closing the window without the buttons (apply/discard)
    closeBehavior_ = CloseBehavior::Custom;
}

void iwSettings::Msg_Group_ComboSelectItem(unsigned /*group_id*/, unsigned ctrl_id, unsigned selection)
{
    if(ctrl_id == ID_cbWindowSize && selection > 0)
    {
        const DisplayMode currentDisplayMode = getDisplayModeSelection();
        // Directly apply new window size
        if(currentDisplayMode == DisplayMode::Windowed && VIDEODRIVER.GetDisplayMode() == DisplayMode::Windowed)
        {
            VIDEODRIVER.ResizeScreen(windowSizes_[selection - 1], DisplayMode::Windowed);
        } else
            closeBehavior_ = CloseBehavior::Custom;
    } else
        closeBehavior_ = CloseBehavior::Custom;
}

void iwSettings::Msg_ScreenResize(const ScreenResizeEvent& sr)
{
    IngameWindow::Msg_ScreenResize(sr);
    updateWindowSizeComboBox();
}

void iwSettings::updateWindowSizeComboBox()
{
    auto* cbWindowSize = GetCtrl<ctrlGroup>(ID_grpWindowSize)->GetCtrl<ctrlComboBox>(ID_cbWindowSize);

    const VideoMode currentSize =
      VIDEODRIVER.GetDisplayMode() == DisplayMode::Windowed ? VIDEODRIVER.GetWindowSize() : SETTINGS.video.windowedSize;
    cbWindowSize->SetText(0, helpers::format("%ux%u", currentSize.width, currentSize.height));
    // Not found == -1 -> First entry selected
    cbWindowSize->SetSelection(static_cast<unsigned>(helpers::indexOf(windowSizes_, currentSize) + 1));
}

void iwSettings::updateResolutionGroups()
{
    const DisplayMode currentDisplayMode = getDisplayModeSelection();
    GetCtrl<ctrlGroup>(ID_grpResolution)->SetVisible(currentDisplayMode == DisplayMode::Fullscreen);
    GetCtrl<ctrlGroup>(ID_grpWindowSize)->SetVisible(currentDisplayMode == DisplayMode::Windowed);
}

DisplayMode iwSettings::getDisplayModeSelection() const
{
    return static_cast<DisplayMode>(GetCtrl<ctrlComboBox>(ID_cbDisplayMode)->GetSelection().value());
}

namespace {
enum { VR_SWAP, VR_BRIGHTNESS, VR_CONTRAST, VR_DEFAULTS, VR_APPLY, VR_CANCEL,
       VR_LABEL_BRIGHTNESS, VR_LABEL_CONTRAST, VR_HINT, VR_CURVATURE, VR_LABEL_CURVATURE, VR_RAY_WIDTH, VR_RAY_COLOR, VR_MARKER_SHAPE, VR_MARKER_SIZE, VR_CURSOR_SIZE, VR_SHARPNESS, VR_START_DISTANCE, VR_COMFORT,
       VR_PTR_X, VR_PTR_Y, VR_PTR_X_VALUE, VR_PTR_Y_VALUE, VR_EXTRA_LABELS };
/// "0", "+1,25 %", "-0,50 %" (ASCII minus: the game font has no typographic minus sign).
std::string pointerOffsetText(int quarters)
{
    if(quarters == 0) return "0";
    const int hundredths = std::abs(quarters) * 25;
    return helpers::format("%c%d,%02d %%", quarters > 0 ? '+' : '-', hundredths / 100, hundredths % 100);
}
}

iwVrSettings::iwVrSettings()
    : IngameWindow(CGI_VR_SETTINGS, IngameWindow::posLastOrCenter, Extent(640, 560),
                   _("VR: Controller / Bild"), LOADER.GetImageN("resource", 41), true)
{
    SETTINGS.vrPreview = SETTINGS.video.vr;
    const auto choice=[&](unsigned id,int row,const char* label,std::initializer_list<const char*> names) {
        const int y=vrMenu::row(row%5);
        AddText(VR_EXTRA_LABELS+id,DrawPoint(vrMenu::labelX,y+8),_(label),COLOR_YELLOW,FontStyle{},NormalFont);
        auto* box=AddComboBox(id,DrawPoint(vrMenu::fieldX,y),Extent(vrMenu::fieldWidth,vrMenu::controlHeight),TextureColor::Grey,NormalFont,160);
        for(const auto* name:names) box->AddItem(_(name));
    };
    choice(VR_START_DISTANCE,0,"Startdistanz",{"Nah (1,2 m)","Mittel (1,8 m)","Weit (2,8 m)"});
    for(unsigned id:{VR_BRIGHTNESS,VR_CONTRAST}) {
        const int row=id==VR_BRIGHTNESS ? 1 : 2;
        choice(id,row,id==VR_BRIGHTNESS ? "Helligkeit" : "Kontrast",{});
        for(int value=50;value<=150;value+=10) GetCtrl<ctrlComboBox>(id)->AddItem(helpers::format("%d %%",value));
    }
    choice(VR_CURVATURE,3,"Fensterwölbung",{"Flach","Leicht gewölbt","Stärker gewölbt"});
    // "Aus" is the last entry and stored separately (rayVisible/markerVisible), so the chosen width/shape survives.
    // Hidden only on the bare map; menus always show ray and marker.
    choice(VR_RAY_WIDTH,5,"Strahl",{"Dünn","Mittel","Dick","Aus"});
    choice(VR_RAY_COLOR,6,"Strahlfarbe",{"Weiß / hell","Hellgrau","Dunkelgrau","Warmgelb"});
    choice(VR_MARKER_SHAPE,7,"Zielpunkt",{"Punkt","Ring","Kreuz","Aus"});
    choice(VR_MARKER_SIZE,8,"Zielpunktgröße",{"Klein","Mittel","Groß"});
    choice(VR_CURSOR_SIZE,9,"Maus-Icon",{"Klein (75 %)","Normal (100 %)","Groß (150 %)","Sehr groß (200 %)","Aus"});
    choice(VR_SHARPNESS,4,"Bildschärfe",{"Weich","Leicht weich","Neutral","Leicht scharf","Scharf"});
    // Pointer offset X/Y (duty P3): slider with "‹"/"›" buttons, 0.25 % per step, ±15 %; value shown on the right.
    for(unsigned id:{VR_PTR_X,VR_PTR_Y}) {
        const int y=vrMenu::row(id==VR_PTR_X ? 5 : 6);
        AddText(VR_EXTRA_LABELS+id,DrawPoint(vrMenu::labelX,y+8),_(id==VR_PTR_X ? "Zeiger waagerecht" : "Zeiger senkrecht"),COLOR_YELLOW,FontStyle{},NormalFont);
        AddProgress(id,DrawPoint(vrMenu::fieldX,y),Extent(vrMenu::fieldWidth-110,vrMenu::controlHeight),TextureColor::Grey,119,120,120);
        AddText(id==VR_PTR_X ? VR_PTR_X_VALUE : VR_PTR_Y_VALUE,DrawPoint(vrMenu::fieldX+vrMenu::fieldWidth-100,y+8),"",COLOR_YELLOW,FontStyle{},NormalFont);
    }
    AddTextButton(90,DrawPoint(20,35),Extent(295,32),TextureColor::Green2,_("Bild / Fenster"),NormalFont);
    AddTextButton(91,DrawPoint(320,35),Extent(295,32),TextureColor::Grey,_("Strahl / Zeiger"),NormalFont);
    AddTextButton(VR_DEFAULTS,DrawPoint(20,465),Extent(285,32),TextureColor::Grey,_("Standardwerte"),NormalFont);
    AddTextButton(VR_COMFORT,DrawPoint(320,465),Extent(295,32),TextureColor::Grey,_("Weitere VR-Optionen"),NormalFont);
    AddTextButton(VR_APPLY,DrawPoint(20,510),Extent(285,32),TextureColor::Green2,_("Übernehmen"),NormalFont);
    AddTextButton(VR_CANCEL,DrawPoint(320,510),Extent(295,32),TextureColor::Red1,_("Abbrechen"),NormalFont);
    refreshControls();
    showPage(false);
}

iwVrSettings::~iwVrSettings()
{
    SETTINGS.vrPreview.reset();
}

void iwVrSettings::refreshControls()
{
    const auto& vr = *SETTINGS.vrPreview;
    GetCtrl<ctrlComboBox>(VR_START_DISTANCE)->SetSelection(vr.startDistance);
    GetCtrl<ctrlComboBox>(VR_SHARPNESS)->SetSelection(vr.sharpness + 2);
    GetCtrl<ctrlComboBox>(VR_CURSOR_SIZE)->SetSelection(vr.cursorSize);
    GetCtrl<ctrlComboBox>(VR_MARKER_SIZE)->SetSelection(vr.markerSize);
    GetCtrl<ctrlComboBox>(VR_MARKER_SHAPE)->SetSelection(vr.markerVisible ? vr.markerShape : 3);
    GetCtrl<ctrlComboBox>(VR_RAY_COLOR)->SetSelection(vr.rayColor);
    GetCtrl<ctrlComboBox>(VR_RAY_WIDTH)->SetSelection(vr.rayVisible ? vr.rayWidth : 3);
    GetCtrl<ctrlComboBox>(VR_CURVATURE)->SetSelection(vr.curvature);
    GetCtrl<ctrlComboBox>(VR_BRIGHTNESS)->SetSelection((vr.brightness - 50) / 10);
    GetCtrl<ctrlComboBox>(VR_CONTRAST)->SetSelection((vr.contrast - 50) / 10);
    refreshPointerOffset();
}

void iwVrSettings::refreshPointerOffset()
{
    const auto& vr = *SETTINGS.vrPreview;
    GetCtrl<ctrlProgress>(VR_PTR_X)->SetPosition(static_cast<unsigned short>(vr.ptrOffsetX + 60));
    GetCtrl<ctrlProgress>(VR_PTR_Y)->SetPosition(static_cast<unsigned short>(vr.ptrOffsetY + 60));
    GetCtrl<ctrlText>(VR_PTR_X_VALUE)->SetText(pointerOffsetText(vr.ptrOffsetX));
    GetCtrl<ctrlText>(VR_PTR_Y_VALUE)->SetText(pointerOffsetText(vr.ptrOffsetY));
}

void iwVrSettings::Msg_ProgressChange(unsigned ctrl_id, unsigned short position)
{
    if(ctrl_id != VR_PTR_X && ctrl_id != VR_PTR_Y) return;
    // Preview applies at once (the driver reads vrPreview); slider position 0..120 = -15 %..+15 %.
    const int quarters = vrMenuModel::ptroff::clampQuarters(static_cast<int>(position) - 60);
    (ctrl_id == VR_PTR_X ? SETTINGS.vrPreview->ptrOffsetX : SETTINGS.vrPreview->ptrOffsetY) = quarters;
    refreshPointerOffset();
}

void iwVrSettings::Msg_ComboSelectItem(unsigned ctrl_id, unsigned selection)
{
    if(ctrl_id == VR_START_DISTANCE && selection <= 2) { SETTINGS.vrPreview->startDistance = selection; SETTINGS.vrPreview->screenDistanceMm=0; }
    if(ctrl_id == VR_SHARPNESS && selection <= 4) SETTINGS.vrPreview->sharpness = static_cast<int>(selection) - 2;
    if(ctrl_id == VR_CURVATURE && selection <= 2) SETTINGS.vrPreview->curvature = selection;
    if(ctrl_id == VR_RAY_WIDTH && selection <= 3)
    {
        SETTINGS.vrPreview->rayVisible = selection != 3;
        if(selection != 3) SETTINGS.vrPreview->rayWidth = selection;
    }
    if(ctrl_id == VR_RAY_COLOR && selection <= 3) SETTINGS.vrPreview->rayColor = selection;
    if(ctrl_id == VR_MARKER_SHAPE && selection <= 3)
    {
        SETTINGS.vrPreview->markerVisible = selection != 3;
        if(selection != 3) SETTINGS.vrPreview->markerShape = selection;
    }
    if(ctrl_id == VR_MARKER_SIZE && selection <= 2) SETTINGS.vrPreview->markerSize = selection;
    if(ctrl_id == VR_CURSOR_SIZE && selection <= 4) SETTINGS.vrPreview->cursorSize = selection;
    if(selection > 10) return;
    if(ctrl_id == VR_BRIGHTNESS) SETTINGS.vrPreview->brightness = 50 + 10 * selection;
    if(ctrl_id == VR_CONTRAST) SETTINGS.vrPreview->contrast = 50 + 10 * selection;
}

void iwVrSettings::Msg_CheckboxChange(unsigned ctrl_id, bool checked)
{
    (void)ctrl_id; (void)checked;
}

void iwVrSettings::showPage(bool pointer)
{
    for(unsigned id:{VR_START_DISTANCE,VR_BRIGHTNESS,VR_CONTRAST,VR_CURVATURE,VR_SHARPNESS}) {
        GetCtrl<ctrlComboBox>(id)->SetVisible(!pointer);GetCtrl<ctrlText>(VR_EXTRA_LABELS+id)->SetVisible(!pointer);
    }
    for(unsigned id:{VR_RAY_WIDTH,VR_RAY_COLOR,VR_MARKER_SHAPE,VR_MARKER_SIZE,VR_CURSOR_SIZE}) {
        GetCtrl<ctrlComboBox>(id)->SetVisible(pointer);GetCtrl<ctrlText>(VR_EXTRA_LABELS+id)->SetVisible(pointer);
    }
    for(unsigned id:{VR_PTR_X,VR_PTR_Y}) {
        GetCtrl<ctrlProgress>(id)->SetVisible(pointer);GetCtrl<ctrlText>(VR_EXTRA_LABELS+id)->SetVisible(pointer);
    }
    GetCtrl<ctrlText>(VR_PTR_X_VALUE)->SetVisible(pointer);GetCtrl<ctrlText>(VR_PTR_Y_VALUE)->SetVisible(pointer);
    GetCtrl<ctrlTextButton>(90)->SetTexture(pointer ? TextureColor::Grey : TextureColor::Green2);
    GetCtrl<ctrlTextButton>(91)->SetTexture(pointer ? TextureColor::Green2 : TextureColor::Grey);
}

void iwVrSettings::Msg_ButtonClick(unsigned ctrl_id)
{
    if(ctrl_id==90 || ctrl_id==91) { showPage(ctrl_id==91);return; }
    if(ctrl_id == VR_COMFORT)
    {
        const auto draft = *SETTINGS.vrPreview;
        Close();
        WINDOWMANAGER.Show(std::make_unique<iwVrComfort>(0, draft));
        return;
    }
    if(ctrl_id == VR_DEFAULTS)
    {
        SETTINGS.vrPreview = VrSettings{};
        refreshControls();
    } else if(ctrl_id == VR_APPLY)
    {
        SETTINGS.video.vr = *SETTINGS.vrPreview;
        SETTINGS.Save();
        Close();
    } else if(ctrl_id == VR_CANCEL)
        Close();
}
