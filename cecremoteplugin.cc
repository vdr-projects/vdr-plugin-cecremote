/*
 * cecremote.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */


#include <getopt.h>
#include <stdlib.h>

#include "cecremoteplugin.h"
#include "ceclog.h"
#include "cecosd.h"
#include "stringtools.h"

static const char *VERSION        = "0.0.1";
static const char *DESCRIPTION    = "Send/Receive CEC commands";
static const char *MAINMENUENTRY  = "CECremote";

using namespace std;

cPluginCecremote::cPluginCecremote(void) :
        mCfgDir("cecremote"), mCfgFile("cecremote.xml")
{
    mCECLogLevel = CEC_LOG_ERROR | CEC_LOG_WARNING | CEC_LOG_DEBUG;
    mCECRemote = NULL;
}

cPluginCecremote::~cPluginCecremote()
{
    if (mCECRemote != NULL) {
        delete mCECRemote;
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
    if (cCECConfigMenu::GetShowMainMenu()) {
        return tr(MAINMENUENTRY);
    }
    return NULL;
}

const char *cPluginCecremote::CommandLineHelp(void)
{
    return "-c  --configdir <dir>     Directory for config files : cecremote\n";
}

bool cPluginCecremote::ProcessArgs(int argc, char *argv[])
{
    static struct option long_options[] =
    {
            { "configdir",      required_argument, NULL, 'c' },
            { NULL }
    };
    int c, option_index = 0;

    while ((c = getopt_long(argc, argv, "c:",
            long_options, &option_index)) != -1) {
        switch (c) {
        case 'c':
            mCfgDir.assign(optarg);
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
    // Initialize any background activities the plugin shall perform.
    return true;
}


bool cPluginCecremote::Start(void)
{
    string file = GetConfigFile();
    if (!mConfigFileParser.Parse(file)) {
        Esyslog("Config file %s not found", file.c_str());
        return false;
    }
/* TODO */
    mCECRemote = new cCECRemote(mCECLogLevel);
    return true;
}

void cPluginCecremote::Stop(void)
{
    // Stop any background activities the plugin is performing.
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

void cPluginCecremote::StartPlayer(int cnt) {
    cnt -= 1;
    Dsyslog("StartPlayer %d", cnt);
    Isyslog("starting player: %s", mCECDevMenuInfo.at(cnt).mMenuName.c_str());

    cControl::Launch(new cCECControl(mCECDevMenuInfo.at(cnt), this));
    cControl::Attach();
}

cOsdObject *cPluginCecremote::MainMenuAction(void)
{
    int count = mCECDevMenuInfo.size();
       if (count == 0) {
           return NULL;
       }
       else if (count == 1) {
           StartPlayer(0);
           return NULL;
       }
    return new cCECOsd(this);
}

cMenuSetupPage *cPluginCecremote::SetupMenu(void)
{
    return NULL;
}

bool cPluginCecremote::SetupParse(const char *Name, const char *Value)
{
    // Parse your own setup parameters and store their values.
    return cCECConfigMenu::SetupParse(Name, Value);
}

bool cPluginCecremote::Service(const char *Id, void *Data)
{
    // Handle custom service requests from other plugins
    return false;
}

const char **cPluginCecremote::SVDRPHelpPages(void)
{
    // Return help text for SVDRP commands this plugin implements
    return NULL;
}

cString cPluginCecremote::SVDRPCommand(const char *Command, const char *Option, int &ReplyCode)
{
    // Process SVDRP commands this plugin implements
    return NULL;
}

VDRPLUGINCREATOR(cPluginCecremote); // Don't touch this!
