/*
 * CECRemote PlugIn for VDR
 *
 * Copyright (C) 2014 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 * This class implements the configuration menu
 *
 */

#ifndef CONFIGMENU_H_
#define CONFIGMENU_H_

#include <vdr/plugin.h>

class cCECConfigMenu: public cMenuSetupPage {
private:
   // static int mWorkingMode;
    static int mShowMainMenu;
   // static const char *WORKINGMODE;
    static const char *ENABLEMAINMENU;
   // static cMediaDetectorThread *mDetector;

    cCECConfigMenu(void) {};

protected:
    virtual void Store(void);

public:
    static const bool GetShowMainMenu(void) { return mShowMainMenu; }
    static const bool SetupParse(const char *Name, const char *Value);
};

#endif /* CONFIGMENU_H_ */
