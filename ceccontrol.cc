/*
 * ceccontrol.cc
 *
 *  Created on: 27.12.2014
 *      Author: uli
 */

#include "ceccontrol.h"
#include "cecplayer.h"
#include "ceclog.h"

cCECControl::cCECControl(const cCECDevInfo &config,
                         cPluginCecremote *plugin) :
    cControl(mCECPlayer = new cCECPlayer(config.mStillPic))
{
    mConfig = config;
    mPlugin = plugin;
}

cCECControl::~cCECControl() {
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
    Dsyslog("cCECControl ProcessKey %d", key);
    eOSState state = osContinue;
    switch (key) {
    case kMenu:
    case kStop:
        state = osEnd;
        break;

    default:
        cCECCmd cmd(CEC_VDRKEYPRESS, (int)key);
        mPlugin->PushCmd(cmd);
        break;
    }

    if (state == osEnd) {
        Hide();
    }
    return (state);
}


