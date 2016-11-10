/*
 * CECRemote PlugIn for VDR
 *
 * Copyright (C) 2015-2016 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 * This class implements the main VDR plugin code.
 */


#include <getopt.h>
#include <stdlib.h>

#include "cecremoteplugin.h"
#include "ceclog.h"
#include "cecosd.h"
#include "stringtools.h"
#include "keymaps.h"
#include "configmenu.h"
#include "rtcwakeup.h"

namespace cecplugin {

static const char *VERSION        = "1.4.1";
static const char *DESCRIPTION    = "Send/Receive CEC commands";
static const char *MAINMENUENTRY  = "CECremote";

using namespace std;

cPluginCecremote::cPluginCecremote(void) :
        mCfgDir("cecremote"), mCfgFile("cecremote.xml"), mStatusMonitor(NULL),
        mStartManually(true)
{
    mCECLogLevel = CEC_LOG_ERROR | CEC_LOG_WARNING | CEC_LOG_DEBUG;
    mCECRemote = NULL;
}

cPluginCecremote::~cPluginCecremote()
{
    if (mCECRemote != NULL) {
        delete mCECRemote;
        mCECRemote = NULL;
    }
    if (mStatusMonitor != NULL) {
        delete mStatusMonitor;
        mStatusMonitor = NULL;
    }
}

const char *cPluginCecremote::Version(void)
{
    return VERSION;
}

const char *cPluginCecremote::Description(void)
{
    return DESCRIPTION;
}

const char *cPluginCecremote::MainMenuEntry(void)
{
    if (cConfigMenu::GetShowMainMenu()) {
        return tr(MAINMENUENTRY);
    }
    return NULL;
}

const char *cPluginCecremote::CommandLineHelp(void)
{
    return "-c  --configdir <dir>     Directory for config files : cecremote\n"
           "-x  --configfile <file>   Config file : cecremote.xml";
}

bool cPluginCecremote::ProcessArgs(int argc, char *argv[])
{
    static struct option long_options[] =
    {
            { "configdir",      required_argument, NULL, 'c' },
            { "configfile",     required_argument, NULL, 'x' },
            { NULL }
    };
    int c, option_index = 0;

    while ((c = getopt_long(argc, argv, "c:x:",
            long_options, &option_index)) != -1) {
        switch (c) {
        case 'c':
            mCfgDir.assign(optarg);
            break;
        case 'x':
            mCfgFile.assign(optarg);
            break;
        default:
            Esyslog("CECRemotePlugin unknown option %c", c);
            return false;
        }
    }

    return true;
}

bool cPluginCecremote::Initialize(void)
{
    string file = GetConfigFile();
    rtcwakeup::RTC_WAKEUP_TYPE rtcwakeup = rtcwakeup::RTC_ERROR;

    if (!mConfigFileParser.Parse(file, mKeyMaps)) {
        Esyslog("Error on parsing config file file %s", file.c_str());
        return false;
    }
    mCECLogLevel = mConfigFileParser.mGlobalOptions.cec_debug;
    if (mConfigFileParser.mGlobalOptions.mRTCDetect) {
        Dsyslog("Use RTC wakeup detection");
        rtcwakeup = rtcwakeup::check();
        mStartManually = (rtcwakeup != rtcwakeup::RTC_WAKEUP);
    }
    // Either rtc wakeup is disabled or not available, so fall back
    // to "old" manual start detection
    if (rtcwakeup == rtcwakeup::RTC_ERROR) {
        Dsyslog("Use VDR wakeup detection: Next Wakeup %d",
                Setup.NextWakeupTime);
        if (Setup.NextWakeupTime > 0) {
            // 600 comes from vdr's MANUALSTART constant in vdr.c
            if (abs(Setup.NextWakeupTime - time(NULL)) < 600) {
                mStartManually = false;
            }
        }
    }
    if (mStartManually) {
        Dsyslog("manual start");
    }
    else {
        Dsyslog("timed start");
    }
    mCECRemote = new cCECRemote(mConfigFileParser.mGlobalOptions, this);

    SetDefaultKeymaps();
    return true;
}

bool cPluginCecremote::Start(void)
{
    mCECRemote->Startup();
    mStatusMonitor = new cStatusMonitor(this);
    return true;
}

void cPluginCecremote::Stop(void)
{
    Dsyslog("Stop Plugin");
    delete mStatusMonitor;
    mStatusMonitor = NULL;
    mCECRemote->Stop();
    delete mCECRemote;
    mCECRemote = NULL;
}

void cPluginCecremote::Housekeeping(void)
{
    // Perform any cleanup or other regular tasks.
}

void cPluginCecremote::MainThreadHook(void)
{
    // Perform actions in the context of the main program thread.
    // WARNING: Use with great care - see PLUGINS.html!
}

cString cPluginCecremote::Active(void)
{
    // Return a message string if shutdown should be postponed
    return NULL;
}

time_t cPluginCecremote::WakeupTime(void)
{
    // Return custom wakeup time for shutdown script
    return 0;
}

void cPluginCecremote::StartPlayer(const cCECMenu &menuitem)
{
    // If no <stillpic> is used, execute only onStart section
    if (menuitem.mStillPic.empty()) {
        Isyslog("Executing: %s", menuitem.mMenuTitle.c_str());
        if (menuitem.isMenuPowerToggle()) {
            ExecToggle(menuitem);
        }
        else {
            PushCmdQueue(menuitem.mOnStart);
        }
    }
    // otherwise start a new player
    else {
        Isyslog("starting player: %s", menuitem.mMenuTitle.c_str());
        cControl::Launch(new cCECControl(menuitem, this));
        cControl::Attach();
    }
}

cOsdObject *cPluginCecremote::MainMenuAction(void)
{
    if (cCECOsd::mMenuItems.size() == 1) {
        StartPlayer(cCECOsd::mMenuItems[0]);
        return NULL;
    }
    return new cCECOsd(this);
}

cMenuSetupPage *cPluginCecremote::SetupMenu(void)
{
    return new cConfigMenu();
}

bool cPluginCecremote::SetupParse(const char *Name, const char *Value)
{
    // Parse your own setup parameters and store their values.
    return cConfigMenu::SetupParse(Name, Value);
}

bool cPluginCecremote::Service(const char *Id, void *Data)
{
    // Handle custom service requests from other plugins
    return false;
}

const char **cPluginCecremote::SVDRPHelpPages(void)
{
    static const char *HelpPages[] = {
            "LSTK\nList known CEC keycodes\n",
            "LSTD\nList CEC devices\n",
            "KEYM\nList available key map\n",
            "VDRK [id]\nDisplay VDR->CEC key map with id\n",
            "CECK [id]\nDisplay CEC->VDR key map with id\n",
            "DISC\nDisconnect CEC",
            "CONN\nConnect CEC",
            NULL
    };
    return HelpPages;
}

cString cPluginCecremote::SVDRPCommand(const char *Command, const char *Option, int &ReplyCode)
{
    ReplyCode = 214;
    if (strcasecmp(Command, "LSTD") == 0) {
        return mCECRemote->ListDevices();
    }
    else if (strcasecmp(Command, "KEYM") == 0) {
        return mKeyMaps.ListKeymaps();
    }
    else if (strcasecmp(Command, "LSTK") == 0) {
        return mKeyMaps.ListKeycodes();
    }
    else if (strcasecmp(Command, "VDRK") == 0) {
        if (Option == NULL) {
            ReplyCode = 901;
            return "Error: Keymap ID required";
        }
        string s = Option;
        return mKeyMaps.ListVDRKeyMap(s);
    }
    else if (strcasecmp(Command, "CECK") == 0) {
        if (Option == NULL) {
            ReplyCode = 901;
            return "Error: Keymap ID required";
        }
        string s = Option;
        return mKeyMaps.ListCECKeyMap(s);
    }
    else if (strcasecmp(Command, "DISC") == 0) {
        cCmd cmd(CEC_DISCONNECT);
        mCECRemote->PushWaitCmd(cmd);
        return "Disconnected";
    }
    else if (strcasecmp(Command, "CONN") == 0) {
        cCmd cmd(CEC_CONNECT);
        mCECRemote->PushWaitCmd(cmd);
        return "Connected";
    }

    ReplyCode = 901;
    return "Error: Unexpected option";
}

/*
 * Set the default keymaps to use.
 */
void cPluginCecremote::SetDefaultKeymaps()
{
    mKeyMaps.SetActiveKeymaps(mConfigFileParser.mGlobalOptions.mVDRKeymap,
                              mConfigFileParser.mGlobalOptions.mCECKeymap);
}

} // namespace cecplugin

VDRPLUGINCREATOR(cecplugin::cPluginCecremote); // Don't touch this!

