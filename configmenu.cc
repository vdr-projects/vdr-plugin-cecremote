/*
 * CECRemote PlugIn for VDR
 *
 * Copyright (C) 2015 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 * This class implements the configuration menu
 *
 */

#include "configmenu.h"

namespace cecplugin {

const char *cConfigMenu::ENABLEMAINMENU = "EnableMainMenu";
int cConfigMenu::mShowMainMenu = true;

cConfigMenu::cConfigMenu() : cMenuSetupPage()
{
    Add(new cMenuEditBoolItem(tr("Show in main menu"), &mShowMainMenu));
}

const bool cConfigMenu::SetupParse(const char *Name, const char *Value)
{
    if (strcasecmp(Name, ENABLEMAINMENU) == 0) {
        mShowMainMenu = atoi(Value);
    }
    else {
        return false;
    }
    return true;
}

void cConfigMenu::Store(void)
{
    SetupStore(ENABLEMAINMENU, (int)mShowMainMenu);
}

} // namespace cecplugin
