/*
 * cecosd.h
 *
 *  Created on: 27.12.2014
 *      Author: uli
 */

#ifndef CECOSD_H_
#define CECOSD_H_

#include <vdr/menu.h>
#include <vdr/plugin.h>
#include "ceccontrol.h"

class cCECOsd : public cOsdMenu {
public:
    cCECOsd(cPluginCecremote *plugin);
    virtual ~cCECOsd() {}
};

class cCECOsdItem : public cOsdItem {
private:
    cCECControl *mControl;
    cPluginCecremote *mPlugin;
    int mCnt;

public:
  cCECOsdItem(int cnt, const char *menutxt, cPluginCecremote *plugin);
  ~cCECOsdItem() {}
  virtual eOSState ProcessKey(eKeys key);
};

#endif /* CECOSD_H_ */
