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

#include "cecconfigmenu.h"

const char *cCECConfigMenu::ENABLEMAINMENU = "EnableMainMenu";
int cCECConfigMenu::mShowMainMenu = true;

const bool cCECConfigMenu::SetupParse(const char *Name, const char *Value)
{
    if (strcasecmp(Name, ENABLEMAINMENU) == 0) {
        mShowMainMenu = atoi(Value);
    }
    else {
        return false;
    }
    return true;
}

void cCECConfigMenu::Store(void)
{
    SetupStore(ENABLEMAINMENU, (int)mShowMainMenu);
}
