// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "iwSave.h"
#include "SaveName.h"
#include "controls/ctrlCheck.h"
#include <boost/filesystem/operations.hpp>
#include <ctime>
#include "ListDir.h"
#include "Loader.h"
#include "RttrConfig.h"
#include "RttrLobbyClient.hpp"
#include "Savegame.h"
#include "Settings.h"
#include "WindowManager.h"
#include "controls/ctrlComboBox.h"
#include "controls/ctrlEdit.h"
#include "controls/ctrlTable.h"
#include "controls/ctrlText.h"
#include "controls/ctrlTextButton.h"
#include "drivers/VideoDriverWrapper.h"
#include "driver/KeyEvent.h"
#include <s25util/utf8.h>
#include "desktops/dskLobby.h"
#include "files.h"
#include "helpers/make_array.h"
#include "helpers/toString.h"
#include "iwConnecting.h"
#include "iwMsgbox.h"
#include "network/GameClient.h"
#include "gameData/GameConsts.h"
#include "gameData/const_gui_ids.h"
#include "liblobby/LobbyClient.h"
#include "s25util/Log.h"
#include <boost/range/adaptors.hpp>
#include <utility>

namespace {
enum
{
    ID_tblSaveGames,
    ID_edtFilename,
    ID_btSaveOrLoad,
    ID_txtAutoSave,
    ID_cbAutoSaveInterval,
    ID_txtSaveFolder,
    ID_keyCaps,
    ID_keyBackspace,
    ID_keySpace,
    ID_keyLeft,
    ID_keyRight,
    ID_keyClear,
    ID_autoName,
    ID_keyFirst = 100,
};

const std::u32string KEYBOARD_ROWS[] = {U"1234567890", U"qwertzuiopü", U"asdfghjklöä", U"yxcvbnmß.-_"};
char32_t KeyboardCase(char32_t c, bool upper)
{
    if(!upper) return c;
    if(c >= U'a' && c <= U'z') return c - U'a' + U'A';
    if(c == U'ä') return U'Ä';
    if(c == U'ö') return U'Ö';
    if(c == U'ü') return U'Ü';
    return c;
}

using namespace std::chrono_literals;
constexpr std::array AUTO_SAVE_INTERVALS{1min, 5min, 10min, 15min, 30min, 60min, 90min};
} // namespace

iwSaveLoad::iwSaveLoad(const std::string& window_title, ITexture* btImg, const unsigned addHeight, const bool showKeyboard)
    : IngameWindow(CGI_SAVE, IngameWindow::posLastOrCenter, Extent(700, 400 + addHeight + (showKeyboard ? 50 : 0)), window_title,
                   LOADER.GetImageN("resource", 41))
{
    const int shift = showKeyboard ? 150 : 0;
    using SRT = ctrlTable::SortType;
    AddTable(ID_tblSaveGames, DrawPoint(20, 30), Extent(660, 300 - shift), TextureColor::Green2, NormalFont,
             ctrlTable::Columns{{_("Filename"), 320, SRT::String},
                                {_("Map"), 250, SRT::String},
                                {_("Time"), 160, SRT::Date},
                                {_("Game Time"), 115, SRT::Time},
                                {}});

    AddText(ID_txtSaveFolder, DrawPoint(20, 333 - shift), RTTRCONFIG.ExpandPath(s25::folders::save).string(), COLOR_YELLOW,
            FontStyle::TOP, SmallFont)
      ->setMaxWidth(510);
    // maxLength 251 = 255 filename limit - 4 chars for ".sav"; just discourages absurdly long
    // input, isValidFileName() may still reject it since it counts bytes, not codepoints.
    AddEdit(ID_edtFilename, DrawPoint(20, 350 - shift), Extent(510, 22), TextureColor::Green2, NormalFont, 251);
    AddImageButton(ID_btSaveOrLoad, DrawPoint(540, 341 - shift), Extent(40, 40), TextureColor::Green2, btImg);
    if(showKeyboard)
    {
        unsigned keyId = ID_keyFirst;
        for(unsigned row = 0; row < 4; ++row)
            for(unsigned col = 0; col < KEYBOARD_ROWS[row].size(); ++col)
                AddTextButton(keyId++, DrawPoint(20 + col*60, 268 + row*34), Extent(56, 30),
                              TextureColor::Green2, s25util::utf32to8(std::u32string(1, KEYBOARD_ROWS[row][col])), NormalFont);
        AddTextButton(ID_keyCaps, DrawPoint(20, 408), Extent(76, 34), TextureColor::Green2, "a/A", NormalFont);
        AddTextButton(ID_keySpace, DrawPoint(100, 408), Extent(210, 34), TextureColor::Green2, _("Space"), NormalFont);
        AddTextButton(ID_keyLeft, DrawPoint(314, 408), Extent(45, 34), TextureColor::Green2, "<", NormalFont);
        AddTextButton(ID_keyRight, DrawPoint(363, 408), Extent(45, 34), TextureColor::Green2, ">", NormalFont);
        AddTextButton(ID_keyBackspace, DrawPoint(412, 408), Extent(125, 34), TextureColor::Green2, _("Backspace"), NormalFont);
        AddTextButton(ID_keyClear, DrawPoint(541, 408), Extent(139, 34), TextureColor::Green2, _("Clear"), NormalFont);
    }
    // Initially fill the table
    RefreshTable();
}

