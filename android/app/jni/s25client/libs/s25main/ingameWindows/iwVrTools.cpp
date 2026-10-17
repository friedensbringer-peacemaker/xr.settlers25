// SPDX-License-Identifier: GPL-2.0-or-later
#include "iwVrTools.h"
#include "VrMenuLayout.h"
#include "Loader.h"
#include "Settings.h"
#include "WindowManager.h"
#include "controls/ctrlEdit.h"
#include "controls/ctrlTextButton.h"
#include "controls/ctrlText.h"
#include "controls/ctrlComboBox.h"
#include "controls/ctrlProgress.h"
#include "controls/ctrlTimer.h"
#include "network/GameClient.h"
#include "GlobalVars.h"
#include <chrono>
#include "helpers/format.hpp"
#include "desktops/Desktop.h"
#include "desktops/dskGameInterface.h"
#include "driver/KeyEvent.h"
#include "drivers/VideoDriverWrapper.h"
#include "iwSettings.h"
#include "gameData/const_gui_ids.h"
#include "s25util/colors.h"
#include <s25util/utf8.h>
#include <algorithm>
#include <array>
#include <cmath>

namespace {
enum { INPUT, CAPS, SYMBOLS, SPACE, LEFT, RIGHT, BACKSPACE, CLEAR, ACCEPT, ENTER, CANCEL, FIRST_KEY = 100 };
const std::array<std::u32string, 4> letters = {U"1234567890", U"qwertzuiopü", U"asdfghjklöä", U"yxcvbnmß.-_"};
const std::array<std::u32string, 4> symbols = {U"!@#$%&*()=", U"[]{}<>/\\|~", U"?:;,+-_.\"'", U"0123456789"};
char32_t upper(char32_t c)
{
    if(c >= U'a' && c <= U'z') return c - U'a' + U'A';
    if(c == U'ä') return U'Ä';
    if(c == U'ö') return U'Ö';
    if(c == U'ü') return U'Ü';
    return c;
}
}

iwVrKeyboard::iwVrKeyboard(ctrlEdit& target)
    : IngameWindow(CGI_VR_KEYBOARD, IngameWindow::posLastOrCenter, Extent(720, 365),
                   _("VR-Tastatur"), LOADER.GetImageN("resource", 41), true),
      target_(&target), targetLifetime_(target.GetLifetime())
{
    auto* input = AddEdit(INPUT, DrawPoint(20, 35), Extent(680, 34), TextureColor::Green2, NormalFont,
                          target.GetMaxLength(), target.IsPassword());
    input->SetVrKeyboardEnabled(false);
    input->SetType(target.GetEditType());
    input->SetText(target.GetText());
    input->SetFocus();
    for(unsigned row = 0; row < 4; ++row)
        for(unsigned col = 0; col < 11; ++col)
            AddTextButton(FIRST_KEY + row*11 + col, DrawPoint(20 + col*62, 83 + row*39), Extent(58, 35),
                          TextureColor::Green2, "", NormalFont);
    AddTextButton(CAPS, DrawPoint(20, 246), Extent(65, 36), TextureColor::Grey, "a/A", NormalFont);
    AddTextButton(SYMBOLS, DrawPoint(90, 246), Extent(75, 36), TextureColor::Grey, "@#?", NormalFont);
    AddTextButton(SPACE, DrawPoint(170, 246), Extent(190, 36), TextureColor::Grey, _("Space"), NormalFont);
    AddTextButton(LEFT, DrawPoint(365, 246), Extent(45, 36), TextureColor::Grey, "<", NormalFont);
    AddTextButton(RIGHT, DrawPoint(415, 246), Extent(45, 36), TextureColor::Grey, ">", NormalFont);
    AddTextButton(BACKSPACE, DrawPoint(465, 246), Extent(135, 36), TextureColor::Grey, _("Backspace"), NormalFont);
    AddTextButton(CLEAR, DrawPoint(605, 246), Extent(95, 36), TextureColor::Grey, _("Clear"), NormalFont);
    AddTextButton(ACCEPT, DrawPoint(20, 309), Extent(220, 36), TextureColor::Green2, _("Apply"), NormalFont);
    AddTextButton(ENTER, DrawPoint(250, 309), Extent(220, 36), TextureColor::Green2, _("Enter / Absenden"), NormalFont);
    AddTextButton(CANCEL, DrawPoint(480, 309), Extent(220, 36), TextureColor::Red1, _("Abort"), NormalFont);
    updateKeys();
}

void iwVrKeyboard::updateKeys()
{
    const auto& rows = symbols_ ? symbols : letters;
    for(unsigned row = 0; row < 4; ++row)
        for(unsigned col = 0; col < 11; ++col)
        {
            auto* button = GetCtrl<ctrlTextButton>(FIRST_KEY + row*11 + col);
            button->SetVisible(col < rows[row].size());
            if(col < rows[row].size())
            {
                const auto c = upper_ && !symbols_ ? upper(rows[row][col]) : rows[row][col];
                button->SetText(s25util::utf32to8(std::u32string(1, c)));
            }
        }
}

void iwVrKeyboard::accept(bool enter)
{
    if(!targetLifetime_.expired() && !target_->IsDisabled())
    {
        target_->SetText(GetCtrl<ctrlEdit>(INPUT)->GetText());
        if(!targetLifetime_.expired())
        {
            target_->SetFocus();
            if(enter) target_->Msg_KeyDown(KeyEvent(KeyType::Return));
        }
    }
    Close();
}

void iwVrKeyboard::Msg_EditEnter(unsigned) { accept(true); }

