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

static const char *VERSION        = "0.0.1";
static const char *DESCRIPTION    = "Send/Receive CEC commands";
static const char *MAINMENUENTRY  = "CECremote";

using namespace std;

string cPluginCecremote::mCfgDir = "cecremote";
string cPluginCecremote::mCfgFile = "cecremote.conf";

cPluginCecremote::cPluginCecremote(void)
{
    // Initialize any member variables here.
    // DON'T DO ANYTHING ELSE THAT MAY HAVE SIDE EFFECTS, REQUIRE GLOBAL
    // VDR OBJECTS TO EXIST OR PRODUCE ANY OUTPUT!
}

cPluginCecremote::~cPluginCecremote()
{
    // Clean up after yourself!
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
            esyslog("CECRemotePlugin unknown option %c", c);
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
    new cCECRemote();
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

cOsdObject *cPluginCecremote::MainMenuAction(void)
{
    // Perform the action when selected from the main VDR menu.
    return NULL;
}

cMenuSetupPage *cPluginCecremote::SetupMenu(void)
{
    // Return a setup menu in case the plugin supports one.
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
