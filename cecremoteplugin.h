/*
 * cecremote.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */
#ifndef CECREMOTEPLUGIN_H
#define CECREMOTEPLUGIN_H

#include <string>

#include <vdr/plugin.h>
#include "cecremote.h"
#include "cecconfigmenu.h"
#include "cecconfigfileparser.h"

class cCECOsd;

class cCECDevInfo {
public:
    cCECDevInfo() : mAddr(0), mMenuName("") {};
    cCECDevInfo(int addr, const std::string &menuname) {
        mAddr = addr;
        mMenuName = menuname;
        mStillPic= "/video/conf/plugins/cecremote/blueray.mpg"; // @TODO
    }

    int mAddr;
    std::string mMenuName;
    std::string mStillPic;
};

typedef std::vector<cCECDevInfo> cCECDevInfoList;
typedef cCECDevInfoList::const_iterator cCECDevInfoListIterator;

class cPluginCecremote : public cPlugin {

protected:
    int mCECLogLevel;

    std::string mCfgDir;
    std::string mCfgFile;
    cCECConfigFileParser mConfigFileParser;
    cCECDevInfoList mCECDevMenuInfo;
    cCECRemote *mCECRemote;

    const std::string GetConfigDir(void) {
        const std::string cfdir = ConfigDirectory();
        return cfdir + "/" + mCfgDir + "/";
    }
    const std::string GetConfigFile(void) {
        const std::string cf = GetConfigDir() + mCfgFile;
        return cf;
    }
    bool AddGlobalOptions (const std::string &sectionname);
    bool AddMenu (const std::string &sectionname);

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

    void StartPlayer(int cnt);
    void PushCmd(const cCECCmd &cmd) {mCECRemote->PushCmd(cmd);}
    cCECDevInfoList *GetDevInfoList() {return &mCECDevMenuInfo; }
};

#endif