void iwVrKeyboard::Msg_ButtonClick(unsigned id)
{
    auto* input = GetCtrl<ctrlEdit>(INPUT);
    input->SetFocus();
    if(id == CANCEL) { Close(); return; }
    if(id == ACCEPT || id == ENTER) { accept(id == ENTER); return; }
    if(id == CAPS) { upper_ = !upper_; updateKeys(); return; }
    if(id == SYMBOLS) { symbols_ = !symbols_; updateKeys(); return; }
    if(id == CLEAR) input->SetText("");
    if(id == SPACE) input->Msg_KeyDown(KeyEvent(U' '));
    if(id == LEFT) input->Msg_KeyDown(KeyEvent(KeyType::Left));
    if(id == RIGHT) input->Msg_KeyDown(KeyEvent(KeyType::Right));
    if(id == BACKSPACE) input->Msg_KeyDown(KeyEvent(KeyType::Backspace));
    if(id >= FIRST_KEY && id < FIRST_KEY + 44)
    {
        const auto& rows = symbols_ ? symbols : letters;
        const auto row = (id - FIRST_KEY) / 11, col = (id - FIRST_KEY) % 11;
        if(col < rows[row].size()) input->Msg_KeyDown(KeyEvent(upper_ && !symbols_ ? upper(rows[row][col]) : rows[row][col]));
    }
}

namespace {
const char* actionNames[] = {"Aus", "Kartenzoom", "Baumenü", "Spielmenü / Transport", "Geschwindigkeit",
    "Speichern", "Pause / Weiter", "Minikarte", "Hauptquartier", "Bauhilfe", "Inventar", "Nachrichten",
    "Chat", "Gebäudenamen", "Produktivität", "Fenster zentrieren", "VR-Schnellmenü", "Ansicht wechseln", "Passthrough an/aus"};
struct Shortcut { const char* label; KeyEvent key; };
const Shortcut shortcuts[] = {
    {"Speichern", KeyEvent(KeyType::F2)}, {"Pause / Weiter", KeyEvent(U'p')},
    {"Minikarte", KeyEvent(U'l')}, {"Hauptquartier", KeyEvent(U'h')},
    {"Bauhilfe", KeyEvent(U' ')}, {"Gebäudenamen", KeyEvent(U'c')},
    {"Produktivität", KeyEvent(U's')}, {"Inventar", KeyEvent(U'i')},
    {"Nachrichten", KeyEvent(U'n')}, {"Chat", KeyEvent(KeyType::Return)},
    {"Schneller", KeyEvent(U'+')}, {"Langsamer", KeyEvent(U'-')},
    {"Kartenzoom +", KeyEvent(U'z')}, {"Kartenzoom -", KeyEvent(U'Z')},
    {"Spielmenü", KeyEvent(U'm')}, {"Musik", KeyEvent(KeyType::F11)},
    {"Tastenhilfe", KeyEvent(KeyType::F8)}, {"Letzte Position", KeyEvent(U'b')}
};
enum { QUICK_VR = 100, QUICK_SETTINGS, QUICK_SCALE_DOWN, QUICK_SCALE_UP, QUICK_CLOSE, QUICK_SCALE_LABEL, QUICK_TOGGLE, QUICK_COMFORT, QUICK_CENTER, QUICK_VIEW, QUICK_PASSTHROUGH, QUICK_OPACITY, QUICK_OPACITY_LABEL, QUICK_TABLE, QUICK_SCREENSHOT, QUICK_RECORD, QUICK_QUIT, QUICK_QUIT_TIMER };
// Table simulator presets: tilt index (30/45/60/75 degrees), centre below eye level in cm, distance index (0.8/1.2/1.8 m).
struct TablePreset { const char* label; int tilt, lowering, distance; };
const TablePreset tablePresets[] = {
    {"Schreibtisch", 1, 40, 0}, {"Couchtisch", 0, 70, 1}, {"Stehpult", 2, 20, 0}, {"Großer Tisch", 0, 50, 2}};
enum { TABLE_TILT_DOWN = 10, TABLE_TILT_UP, TABLE_HEIGHT_DOWN, TABLE_HEIGHT_UP, TABLE_DIST_DOWN, TABLE_DIST_UP,
       TABLE_VIEW = 20, TABLE_CENTER, TABLE_CLOSE, TABLE_TILT_LABEL = 30, TABLE_HEIGHT_LABEL, TABLE_DIST_LABEL,
       TABLE_TITLE = 40, TABLE_HINT, TABLE_ROW_LABEL = 50 };
}

