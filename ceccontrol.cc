/*
 * CECRemote PlugIn for VDR
 *
 * Copyright (C) 2014, 2015 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 * This class implements the VDR cControl for the cec player
 *
 */

#include "ceccontrol.h"
#include "cecplayer.h"
#include "ceclog.h"

cCECControl::cCECControl(const cCECMenu &menuitem, cPluginCecremote *plugin) :
    cControl(mCECPlayer = new cCECPlayer(menuitem))
{
    mPlugin = plugin;
    mMenuItem = menuitem;
    mPlugin->mKeyMaps.SetActiveKeymaps(menuitem.mVDRKeymap, menuitem.mCECKeymap);
    mPlugin->ExecCmd(mMenuItem.onStart);
}

cCECControl::~cCECControl() {
    mPlugin->ExecCmd(mMenuItem.onStop);
    delete mCECPlayer;
}

void cCECControl::Hide(void)
{
    Dsyslog("Hide cCECControl");
}

eOSState cCECControl::ProcessKey(eKeys key)
{
    cKey k;
    if (key != kNone) Dsyslog("cCECControl ProcessKey %d %s", key, k.ToString(key,false));

    if (mMenuItem.isStopKey(key)) {
        Hide();
        return osEnd;
    }
    if (key == kNone) {
        return osContinue;
    }

    cCECCmd cmd(CEC_VDRKEYPRESS, (int)key, mMenuItem.mAddress);
    mPlugin->PushCmd(cmd);

    return (osContinue);
}

