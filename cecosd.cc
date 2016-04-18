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

#include <algorithm>
#include <stdexcept>
#include "cecremoteplugin.h"
#include "cecosd.h"
#include "ceclog.h"

using namespace std;
using namespace cecplugin;

namespace cecplugin {

std::vector<cCECMenu> cCECOsd::mMenuItems;

cCECOsd::cCECOsd(cPluginCecremote *plugin) :
                 cOsdMenu(tr("CEC Device")) {

  int cnt = 1;
  char num[4];
  cCECMenu menuitem;
  string menutxt;
  cCECMenuList *menulist = plugin->GetMenuList();

  for (cCECMenuListIterator i = menulist->begin();
       i != menulist->end(); i++) {
      menuitem = *i;
      if (cnt <= 9) {
          sprintf(num,"%d ", cnt);
      }
      else {
          strcpy (num, "  ");
      }

      menutxt = num + menuitem.mMenuTitle;
      Add(new cCECOsdItem(menuitem, menutxt.c_str(), plugin));
      mMenuItems.push_back(menuitem);
      cnt++;
  }
}

cCECOsdItem::cCECOsdItem(const cCECMenu &menuitem, const char *menutxt,
                         cPluginCecremote *plugin) :
        cOsdItem(menutxt), mControl(NULL) {
    mMenuItem = menuitem;
    mPlugin = plugin;
    Dsyslog("Menu %s", menutxt);
}

eOSState cCECOsdItem::ProcessKey(eKeys key) {
    eOSState state = osUnknown;

    if (key == kOk) {
        mPlugin->StartPlayer(mMenuItem);
        return osEnd;
    }
    if ((key > k0) && (key <= k9)) {
        try {
            mPlugin->StartPlayer(cCECOsd::mMenuItems.at(key-k1));
            state = osEnd;
        } catch (const std::out_of_range &oor) {
            Isyslog("StartPlayer Out of range");
        }
    }
    return state;
}

} // namespace cecplugin