iwVrQuickMenu::iwVrQuickMenu(bool favorites)
    : IngameWindow(CGI_VR_QUICK_MENU, IngameWindow::posLastOrCenter, Extent(690, favorites ? 330 : 530),
                   _("VR-Schnellmenü"), LOADER.GetImageN("resource", 41), true), favorites_(favorites)
{
    // Two stages (skill xr-vr-menu P5): the favorites page holds the 6 free favorites plus exactly 8 large
    // buttons; the second page holds all game shortcuts and the tools. Every entry exists once; the VR comfort
    // pages are reached through "VR-Einstellungen" -> "Weitere VR-Optionen".
    const bool inGame = dynamic_cast<dskGameInterface*>(WINDOWMANAGER.GetCurrentDesktop()) != nullptr;
    const auto big = [&](unsigned id, unsigned slot, TextureColor tc, const std::string& text) {
        AddTextButton(id, DrawPoint(20 + (slot%2)*330, 115 + (slot/2)*45), Extent(320, 38), tc, text, NormalFont);
    };
    if(favorites)
    {
        for(unsigned i=0; i<6; ++i)
        {
            const int action=SETTINGS.video.vr.favorites[i];
            AddTextButton(200+i, DrawPoint(20+(i%3)*220,35+(i/3)*40),Extent(210,32),TextureColor::Green2,
                          _(actionNames[action]),NormalFont)->SetEnabled(action && (inGame || action>=15));
        }
        big(QUICK_CLOSE, 0, TextureColor::Green2, _("Weiter"));
        big(QUICK_CENTER, 1, TextureColor::Grey, _("Fenster zentrieren"));
        big(QUICK_VIEW, 2, TextureColor::Grey,
            SETTINGS.video.vr.presentation == 0 ? _("Zur Tischansicht wechseln") : _("Zur Leinwand wechseln"));
        big(QUICK_PASSTHROUGH, 3, TextureColor::Grey, "");
        big(QUICK_VR, 4, TextureColor::Grey, _("VR-Einstellungen"));
        big(QUICK_SETTINGS, 5, TextureColor::Grey, _("Spiel-Einstellungen"));
        big(QUICK_TOGGLE, 6, TextureColor::Grey, _("Werkzeuge / alle Aktionen"));
        // Dangerous action, therefore red; armed state see refreshQuit().
        big(QUICK_QUIT, 7, TextureColor::Red1, "");
        AddTimer(QUICK_QUIT_TIMER,std::chrono::milliseconds(100))->Stop(); // runs only while armed
        refreshPassthrough();
        // Greyed out when the runtime offers no passthrough (P3).
        GetCtrl<ctrlTextButton>(QUICK_PASSTHROUGH)->SetEnabled(VIDEODRIVER.GetDriver()->VrPassthroughAvailable());
        refreshQuit();
        return;
    }
    for(unsigned i = 0; i < sizeof(shortcuts)/sizeof(shortcuts[0]); ++i)
        AddTextButton(i, DrawPoint(20 + (i%3)*220, 35 + (i/3)*40), Extent(210, 32), TextureColor::Green2,
                      _(shortcuts[i].label), NormalFont)->SetEnabled(inGame);
    const int base = 275;
    AddTextButton(QUICK_SCALE_DOWN, DrawPoint(20, base), Extent(155, 32), TextureColor::Grey, _("GUI kleiner"), NormalFont);
    AddTextButton(QUICK_SCALE_UP, DrawPoint(185, base), Extent(155, 32), TextureColor::Grey, _("GUI größer"), NormalFont);
    AddTextButton(QUICK_TABLE, DrawPoint(350, base), Extent(320, 32), TextureColor::Green2, _("Tisch-Simulator"), NormalFont);
    AddText(QUICK_SCALE_LABEL, DrawPoint(20, base+48), "", COLOR_YELLOW, FontStyle{}, NormalFont);
    AddTextButton(QUICK_SCREENSHOT, DrawPoint(20, base+80), Extent(650, 32), TextureColor::Green2,
                  SETTINGS.video.vr.screenshotMode ? _("Screenshot-Modus beenden") : _("Screenshot-Modus"), NormalFont);
    const int recordStatus=VIDEODRIVER.GetDriver()->VrRecordStatus();
    const char* recordLabel = recordStatus==2 ? "Aufnahme beenden (läuft)" : recordStatus==3 ? "Aufnahme fehlgeschlagen - zurücksetzen"
                                              : "Video aufnehmen (Spielbild + Ton)";
    AddTextButton(QUICK_RECORD, DrawPoint(20, base+120), Extent(650, 32), recordStatus==2 ? TextureColor::Red1 : TextureColor::Green2,
                  _(recordLabel), NormalFont)->SetEnabled(recordStatus!=0);
    AddText(QUICK_OPACITY_LABEL, DrawPoint(20, base+168), "", COLOR_YELLOW, FontStyle{}, NormalFont);
    AddProgress(QUICK_OPACITY, DrawPoint(300, base+160), Extent(370, 32), TextureColor::Grey, 119, 120, 10);
    AddTextButton(QUICK_TOGGLE, DrawPoint(20, base+210), Extent(320, 32), TextureColor::Grey, _("Favoriten"), NormalFont);
    AddTextButton(QUICK_CLOSE, DrawPoint(350, base+210), Extent(320, 32), TextureColor::Green2, _("Weiter"), NormalFont);
    refreshPassthrough();
    refreshScale();
}

void iwVrQuickMenu::refreshQuit()
{
    auto* button=GetCtrl<ctrlTextButton>(QUICK_QUIT);
    const bool armed=quitConfirm_.armed();
    // Armed must be visible at a glance: other red, lit, yellow text - not only a status line.
    button->SetTexture(armed ? TextureColor::Red2 : TextureColor::Red1);
    button->SetIlluminated(armed);
    button->SetTextColor(armed ? COLOR_YELLOW : COLOR_WHITE);
    button->SetText(armed ? helpers::format(_("Sicher? Nochmal klicken (%u s)"),
                                            static_cast<unsigned>(std::ceil(quitConfirm_.remaining())))
                          : std::string(_("Spiel beenden")));
}

void iwVrQuickMenu::quitGame()
{
    // Save first: settings always, the running game (not replays) into its autosave file.
    SETTINGS.Save();
    if(dynamic_cast<dskGameInterface*>(WINDOWMANAGER.GetCurrentDesktop()))
        GAMECLIENT.SaveAutosaveNow();
    GLOBALVARS.notdone = false; // same clean exit as "Quit" in the main menu
}

void iwVrQuickMenu::Msg_Timer(unsigned id)
{
    if(id!=QUICK_QUIT_TIMER) return;
    const auto now=std::chrono::steady_clock::now();
    const float dt=std::chrono::duration<float>(now-quitTick_).count();
    quitTick_=now;
    const unsigned before=static_cast<unsigned>(std::ceil(quitConfirm_.remaining()));
    if(quitConfirm_.update(dt)) { GetCtrl<ctrlTimer>(QUICK_QUIT_TIMER)->Stop(); refreshQuit(); return; }
    if(static_cast<unsigned>(std::ceil(quitConfirm_.remaining()))!=before) refreshQuit();
}

