// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "IngameWindow.h"
#include <chrono>
#include <memory>
#include "driver/VrMenuModel.h"
#include "driver/VrSettings.h"
class ctrlEdit;

class iwVrKeyboard : public IngameWindow
{
public:
    explicit iwVrKeyboard(ctrlEdit& target);
private:
    ctrlEdit* target_;
    std::weak_ptr<int> targetLifetime_;
    bool upper_ = false, symbols_ = false;
    void updateKeys();
    void accept(bool enter);
    void Msg_ButtonClick(unsigned id) override;
    void Msg_EditEnter(unsigned id) override;
};

class iwVrQuickMenu : public IngameWindow
{
public:
    explicit iwVrQuickMenu(bool favorites = true);
private:
    bool favorites_;
    /// "Beenden": first click arms visibly, second click within 3 s saves and quits.
    vrMenuModel::ConfirmAction quitConfirm_;
    std::chrono::steady_clock::time_point quitTick_;
    void refreshScale();
    void refreshPassthrough();
    void refreshQuit();
    void quitGame();
    void Msg_ProgressChange(unsigned id,unsigned short position) override;
    void Msg_ButtonClick(unsigned id) override;
    void Msg_Timer(unsigned id) override;
};

class iwVrComfort : public IngameWindow
{
public:
    explicit iwVrComfort(unsigned page = 0, const VrSettings& draft = VrSettings{});
private:
    unsigned page_;
    VrSettings draft_;
    void Msg_ButtonClick(unsigned id) override;
    void Msg_ComboSelectItem(unsigned id, unsigned selection) override;
};
/// Table simulator: presets and +/- steps for the tabletop view, applied and saved immediately.
class iwVrTable : public IngameWindow
{
public:
    iwVrTable();
private:
    void apply(int tilt, int lowering, int distance);
    void refresh();
    void Msg_ButtonClick(unsigned id) override;
};
class iwVrResume : public IngameWindow
{
public:
    iwVrResume();
private:
    void Msg_ButtonClick(unsigned id) override;
};
class iwVrResolution : public IngameWindow
{
public:
    iwVrResolution();
private:
    void Msg_ButtonClick(unsigned id) override;
    void Msg_Timer(unsigned id) override;
};
