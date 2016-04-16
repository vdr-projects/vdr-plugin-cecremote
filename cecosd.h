/*
 * CECRemote PlugIn for VDR
 *
 * Copyright (C) 2015 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 * This class implements the OSD menu.
 */

#ifndef CECOSD_H_
#define CECOSD_H_

#include <vdr/menu.h>
#include <vdr/plugin.h>
#include "ceccontrol.h"

namespace cecplugin {

class cCECOsd : public cOsdMenu {
public:
    static std::vector<cCECMenu> mMenuItems;
    cCECOsd(cPluginCecremote *plugin);
    virtual ~cCECOsd() {}
};

class cCECOsdItem : public cOsdItem {
private:
    cCECControl *mControl;
    cPluginCecremote *mPlugin;
    cCECMenu mMenuItem;

public:
  cCECOsdItem(const cCECMenu &menuitem, const char *menutxt, cPluginCecremote *plugin);
  ~cCECOsdItem() {}
  virtual eOSState ProcessKey(eKeys key);
};

} // namespace cecplugin

#endif /* CECOSD_H_ */