void iwVrQuickMenu::Msg_ButtonClick(unsigned id)
{
    if(id==QUICK_QUIT)
    {
        if(quitConfirm_.click()==vrMenuModel::ConfirmAction::Confirmed) { quitGame(); return; }
        quitTick_=std::chrono::steady_clock::now();
        GetCtrl<ctrlTimer>(QUICK_QUIT_TIMER)->Start();
        auto& vr=SETTINGS.video.vr;
        ++vr.hapticConfirmRequest; // double pulse in the OpenXR driver
        if(SETTINGS.vrPreview) SETTINGS.vrPreview->hapticConfirmRequest=vr.hapticConfirmRequest;
        refreshQuit();
        return;
    }
    // Any other action disarms "Beenden"; closing the window discards the state with it.
    if(quitConfirm_.armed()) { quitConfirm_.cancel(); GetCtrl<ctrlTimer>(QUICK_QUIT_TIMER)->Stop(); refreshQuit(); }
    if(id==QUICK_PASSTHROUGH && !VIDEODRIVER.GetDriver()->VrPassthroughAvailable()) return;
    if(id==QUICK_PASSTHROUGH) {
        KeyEvent key(KeyType::VrAction);key.c=18;WINDOWMANAGER.Msg_KeyDown(key);
        refreshPassthrough();return;
    }
    if(id == QUICK_SCALE_DOWN || id == QUICK_SCALE_UP)
    {
        const auto range = VIDEODRIVER.getGuiScaleRange();
        const unsigned current = VIDEODRIVER.getGuiScale().percent();
        const unsigned next = id == QUICK_SCALE_UP ? std::min(range.maxPercent, current + 10)
                                                   : std::max(range.minPercent, current > 10 ? current - 10 : 0);
        SETTINGS.video.guiScale = next;
        VIDEODRIVER.setGuiScalePercent(next);
        SETTINGS.Save();
        refreshScale();
        return;
    }
    Close();
    if(id >= 200 && id < 206) { KeyEvent key(KeyType::VrAction); key.c=SETTINGS.video.vr.favorites[id-200]; WINDOWMANAGER.Msg_KeyDown(key); }
    else if(id == QUICK_VIEW) { KeyEvent key(KeyType::VrAction); key.c=17; WINDOWMANAGER.Msg_KeyDown(key); }
    else if(id == QUICK_CENTER) ++SETTINGS.video.vr.recenterRequest;
    else if(id == QUICK_TABLE) WINDOWMANAGER.Show(std::make_unique<iwVrTable>());
    else if(id == QUICK_RECORD)
    {
        // Transient; the OpenXR driver starts/stops the encoder on the next frame. After a failed start this resets.
        const bool on = !SETTINGS.video.vr.recordVideo;
        SETTINGS.video.vr.recordVideo = on;
        if(SETTINGS.vrPreview) SETTINGS.vrPreview->recordVideo = on;
    }
    else if(id == QUICK_SCREENSHOT)
    {
        // Transient: hides ray/marker/cursor on the bare map and ignores short focus loss (Meta button combo).
        const bool on = !SETTINGS.video.vr.screenshotMode;
        SETTINGS.video.vr.screenshotMode = on;
        if(SETTINGS.vrPreview) SETTINGS.vrPreview->screenshotMode = on;
    }
    else if(id == QUICK_TOGGLE) WINDOWMANAGER.Show(std::make_unique<iwVrQuickMenu>(!favorites_));
    else if(id == QUICK_VR) WINDOWMANAGER.Show(std::make_unique<iwVrSettings>());
    else if(id == QUICK_SETTINGS) WINDOWMANAGER.Show(std::make_unique<iwSettings>());
    else if(id < sizeof(shortcuts)/sizeof(shortcuts[0]))
    {
        if(auto* game = dynamic_cast<dskGameInterface*>(WINDOWMANAGER.GetCurrentDesktop()))
            static_cast<Window*>(game)->Msg_KeyDown(shortcuts[id].key);
    }
}

void iwVrQuickMenu::refreshPassthrough()
{
    const auto& vr=SETTINGS.video.vr;
    // Toggle on the favorites page, opacity slider on the tools page.
    if(auto* button=GetCtrl<ctrlTextButton>(QUICK_PASSTHROUGH))
        button->SetText(vr.passthrough ? _("Passthrough ausschalten") : _("Passthrough einschalten"));
    if(auto* slider=GetCtrl<ctrlProgress>(QUICK_OPACITY)) slider->SetPosition(vr.passthroughPercent/10);
    if(auto* label=GetCtrl<ctrlText>(QUICK_OPACITY_LABEL)) label->SetText(helpers::format("Umgebung: %d %%",vr.passthroughPercent));
}
void iwVrQuickMenu::Msg_ProgressChange(unsigned id,unsigned short position)
{
    if(id!=QUICK_OPACITY) return;
    auto& vr=SETTINGS.video.vr;
    vr.passthroughPercent=std::min<unsigned>(10,position)*10;
    vr.passthrough=vr.passthroughPercent>0;
    if(SETTINGS.vrPreview) { SETTINGS.vrPreview->passthrough=vr.passthrough; SETTINGS.vrPreview->passthroughPercent=vr.passthroughPercent; }
    SETTINGS.Save();refreshPassthrough();
}

void iwVrQuickMenu::refreshScale()
{
    const auto range = VIDEODRIVER.getGuiScaleRange();
    const auto scale = VIDEODRIVER.getGuiScale().percent();
    GetCtrl<ctrlText>(QUICK_SCALE_LABEL)->SetText(helpers::format("GUI: %u %% (max. %u %% bei dieser Auflösung)", scale, range.maxPercent));
    GetCtrl<ctrlTextButton>(QUICK_SCALE_UP)->SetEnabled(scale < range.maxPercent);
    GetCtrl<ctrlTextButton>(QUICK_SCALE_DOWN)->SetEnabled(scale > range.minPercent);
}

