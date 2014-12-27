/*
 * cecosd.cc
 *
 *  Created on: 27.12.2014
 *      Author: uli
 */

#include <algorithm>
#include "cecremoteplugin.h"
#include "cecosd.h"
//#include "ceclog.h"

using namespace std;

cCECOsd::cCECOsd(cPluginCecremote *plugin) :
                 cOsdMenu(tr("CEC Device")) {

  int cnt = 1;
  char num[4];
  cCECDevInfo nConf;
  string menutxt;
  cCECDevInfoList *infolist = plugin->GetDevInfoList();

  for (cCECDevInfoListIterator i = infolist->begin();
       i != infolist->end(); i++) {
      nConf = *i;
      if (cnt <= 9) {
          sprintf(num,"%d ", cnt);
      }
      else {
          strcpy (num, "  ");
      }

      menutxt = num + nConf.mMenuName;
      Add(new cCECOsdItem(cnt, menutxt.c_str(), plugin));
      cnt++;
  }
}

cCECOsdItem::cCECOsdItem(int cnt, const char *menutxt,
                         cPluginCecremote *plugin) :
        cOsdItem(menutxt), mControl(NULL) {
    mCnt = cnt;
    mPlugin = plugin;
}

eOSState cCECOsdItem::ProcessKey(eKeys key) {
    eOSState state = osUnknown;

    if (key == kOk) {
        mPlugin->StartPlayer(mCnt);
        return osEnd;
    }
    if ((key > k0) && (key <= k9)) {
        try {
            mPlugin->StartPlayer(key-k0);
            state = osEnd;
        } catch (const std::out_of_range &oor) {
        }
    }
    return state;
}
