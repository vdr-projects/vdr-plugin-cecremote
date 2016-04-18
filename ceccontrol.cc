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
#include "stillpicplayer.h"
#include "ceclog.h"

namespace cecplugin {

cCECControl::cCECControl(const cCECMenu &menuitem, cPluginCecremote *plugin) :
    cControl(mStillPicPlayer = new cStillPicPlayer(menuitem))
{
    mPlugin = plugin;
    mMenuItem = menuitem;
    mPlugin->mKeyMaps.SetActiveKeymaps(menuitem.mVDRKeymap, menuitem.mCECKeymap);
    mPlugin->PushCmdQueue(mMenuItem.mOnStart);
}

cCECControl::~cCECControl() {
    mPlugin->PushCmdQueue(mMenuItem.mOnStop);
    mPlugin->SetDefaultKeymaps();
    delete mStillPicPlayer;
}

void cCECControl::Hide(void)
{
    Dsyslog("Hide cCECControl");
}

/*
 * Get VDR keys and queue them to send to the active CEC device.
 */
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

    key = (eKeys)((int)key & ~k_Repeat);
    cCmd cmd(CEC_VDRKEYPRESS, (int)key, &mMenuItem.mDevice);
    mPlugin->PushCmd(cmd);

    return (osContinue);
}

} // namespace cecplugin