iwVrComfort::iwVrComfort(unsigned page, const VrSettings& draft)
    : IngameWindow(CGI_VR_COMFORT, IngameWindow::posLastOrCenter, Extent(640,560),
                   _("VR-Komfort"), LOADER.GetImageN("resource",41),true), page_(page), draft_(draft)
{
    const unsigned pages[]={1,4,0,2,3};
    const char* groups[]={"Controller","Fenster","Komfort","Favoriten","Grafik"};
    const unsigned group=(page==1 || page==5 || page==6) ? 1 : (page==4 || page==7) ? 4 : page;
    for(unsigned i=0;i<5;++i) AddTextButton(300+pages[i],DrawPoint(20+i*120,35),Extent(115,32),
        pages[i]==group ? TextureColor::Green2 : TextureColor::Grey,_(groups[i]),NormalFont);
    if(group==1) {
        const unsigned subpages[]={1,6,5};const char* names[]={"Tasten","Achsen","Navigation"};
        for(unsigned i=0;i<3;++i) AddTextButton(600+subpages[i],DrawPoint(20+i*200,75),Extent(195,32),
            page==subpages[i] ? TextureColor::Green2 : TextureColor::Grey,_(names[i]),NormalFont);
    } else if(group==4) {
        AddTextButton(604,DrawPoint(20,75),Extent(295,32),page==4 ? TextureColor::Green2 : TextureColor::Grey,_("Darstellung / Tisch"),NormalFont);
        AddTextButton(607,DrawPoint(320,75),Extent(295,32),page==7 ? TextureColor::Green2 : TextureColor::Grey,_("Position / Abstand"),NormalFont);
    }
    const auto choice=[&](unsigned id,int y,const char* label, std::initializer_list<const char*> names,unsigned selection) {
        AddText(100+id,DrawPoint(vrMenu::labelX,y+8),_(label),COLOR_YELLOW,FontStyle{},NormalFont);
        auto* box=AddComboBox(id,DrawPoint(vrMenu::fieldX,y),Extent(vrMenu::fieldWidth,vrMenu::controlHeight),TextureColor::Grey,NormalFont,180);
        for(auto name:names) box->AddItem(_(name)); box->SetSelection(selection);
    };
    const auto actionChoice=[&](unsigned id,int y,const std::string& label,int action) {
        AddText(100+id,DrawPoint(vrMenu::labelX,y+8),label,COLOR_YELLOW,FontStyle{},NormalFont);
        auto* box=AddComboBox(id,DrawPoint(vrMenu::fieldX,y),Extent(vrMenu::fieldWidth,vrMenu::controlHeight),TextureColor::Grey,NormalFont,190);
        for(auto name:actionNames) box->AddItem(_(name)); box->SetSelection(action);
    };
    if(page==0) {
        choice(0,vrMenu::row(0),"Zeigerglättung",{"Aus","Leicht","Stärker"},draft_.smoothing);
        choice(1,vrMenu::row(1),"Klick-Vibration",{"Aus","An"},draft_.haptics);
        choice(2,vrMenu::row(2),"Zeigerhand",{"Rechts","Links"},draft_.leftHanded);
        choice(3,vrMenu::row(3),"Pause bei Fokusverlust",{"Aus","Einzelspieler"},draft_.autoPause);
        choice(6,vrMenu::row(4),"Klick stabilisieren",{"Aus","An"},draft_.clickStabilization);
        choice(7,vrMenu::row(5),"Schaltflächen hervorheben",{"Aus","An"},draft_.hoverHighlight);
        choice(8,vrMenu::row(6),"Online-Gebäudehilfe",{"Aus","settlers2.net: Bauanleitung","settlers2.net: Datenblatt","SiedlerCommunity","JH-ISCH: Gebäudetipps"},draft_.buildingHelpSite);
    } else if(page==1) {
        const char* keys[]={"X links","Y links","A rechts","B rechts"};
        for(unsigned i=0;i<4;++i) {
            actionChoice(i,vrMenu::row(i*2),std::string(keys[i])+" kurz",draft_.buttons[i]);
            actionChoice(4+i,vrMenu::row(i*2+1),std::string(keys[i])+" lang",draft_.longButtons[i]);
        }
        AddText(90,DrawPoint(20,450),_("Lang: 0,6 s halten. Kurz erst beim Loslassen, falls Lang belegt."),COLOR_YELLOW,FontStyle{},NormalFont);
    } else if(page==2) {
        for(unsigned i=0;i<6;++i) actionChoice(i,vrMenu::row(i),helpers::format("Favorit %u",i+1),draft_.favorites[i]);
    } else if(page==6) {
        choice(0,vrMenu::row(0),"Links: X/Y tauschen",{"Aus","An"},draft_.swapLeftStickAxes);
        choice(1,vrMenu::row(1),"Links: X umkehren",{"Aus","An"},draft_.invertLeftX);
        choice(2,vrMenu::row(2),"Links: Y umkehren",{"Aus","An"},draft_.invertLeftY);
        choice(3,vrMenu::row(3),"Rechts: X/Y tauschen",{"Aus","An"},draft_.swapRightStickAxes);
        choice(4,vrMenu::row(4),"Rechts: X umkehren",{"Aus","An"},draft_.invertRightX);
        choice(5,vrMenu::row(5),"Rechts: Y umkehren",{"Aus","An"},draft_.invertRightY);
        AddText(90,DrawPoint(20,450),_("Stick-Zoom: immer vor/zurück; Y-Umkehrung gilt."),COLOR_YELLOW,FontStyle{},NormalFont);
    } else if(page==5) {
        choice(2,vrMenu::row(0),"Kartentempo",{"Normal (1x)","Schnell (2x)","Sehr schnell (4x)"},draft_.mapSpeed);
        choice(3,vrMenu::row(1),"Karten-Turbo (3x)",{"Aus","Rechten Stick gedrückt halten","Doppeltippen + halten","Beide Möglichkeiten"},draft_.mapBoost);
        choice(6,vrMenu::row(2),"Listen mit Stick scrollen",{"Aus","An"},draft_.smartScroll);
        choice(7,vrMenu::row(3),"Trigger halten: Karte greifen",{"Aus","An (bewegen + vor/zurück zoomt)"},draft_.triggerGrab);
        choice(8,vrMenu::row(4),"Rechter Griff: Scrolltempo",{"1x (Karte folgt dem Zeiger)","2x","3x","4x"},draft_.gripScrollSpeed);
        AddText(90,DrawPoint(20,450),_("Linker Griff + Stick: Zoom. Rechter Griff: Karte ziehen."),COLOR_YELLOW,FontStyle{},NormalFont);
    } else if(page==7) {
        choice(0,vrMenu::row(0),"Fenster am Rand verschieben",{"Aus","An"},draft_.frameMove);
        choice(1,vrMenu::row(1),"Stick: Größe / Abstand",{"Ohne Grip","Gesperrt"},draft_.panelControl==0 ? 0 : 1);
        choice(5,vrMenu::row(2),"Letzten Abstand merken",{"Aus","An"},draft_.rememberDistance);
        choice(6,vrMenu::row(3),"Fenstergröße merken",{"Aus","An"},draft_.rememberPanel);
    } else if(page==4) {
        choice(0,vrMenu::row(0),"Darstellung",{"Leinwand","Tischansicht"},draft_.presentation);
        choice(1,vrMenu::row(1),"Neigung des Spielbretts",{"30 Grad","45 Grad","60 Grad","75 Grad"},draft_.tableTilt);
        choice(2,vrMenu::row(2),"Tischmitte unter Blickhöhe",{"0 cm","10 cm","20 cm","30 cm","40 cm","50 cm","60 cm","70 cm","80 cm","90 cm"},draft_.tableLoweringCm/10);
        choice(3,vrMenu::row(3),"Tisch-Startabstand",{"Nah (0,8 m)","Mittel (1,2 m)","Weit (1,8 m)"},draft_.tableDistance);
        choice(4,vrMenu::row(4),"Beta: 3D-Hauptquartier",{"Aus","HQ-Modell auf der Karte"},draft_.headquarters3d);
        choice(5,vrMenu::row(5),"Tisch-Quelle",{"Simuliert (Einstellungen oben)","Echter Tisch (Raumscan)"},draft_.realTable);
        const char* tableStatus[]={"Echter Tisch: von dieser Quest nicht unterstützt.","Echter Tisch: Raumdaten-Freigabe in der Brille erlauben.",
            "Echter Tisch: suche im Raumscan ...","Echter Tisch: keiner gefunden - Raum in den Quest-Einstellungen scannen.",
            "Echter Tisch gefunden: Brett liegt flach darauf."};
        const int status=std::max(0,std::min(4,VIDEODRIVER.GetDriver()->VrTableStatus()));
        AddText(89,DrawPoint(20,vrMenu::row(6)),SETTINGS.video.vr.realTable ? _(tableStatus[status]) : _("Echter Tisch: aus, simulierter Tisch aktiv."),COLOR_YELLOW,FontStyle{},NormalFont);
        AddText(90,DrawPoint(20,vrMenu::row(7)),_("Kleinerer Höhenwert = höherer Tisch. 0 cm = Blickhöhe."),COLOR_YELLOW,FontStyle{},NormalFont);
        AddText(91,DrawPoint(20,vrMenu::row(8)),_("Beta-HQ: nur im Tischmodus; externes Modell erforderlich."),COLOR_YELLOW,FontStyle{},NormalFont);
    } else {
        const char* profiles[]={"Beste Leistung (1600 x 1000)","Ausgewogen (1920 x 1200)","Beste Lesbarkeit (3840 x 2400)"};
        for(unsigned i=0;i<3;++i) AddTextButton(410+i,DrawPoint(20,vrMenu::row(i)),Extent(595,32),TextureColor::Green2,_(profiles[i]),NormalFont)->SetEnabled(VIDEODRIVER.ListVideoModes().count(i==0 ? VideoMode(1600,1000) : i==1 ? VideoMode(1920,1200) : VideoMode(3840,2400)) != 0);
        AddText(90,DrawPoint(20,vrMenu::row(4)),_("Die Bildfilter bleiben erhalten. GUI: 160 / 200 / 400 %."),COLOR_YELLOW,FontStyle{},NormalFont);
        AddText(91,DrawPoint(20,vrMenu::row(5)),_("Hohe Auflösung benötigt mehr Grafikleistung."),COLOR_YELLOW,FontStyle{},NormalFont);
    }
    AddTextButton(500,DrawPoint(20,510),Extent(285,32),TextureColor::Green2,_("Übernehmen"),NormalFont);
    AddTextButton(501,DrawPoint(320,510),Extent(295,32),TextureColor::Red1,_("Abbrechen"),NormalFont);
}
void iwVrComfort::Msg_ComboSelectItem(unsigned id,unsigned selection)
{
    if(page_==0) switch(id) {
        case 8:draft_.buildingHelpSite=selection;break;
        case 0:draft_.smoothing=selection;break;case 1:draft_.haptics=selection;break;
        case 2:draft_.leftHanded=selection;break;case 3:draft_.autoPause=selection;break;case 4:draft_.rememberPanel=selection;break;
        case 5:draft_.smartScroll=selection;break;case 6:draft_.clickStabilization=selection;break;case 7:draft_.hoverHighlight=selection;break;
    }
    if(page_==1 && id<8) (id<4 ? draft_.buttons[id] : draft_.longButtons[id-4])=selection;
    if(page_==2 && id<6) draft_.favorites[id]=selection;
    if(page_==6) switch(id) {
        case 0:draft_.swapLeftStickAxes=selection;break;
        case 1:draft_.invertLeftX=selection;break;
        case 2:draft_.invertLeftY=selection;break;
        case 3:draft_.swapRightStickAxes=selection;break;
        case 4:draft_.invertRightX=selection;break;
        case 5:draft_.invertRightY=selection;break;
    }
    if(page_==5 || page_==7) switch(id) {
        case 0:draft_.frameMove=selection;break;case 1:draft_.panelControl=selection==0 ? 0 : 2;break;
        case 2:draft_.mapSpeed=selection;break;case 3:draft_.mapBoost=selection;break;
        case 5:draft_.rememberDistance=selection;break;
        case 6: if(page_==5) draft_.smartScroll=selection; else draft_.rememberPanel=selection;break;
        case 7: if(page_==5) draft_.triggerGrab=selection;break;
        case 8: if(page_==5) draft_.gripScrollSpeed=selection;break;
    }
    if(page_==4) switch(id) {
        case 0:draft_.presentation=selection;break;
        case 1:draft_.tableTilt=selection;break;
        case 2:draft_.tableLoweringCm=selection*10;break;
        case 3:draft_.tableDistance=selection;break;
        case 4:draft_.headquarters3d=selection;break;
        case 5:draft_.realTable=selection;break;
    }
}
void iwVrComfort::Msg_ButtonClick(unsigned id)
{
    if(id>=600 && id<=607) { Close(); WINDOWMANAGER.Show(std::make_unique<iwVrComfort>(id-600,draft_)); return; }
    if(id>=300 && id<=307) { Close(); WINDOWMANAGER.Show(std::make_unique<iwVrComfort>(id-300,draft_)); return; }
    if(id>=410 && id<=412)
    {
        const VideoMode modes[]={{1600,1000},{1920,1200},{3840,2400}};
        const auto modesAvailable=VIDEODRIVER.ListVideoModes();
        if(modesAvailable.count(modes[id-410]) && VIDEODRIVER.ResizeScreen(modes[id-410],VIDEODRIVER.GetDisplayMode()))
        {
            VIDEODRIVER.setGuiScalePercent(id==410 ? 160 : id==411 ? 200 : 400);
            SETTINGS.video.windowedSize=SETTINGS.video.fullscreenSize=VIDEODRIVER.GetWindowSize();
            SETTINGS.Save();
        }
        return;
    }
    if(id==500)
    {
        auto& vr=SETTINGS.video.vr;
        vr.buildingHelpSite=draft_.buildingHelpSite;
        vr.headquarters3d=draft_.headquarters3d;vr.realTable=draft_.realTable;
        vr.frameMove=draft_.frameMove;
        vr.swapLeftStickAxes=draft_.swapLeftStickAxes;
        vr.invertLeftX=draft_.invertLeftX;vr.invertLeftY=draft_.invertLeftY;
        vr.invertRightX=draft_.invertRightX;vr.swapRightStickAxes=draft_.swapRightStickAxes;
        vr.rayWidth=draft_.rayWidth;vr.rayColor=draft_.rayColor;vr.rayVisible=draft_.rayVisible;vr.markerVisible=draft_.markerVisible;
        vr.markerShape=draft_.markerShape;vr.markerSize=draft_.markerSize;vr.cursorSize=draft_.cursorSize;
        vr.brightness=draft_.brightness;vr.contrast=draft_.contrast;vr.sharpness=draft_.sharpness;
        vr.invertRightY=draft_.invertRightY;vr.panelControl=draft_.panelControl;
        vr.mapSpeed=draft_.mapSpeed;vr.mapBoost=draft_.mapBoost;vr.triggerGrab=draft_.triggerGrab;vr.gripScrollSpeed=draft_.gripScrollSpeed;
        vr.startCentered=draft_.startCentered;vr.rememberDistance=draft_.rememberDistance;
        if(vr.tableDistance!=draft_.tableDistance) vr.tableDistanceMm=0;
        if(vr.startDistance!=draft_.startDistance) vr.screenDistanceMm=0;
        vr.presentation=draft_.presentation;vr.tableTilt=draft_.tableTilt;
        vr.tableLoweringCm=draft_.tableLoweringCm;vr.tableDistance=draft_.tableDistance;
        vr.curvature=draft_.curvature;vr.startDistance=draft_.startDistance;
        vr.smartScroll=draft_.smartScroll;vr.clickStabilization=draft_.clickStabilization;vr.hoverHighlight=draft_.hoverHighlight;
        vr.smoothing=draft_.smoothing;vr.haptics=draft_.haptics;vr.leftHanded=draft_.leftHanded;
        vr.autoPause=draft_.autoPause;vr.rememberPanel=draft_.rememberPanel;
        vr.buttons=draft_.buttons;vr.longButtons=draft_.longButtons;vr.favorites=draft_.favorites;
        if(SETTINGS.vrPreview) *SETTINGS.vrPreview=vr;
        SETTINGS.Save();
    }
    Close();
}
iwVrTable::iwVrTable()
    : IngameWindow(CGI_VR_TABLE, IngameWindow::posLastOrCenter, Extent(640, 330), _("Tisch-Simulator"),
                   LOADER.GetImageN("resource", 41), true)
{
    // Control IDs must be unique per window (preset buttons use 0..3, see the TABLE_* enum).
    AddText(TABLE_TITLE, DrawPoint(20, 38), _("Voreinstellung – wirkt sofort:"), COLOR_YELLOW, FontStyle{}, NormalFont);
    for(unsigned i = 0; i < 4; ++i)
        AddTextButton(i, DrawPoint(20 + i * 150, 62), Extent(145, 32), TextureColor::Green2, _(tablePresets[i].label), NormalFont);
    const char* rows[] = {"Neigung des Spielbretts", "Tischmitte unter Blickhöhe", "Abstand zum Tisch"};
    for(unsigned r = 0; r < 3; ++r)
    {
        const int y = 112 + static_cast<int>(r) * 40;
        AddText(TABLE_ROW_LABEL + r, DrawPoint(20, y + 8), _(rows[r]), COLOR_YELLOW, FontStyle{}, NormalFont);
        AddTextButton(TABLE_TILT_DOWN + 2 * r, DrawPoint(330, y), Extent(60, 32), TextureColor::Grey, "-", NormalFont);
        AddText(TABLE_TILT_LABEL + r, DrawPoint(400, y + 8), "", COLOR_YELLOW, FontStyle{}, NormalFont);
        AddTextButton(TABLE_TILT_UP + 2 * r, DrawPoint(555, y), Extent(60, 32), TextureColor::Grey, "+", NormalFont);
    }
    AddText(TABLE_HINT, DrawPoint(20, 240), _("Änderungen werden sofort übernommen und gespeichert."), COLOR_YELLOW, FontStyle{}, NormalFont);
    // "Zentrieren" and "Leinwand/Tisch" exist once, in the VR quick menu (P5).
    AddTextButton(TABLE_CLOSE, DrawPoint(20, 275), Extent(595, 32), TextureColor::Red1, _("Fertig"), NormalFont);
    refresh();
}

