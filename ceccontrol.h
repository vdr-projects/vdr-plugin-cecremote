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

#ifndef CECONTROL_H_
#define CECONTROL_H_

#include <vdr/player.h>

#include "cecremoteplugin.h"
#include "cecplayer.h"

class cCECControl: public cControl {
private:
    cCECPlayer *mCECPlayer;
    cPluginCecremote *mPlugin;
    cCECMenu mMenuItem;

public:
    cCECControl(const cCECMenu &menuitem, cPluginCecremote *plugin);
    virtual ~cCECControl();

    virtual void Hide(void);
    virtual cOsdObject *GetInfo(void) { return NULL; }
    virtual eOSState ProcessKey(eKeys Key);
};

#endif /* CECONTROL_H_ */
