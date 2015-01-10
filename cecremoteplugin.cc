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
        mCfgDir("cecremote"), mCfgFile("cecremote.conf"), mCECRemote(NULL)
{
    mCECLogLevel = CEC_LOG_ERROR | CEC_LOG_WARNING | CEC_LOG_DEBUG;
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

// Read global options for the plugin
bool cPluginCecremote::AddGlobalOptions (const string &sectionname)
{
    stringList vals;
    stringList::iterator it;
    string cecdebug;

    if (sectionname != "GLOBAL") {
        return false;
    }

    if (mConfigFileParser.GetSingleValue(sectionname, "CEC_DEBUG", cecdebug)) {
        Isyslog("CEC_Debug %s", cecdebug.c_str());
        mCECLogLevel = atoi(cecdebug.c_str());
        if ((mCECLogLevel < 0) || (mCECLogLevel > CEC_LOG_ALL)) {
            Esyslog("CEC_Debug out of range");
        }
    }

    return true;
}

bool cPluginCecremote::AddMenu(const string &sectionname)
{
    string menuname;
    string stillpic;
    stringList vals;
    string dev = "DEVICE#";
    char num;
    bool makeactive = false;
    bool poweron = false;
    bool poweroff = false;

    if (sectionname.compare(0,dev.length(), dev) != 0) {
        return false;
    }
    if (sectionname.length() > dev.length()+1) {
        Esyslog("Invalid device section name %s", sectionname.c_str());
        return false;
    }
    num = sectionname[dev.length()];
    if (!mConfigFileParser.GetSingleValue(sectionname, "name", menuname)) {
        Esyslog("Missing name in device section %s", sectionname.c_str());
        return false;
    }
    Dsyslog("Menu#%c: %s", num, menuname.c_str());
    if (!mConfigFileParser.GetSingleValue(sectionname, "stillpic", stillpic)) {
        Esyslog("Missing stillpic in device section %s", sectionname.c_str());
        return false;
    }

    if (mConfigFileParser.GetValues(sectionname, "onstart", vals)) {
        stringList::iterator it;
        for (it = vals.begin(); it != vals.end(); it++) {
            string s = StringTools::ToUpper(*it);
            Dsyslog("+%s+ ", s.c_str());
            if (s == "POWERON") {
                poweron = true;
            }
            else {
                Esyslog("%s not allowed for onstart in section %s",
                        s.c_str(), sectionname.c_str());
            }
        }
    }
    if (mConfigFileParser.GetValues(sectionname, "onstop", vals)) {
        stringList::iterator it;
        for (it = vals.begin(); it != vals.end(); it++) {
            string s = StringTools::ToUpper(*it);
            Dsyslog("-%s- ", s.c_str());
            if (s == "MAKEACTIVE") {
                makeactive = true;
            }
            else if (s == "POWEROFF") {
                poweroff = true;
            }
            else {
                Esyslog("%s not allowed for onstop in section %s",
                        s.c_str(), sectionname.c_str());
            }
        }
    }

    cCECDevInfo info((int)num-'0', menuname, stillpic, makeactive, poweron, poweroff);
    mCECDevMenuInfo.push_back(info);
    return true;
}

bool cPluginCecremote::Start(void)
{
    Section::iterator iter;
    string sectionname;

    string file = GetConfigFile();
    if (!mConfigFileParser.Parse(file)) {
        Esyslog("Config file %s not found", file.c_str());
        return false;
    }

    if (!mConfigFileParser.GetFirstSection(iter, sectionname)) {
        Esyslog("Empty config file %s", file.c_str());
        return false;
    }
    Dsyslog("First Section:%s ", sectionname.c_str());
    if (!AddGlobalOptions(sectionname)) {
        if (!AddMenu(sectionname)) {
            return false;
        }
    }

    while (mConfigFileParser.GetNextSection(iter, sectionname)) {
        Dsyslog("Section:%s ", sectionname.c_str());
        if (!AddGlobalOptions(sectionname)) {
            if (!AddMenu(sectionname)) {
                return false;
            }
        }
    }
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