void iwSaveLoad::Msg_EditEnter(const unsigned /*ctrl_id*/)
{
    SaveLoad();
}

void iwSaveLoad::Msg_ButtonClick(const unsigned ctrl_id)
{
    if(ctrl_id == ID_btSaveOrLoad)
    {
        SaveLoad();
        return;
    }
    auto* edit = GetCtrl<ctrlEdit>(ID_edtFilename);
    edit->SetFocus();
    if(ctrl_id == ID_keyCaps)
    {
        keyboardUpper_ = !keyboardUpper_;
        unsigned id = ID_keyFirst;
        for(const auto& row : KEYBOARD_ROWS)
            for(char32_t c : row)
                GetCtrl<ctrlTextButton>(id++)->SetText(s25util::utf32to8(std::u32string(1, KeyboardCase(c, keyboardUpper_))));
        return;
    }
    if(ctrl_id == ID_keyClear) { edit->SetText(""); return; }
    if(ctrl_id == ID_keyBackspace) { edit->Msg_KeyDown(KeyEvent(KeyType::Backspace)); return; }
    if(ctrl_id == ID_keyLeft) { edit->Msg_KeyDown(KeyEvent(KeyType::Left)); return; }
    if(ctrl_id == ID_keyRight) { edit->Msg_KeyDown(KeyEvent(KeyType::Right)); return; }
    if(ctrl_id == ID_keySpace) { edit->Msg_KeyDown(KeyEvent(U' ')); return; }
    unsigned id = ID_keyFirst;
    for(const auto& row : KEYBOARD_ROWS)
        for(char32_t c : row)
            if(id++ == ctrl_id)
            {
                edit->Msg_KeyDown(KeyEvent(KeyboardCase(c, keyboardUpper_)));
                return;
            }
}

void iwSaveLoad::Msg_TableSelectItem(const unsigned /*ctrl_id*/, const std::optional<unsigned>& selection)
{
    // On selecting a table entry put the filename into the edit control
    GetCtrl<ctrlEdit>(ID_edtFilename)
      ->SetText(selection ? GetCtrl<ctrlTable>(ID_tblSaveGames)->GetItemText(*selection, 0) : "");
}

void iwSaveLoad::RefreshTable()
{
    static bool loadedOnce = false;

    auto* table = GetCtrl<ctrlTable>(ID_tblSaveGames);

    table->DeleteAllItems();

    std::vector<boost::filesystem::path> saveFiles = ListDir(RTTRCONFIG.ExpandPath(s25::folders::save), "sav");
    for(const auto& saveFile : saveFiles)
    {
        Savegame save;
        if(!save.Load(saveFile, SaveGameDataToLoad::Header))
        {
            // Show errors only first time this is loaded
            if(!loadedOnce)
            {
                LOG.write(_("Invalid Savegame %1%! Reason: %2%\n")) % saveFile
                  % (save.GetLastErrorMsg().empty() ? _("Unknown") : save.GetLastErrorMsg());
            }
            continue;
        }

        const auto fileName = saveFile.stem().string();
        const std::string dateStr = s25util::Time::FormatTime("%d.%m.%Y - %H:%i", save.GetSaveTime());
        const std::string gameTime = GAMECLIENT.FormatGFTime(save.start_gf);

        table->AddRow({fileName, save.GetMapName(), dateStr, gameTime, saveFile.string()});
    }

    // Sort by time
    table->SortRows(2, TableSortDir::Descending);
    loadedOnce = true;
}

void iwSave::SaveLoad()
{
    if(SETTINGS.video.vr.automaticSaveName && GetCtrl<ctrlEdit>(ID_edtFilename)->GetText()==suggestedName_)
        SuggestName();
    const auto fileNameResult = GetCtrl<ctrlEdit>(ID_edtFilename)->GetFileName(".sav");
    switch(fileNameResult.status)
    {
        case FileNameStatus::Empty:
            WINDOWMANAGER.Show(std::make_unique<iwMsgbox>(_("Invalid Filename"), _("Please enter a filename."), this,
                                                          MsgboxButton::Ok, MsgboxIcon::ExclamationRed));
            return;
        case FileNameStatus::Invalid:
            WINDOWMANAGER.Show(std::make_unique<iwMsgbox>(_("Invalid Filename"), _("Please enter a valid filename."),
                                                          this, MsgboxButton::Ok, MsgboxIcon::ExclamationRed));
            return;
        case FileNameStatus::Valid: break;
    }
    GAMECLIENT.SaveToFile(RTTRCONFIG.ExpandPath(s25::folders::save) / fileNameResult.name);
    lastSavedName_=GetCtrl<ctrlEdit>(ID_edtFilename)->GetText();

    RefreshTable();
    GetCtrl<ctrlEdit>(ID_edtFilename)->SetText("");
    if(SETTINGS.video.vr.automaticSaveName) SuggestName();
}

