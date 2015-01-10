/*
 * cecontrol.h
 *
 *  Created on: 27.12.2014
 *      Author: uli
 */

#ifndef CECONTROL_H_
#define CECONTROL_H_

#include <vdr/player.h>

#include "cecremoteplugin.h"
#include "cecplayer.h"

class cCECControl: public cControl {
private:
    cCECDevInfo mConfig;
    cCECPlayer *mCECPlayer;
    cPluginCecremote *mPlugin;

public:
    cCECControl(const cCECDevInfo &config, cPluginCecremote *plugin);
    virtual ~cCECControl();

    virtual void Hide(void);
    virtual cOsdObject *GetInfo(void) { return NULL; }
    virtual eOSState ProcessKey(eKeys Key);
};

#endif /* CECONTROL_H_ */