void iwVrTable::refresh()
{
    const auto& vr = SETTINGS.video.vr;
    const char* distances[] = {"Nah (0,8 m)", "Mittel (1,2 m)", "Weit (1,8 m)"};
    GetCtrl<ctrlText>(TABLE_TILT_LABEL)->SetText(helpers::format("%d Grad", 30 + 15 * vr.tableTilt));
    GetCtrl<ctrlText>(TABLE_HEIGHT_LABEL)->SetText(helpers::format("%d cm", vr.tableLoweringCm));
    GetCtrl<ctrlText>(TABLE_DIST_LABEL)->SetText(_(distances[vr.tableDistance]));
    GetCtrl<ctrlTextButton>(TABLE_TILT_DOWN)->SetEnabled(vr.tableTilt > 0);
    GetCtrl<ctrlTextButton>(TABLE_TILT_UP)->SetEnabled(vr.tableTilt < 3);
    GetCtrl<ctrlTextButton>(TABLE_HEIGHT_DOWN)->SetEnabled(vr.tableLoweringCm > 0);
    GetCtrl<ctrlTextButton>(TABLE_HEIGHT_UP)->SetEnabled(vr.tableLoweringCm < 90);
    GetCtrl<ctrlTextButton>(TABLE_DIST_DOWN)->SetEnabled(vr.tableDistance > 0);
    GetCtrl<ctrlTextButton>(TABLE_DIST_UP)->SetEnabled(vr.tableDistance < 2);
}