iwSave::iwSave() : iwSaveLoad(_("Save game!"), LOADER.GetTextureN("io", 47), 80, false)
{
    auto* fileNameEdit = GetCtrl<ctrlEdit>(ID_edtFilename);
    fileNameEdit->SetType(EditType::Filename);
    AddCheckBox(ID_autoName,DrawPoint(20,425),Extent(655,30),TextureColor::Grey,
                _("Namen vorschlagen: Zeitstempel - Spieler - Karte"),NormalFont,false)
      ->setChecked(SETTINGS.video.vr.automaticSaveName);
    if(SETTINGS.video.vr.automaticSaveName) SuggestName();
    DrawPoint pos(GetSize().x / 2, fileNameEdit->GetPos().y + fileNameEdit->GetSize().y + 10);

    ctrlComboBox* combo =
      AddComboBox(ID_cbAutoSaveInterval, pos, Extent(130, 22), TextureColor::Green2, NormalFont, 100);
    pos += DrawPoint(-5, 11);
    AddText(ID_txtAutoSave, pos, _("Auto-Save every:"), 0xFFFFFF00, FontStyle::RIGHT | FontStyle::VCENTER, NormalFont);

    // Add intervals
    combo->AddItem(_("Disabled"));
    for(const std::chrono::minutes interval : AUTO_SAVE_INTERVALS)
        combo->AddItem((boost::format(_("%1% min")) % interval.count()).str());
    // Last entry is only for debugging
    if(SETTINGS.global.debugMode)
        combo->AddItem(_("Every GF"));

    // Select interval
    combo->SetSelection(0); // Use disabled by default and change if possible
    if(SETTINGS.interface.autosaveInterval == 1)
        combo->SetSelection(AUTO_SAVE_INTERVALS.size() + 1);
    else
    {
        // Start selection index at 1, 0 is "disabled"
        for(const auto& i : AUTO_SAVE_INTERVALS | boost::adaptors::indexed(1))
        {
            if(SETTINGS.interface.autosaveInterval == duration_to_gfs(i.value()))
            {
                combo->SetSelection(static_cast<unsigned>(i.index()));
                break;
            }
        }
    }
}

void iwSave::SuggestName()
{
    const auto now=std::time(nullptr);
    const auto* local=std::localtime(&now);
    char timestamp[32]{};
    if(local) std::strftime(timestamp,sizeof(timestamp),"%Y-%m-%d_%H-%M-%S",local);
    const auto base=saveName::stem(local ? timestamp : "Spielstand",GAMECLIENT.GetLocalPlayerName(),GAMECLIENT.GetMapTitle());
    auto candidate=base;
    unsigned suffix=2;
    while(candidate==lastSavedName_ || boost::filesystem::exists(RTTRCONFIG.ExpandPath(s25::folders::save)/(candidate+".sav")))
        candidate=base+"-"+std::to_string(suffix++);
    suggestedName_=candidate;
    GetCtrl<ctrlEdit>(ID_edtFilename)->SetText(candidate);
}

void iwSave::Msg_CheckboxChange(unsigned id,bool checked)
{
    if(id!=ID_autoName) return;
    SETTINGS.video.vr.automaticSaveName=checked;
    SETTINGS.Save();
    if(checked) SuggestName();
}

void iwSave::Msg_ComboSelectItem(const unsigned /*ctrl_id*/, const unsigned selection)
{
    if(selection == 0) // First entry is "disabled"
        SETTINGS.interface.autosaveInterval = 0;
    else if(selection > AUTO_SAVE_INTERVALS.size()) // Last entry is "every GF" (in debug mode)
        SETTINGS.interface.autosaveInterval = 1;
    else
    {
        // selection is the index into the array ignoring the first ("disabled") entry
        RTTR_Assert(selection >= 1 && selection <= AUTO_SAVE_INTERVALS.size());
        SETTINGS.interface.autosaveInterval = duration_to_gfs(AUTO_SAVE_INTERVALS[selection - 1]);
    }
}

iwLoad::iwLoad(CreateServerInfo csi) : iwSaveLoad(_("Load game!"), LOADER.GetTextureN("io", 48)), csi(std::move(csi)) {}

void iwLoad::SaveLoad()
{
    const auto* table = GetCtrl<ctrlTable>(ID_tblSaveGames);
    if(!table->GetSelection())
        return;

    if(GAMECLIENT.HostGame(csi, {table->GetItemText(*table->GetSelection(), 4), MapType::Savegame}))
    {
        std::unique_ptr<ILobbyClient> lobbyClient;
        if(csi.type == ServerType::Lobby)
            lobbyClient = std::make_unique<RttrLobbyClient>(LOBBYCLIENT);
        WINDOWMANAGER.Show(std::make_unique<iwConnecting>(csi.type, std::move(lobbyClient)));
    } else
    {
        if(LOBBYCLIENT.IsLoggedIn())
            WINDOWMANAGER.Switch(std::make_unique<dskLobby>());
        else
            Close();
    }
}

/// Handle double click on the table
void iwLoad::Msg_TableChooseItem(const unsigned /*ctrl_id*/, const unsigned /*selection*/)
{
    SaveLoad();
}
