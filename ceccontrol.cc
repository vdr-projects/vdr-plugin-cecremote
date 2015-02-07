/*
 * ceccontrol.cc
 *
 *  Created on: 27.12.2014
 *      Author: uli
 */

#include "ceccontrol.h"
#include "cecplayer.h"
#include "ceclog.h"

cCECControl::cCECControl(const cCECMenu &menuitem, cPluginCecremote *plugin) :
    cControl(mCECPlayer = new cCECPlayer(menuitem))
{
    mPlugin = plugin;
    mMenuItem = menuitem;
    mPlugin->ExecCmd(mMenuItem.onStart);
}

cCECControl::~cCECControl() {
    mPlugin->ExecCmd(mMenuItem.onStop);
    delete mCECPlayer;
}

void cCECControl::Hide(void)
{
    Dsyslog("Hide cCECControl");
 //   cMutexLock MutexLock(&mControlMutex);
  /*  if (mMenuPlaylist != NULL) {
        mMenuPlaylist->Clear();
        delete mMenuPlaylist;
        cStatus::MsgOsdClear();
#ifdef USE_GRAPHTFT
        cStatus::MsgOsdMenuDestroy();
#endif
    }
    mMenuPlaylist = NULL;*/
}


eOSState cCECControl::ProcessKey(eKeys key)
{
    cKey k;
    if (key != kNone) Dsyslog("cCECControl ProcessKey %d %s", key, k.ToString(key,false));
    eOSState state = osContinue;
    switch (key) {
    case kMenu:
    case kStop:
        state = osEnd;
        break;
    case kNone:
        break;
    default:
        cCECCmd cmd(CEC_VDRKEYPRESS, (int)key, mMenuItem.mAddress);
        mPlugin->PushCmd(cmd);
        break;
    }

    if (state == osEnd) {
        Hide();
    }
    return (state);
}