void iwVrTable::apply(int tilt, int lowering, int distance)
{
    tilt = std::max(0, std::min(3, tilt));
    lowering = std::max(0, std::min(90, lowering));
    distance = std::max(0, std::min(2, distance));
    auto& vr = SETTINGS.video.vr;
    // A new distance or switching to the table places the board again in front of the user.
    const bool reposition = vr.tableDistance != distance || vr.presentation != 1;
    const auto update = [&](VrSettings& s) {
        if(s.tableDistance != distance) s.tableDistanceMm = 0;
        s.tableTilt = tilt; s.tableLoweringCm = lowering; s.tableDistance = distance; s.presentation = 1;
    };
    update(vr);
    if(SETTINGS.vrPreview) update(*SETTINGS.vrPreview);
    if(reposition) ++vr.recenterRequest;
    SETTINGS.Save();
    refresh();
}

void iwVrTable::Msg_ButtonClick(unsigned id)
{
    const auto& vr = SETTINGS.video.vr;
    if(id < 4) { apply(tablePresets[id].tilt, tablePresets[id].lowering, tablePresets[id].distance); return; }
    switch(id)
    {
        case TABLE_TILT_DOWN: apply(vr.tableTilt - 1, vr.tableLoweringCm, vr.tableDistance); return;
        case TABLE_TILT_UP: apply(vr.tableTilt + 1, vr.tableLoweringCm, vr.tableDistance); return;
        case TABLE_HEIGHT_DOWN: apply(vr.tableTilt, vr.tableLoweringCm - 10, vr.tableDistance); return;
        case TABLE_HEIGHT_UP: apply(vr.tableTilt, vr.tableLoweringCm + 10, vr.tableDistance); return;
        case TABLE_DIST_DOWN: apply(vr.tableTilt, vr.tableLoweringCm, vr.tableDistance - 1); return;
        case TABLE_DIST_UP: apply(vr.tableTilt, vr.tableLoweringCm, vr.tableDistance + 1); return;
        case TABLE_CLOSE: Close(); return;
        default: return;
    }
}

