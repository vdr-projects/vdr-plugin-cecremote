/*
 * CECRemote PlugIn for VDR
 *
 * Copyright (C) 2015 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 * This class implements the main VDR plugin code.
 */

#ifndef CECREMOTEPLUGIN_H
#define CECREMOTEPLUGIN_H

#include <string>

#include <vdr/plugin.h>
#include "cecremote.h"
#include "cecconfigmenu.h"
#include "configfileparser.h"
#include "cecstatusmonitor.h"

namespace cecplugin {
class cCECOsd;
class cCECStatusMonitor;

class cPluginCecremote : public cPlugin {
    friend class cCECStatusMonitor;
protected:
    int mCECLogLevel;

    std::string mCfgDir;
    std::string mCfgFile;

    cConfigFileParser mConfigFileParser;
    cCECRemote *mCECRemote;
    cCECStatusMonitor *mStatusMonitor;
    bool mStartManually;

    const std::string GetConfigDir(void) {
        const std::string cfdir = ConfigDirectory();
        if (mCfgDir[0] == '/') {
            return mCfgDir  + "/";
        }
        return cfdir + "/" + mCfgDir + "/";
    }
    const std::string GetConfigFile(void) {
        const std::string cf = GetConfigDir() + mCfgFile;
        return cf;
    }
    void ExecToggle(cCECMenu menu) {
        cCECCmd cmd(CEC_EXECTOGGLE, menu.mDevice, menu.mOnPowerOn, menu.mOnPowerOff);
        mCECRemote->PushWaitCmd(cmd);
    }

public:
    cCECkeymaps mKeyMaps;

    cPluginCecremote(void);
    virtual ~cPluginCecremote();
    virtual const char *Version(void);
    virtual const char *Description(void);
    virtual const char *CommandLineHelp(void);
    virtual bool ProcessArgs(int argc, char *argv[]);
    virtual bool Initialize(void);
    virtual bool Start(void);
    virtual void Stop(void);
    virtual void Housekeeping(void);
    virtual void MainThreadHook(void);
    virtual cString Active(void);
    virtual time_t WakeupTime(void);
    virtual const char *MainMenuEntry(void);
    virtual cOsdObject *MainMenuAction(void);
    virtual cMenuSetupPage *SetupMenu(void);
    virtual bool SetupParse(const char *Name, const char *Value);
    virtual bool Service(const char *Id, void *Data = NULL);
    virtual const char **SVDRPHelpPages(void);
    virtual cString SVDRPCommand(const char *Command, const char *Option, int &ReplyCode);

    void SetDefaultKeymaps();
    void StartPlayer(const cCECMenu &menuitem);
    void PushCmd(const cCECCmd &cmd) {mCECRemote->PushCmd(cmd);}
    void PushCmdQueue(const cCmdQueue &cmdList) {mCECRemote->PushCmdQueue(cmdList);}
    cCECMenuList *GetMenuList() {return &mConfigFileParser.mMenuList;}
    bool GetStartManually() {return mStartManually;}
};

} // namespace cecplugin

#endif
