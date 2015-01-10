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
    cControl(mCECPlayer = new cCECPlayer(config))
{
    mConfig = config;
    mPlugin = plugin;

    if (config.mPowerOn) {
        cCECCmd cmd(CEC_POWERON, 0, config.mAddr);
        mPlugin->PushCmd(cmd);
    }
    if (mConfig.mMakeActive) {
        cCECCmd cmd(CEC_MAKEINACTIVE);
        mPlugin->PushCmd(cmd);
    }
}

cCECControl::~cCECControl() {
    if (mConfig.mPowerOff) {
        cCECCmd cmd(CEC_POWEROFF, 0, mConfig.mAddr);
        mPlugin->PushCmd(cmd);
    }
    if (mConfig.mMakeActive) {
        cCECCmd cmd(CEC_MAKEACTIVE);
        mPlugin->PushCmd(cmd);
    }
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