iwVrResume::iwVrResume()
    : IngameWindow(CGI_VR_RESUME, IngameWindow::posLastOrCenter,Extent(520,210),_("Spiel pausiert"),LOADER.GetImageN("resource",41),true)
{
    AddText(0,DrawPoint(20,45),_("Die Quest war nicht mehr im Spiel. Bereit weiterzuspielen?"),COLOR_YELLOW,FontStyle{},NormalFont);
    AddTextButton(1,DrawPoint(20,100),Extent(480,36),TextureColor::Green2,_("Ja, weiterspielen"),NormalFont);
    AddTextButton(2,DrawPoint(20,150),Extent(480,36),TextureColor::Grey,_("Pausiert lassen"),NormalFont);
}
void iwVrResume::Msg_ButtonClick(unsigned id) { if(id==1) GAMECLIENT.SetPause(false); Close(); }

iwVrResolution::iwVrResolution()
    : IngameWindow(CGI_VR_RESOLUTION, IngameWindow::posLastOrCenter,Extent(560,220),_("Bilddarstellung bestätigen"),LOADER.GetImageN("resource",41),true)
{
    closeBehavior_=CloseBehavior::Custom;
    AddText(0,DrawPoint(20,45),_("Ist alles lesbar und erreichbar? Rückkehr nach 15 Sekunden."),COLOR_YELLOW,FontStyle{},NormalFont);
    AddTextButton(1,DrawPoint(20,105),Extent(250,42),TextureColor::Green2,_("Behalten"),NormalFont);
    AddTextButton(2,DrawPoint(285,105),Extent(250,42),TextureColor::Red1,_("Zurück"),NormalFont);
    AddText(3,DrawPoint(20,170),"",COLOR_YELLOW,FontStyle{},NormalFont);
    AddTimer(4,std::chrono::milliseconds(200));
}
void iwVrResolution::Msg_ButtonClick(unsigned id) { VIDEODRIVER.ConfirmVrResolution(id==1); Close(); }
void iwVrResolution::Msg_Timer(unsigned) {
    const unsigned seconds=VIDEODRIVER.VrResolutionSeconds();
    GetCtrl<ctrlText>(3)->SetText(helpers::format("Verbleibend: %u s",seconds));
    if(!seconds) Close();
}
