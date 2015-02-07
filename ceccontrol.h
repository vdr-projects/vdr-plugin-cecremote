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
