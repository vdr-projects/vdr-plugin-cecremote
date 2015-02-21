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
#include "cecconfigfileparser.h"

class cCECOsd;

class cPluginCecremote : public cPlugin {

protected:
    int mCECLogLevel;

    std::string mCfgDir;
    std::string mCfgFile;

    cCECConfigFileParser mConfigFileParser;
    cCECRemote *mCECRemote;

    const std::string GetConfigDir(void) {
        const std::string cfdir = ConfigDirectory();
        return cfdir + "/" + mCfgDir + "/";
    }
    const std::string GetConfigFile(void) {
        const std::string cf = GetConfigDir() + mCfgFile;
        return cf;
    }
    void ExecToggle(const cCECMenu menu) {
        mCECRemote->ExecToggle(menu.mAddress, menu.onPowerOn, menu.onPowerOff);
    }

public:
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

    void StartPlayer(const cCECMenu &menuitem);
    void PushCmd(const cCECCmd &cmd) {mCECRemote->PushCmd(cmd);}
    void ExecCmd(const cCmdQueue &cmdList) {mCECRemote->ExecCmd(cmdList);}
    cCECMenuList *GetMenuList() {return &mConfigFileParser.mMenuList; }
    cCECkeymaps mKeyMaps;
};

#endif
