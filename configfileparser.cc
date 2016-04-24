/*
 * CECRemote PlugIn for VDR
 *
 * Copyright (C) 2015-2016 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 * configfileparser.cc: Class for parsing the plugin configuration file.
 */

#include <vdr/plugin.h>
#include <stdio.h>
#include <stdexcept>
#include "ceclog.h"
#include "configfileparser.h"
#include "stringtools.h"
#include "opcodemap.h"

using namespace std;
using namespace pugi;

namespace cecplugin {
// Keywords used in the XML config file
const char *cConfigFileParser::XML_ONSTART = "onstart";
const char *cConfigFileParser::XML_ONSTOP  = "onstop";
const char *cConfigFileParser::XML_ONPOWERON = "onpoweron";
const char *cConfigFileParser::XML_ONPOWEROFF = "onpoweroff";
const char *cConfigFileParser::XML_GLOBAL = "global";
const char *cConfigFileParser::XML_MENU = "menu";
const char *cConfigFileParser::XML_CECKEYMAP = "ceckeymap";
const char *cConfigFileParser::XML_VDRKEYMAP = "vdrkeymap";
const char *cConfigFileParser::XML_ID = "id";
const char *cConfigFileParser::XML_KEY = "key";
const char *cConfigFileParser::XML_CODE = "code";
const char *cConfigFileParser::XML_VALUE = "value";
const char *cConfigFileParser::XML_STOP = "stop";
const char *cConfigFileParser::XML_KEYMAPS = "keymaps";
const char *cConfigFileParser::XML_FILE = "file";
const char *cConfigFileParser::XML_CEC = "cec";
const char *cConfigFileParser::XML_VDR = "vdr";
const char *cConfigFileParser::XML_POWERON = "poweron";
const char *cConfigFileParser::XML_POWEROFF = "poweroff";
const char *cConfigFileParser::XML_MAKEACTIVE = "makeactive";
const char *cConfigFileParser::XML_MAKEINACTIVE = "makeinactive";
const char *cConfigFileParser::XML_EXEC = "exec";
const char *cConfigFileParser::XML_TEXTVIEWON = "textviewon";
const char *cConfigFileParser::XML_COMBOKEYTIMEOUTMS = "combokeytimeoutms";
const char *cConfigFileParser::XML_CECDEBUG = "cecdebug";
const char *cConfigFileParser::XML_CECDEVICETYPE = "cecdevicetype";
const char *cConfigFileParser::XML_DEVICE = "device";
const char *cConfigFileParser::XML_PHYSICAL = "physical";
const char *cConfigFileParser::XML_LOGICAL = "logical";
const char *cConfigFileParser::XML_ONMANUALSTART = "onmanualstart";
const char *cConfigFileParser::XML_ONSWITCHTOTV = "onswitchtotv";
const char *cConfigFileParser::XML_ONSWITCHTORADIO = "onswitchtoradio";
const char *cConfigFileParser::XML_ONSWITCHTOREPLAY = "onswitchtoreplay";
const char *cConfigFileParser::XML_ONACTIVESOURCE = "onactivesource";
const char *cConfigFileParser::XML_HDMIPORT = "hdmiport";
const char *cConfigFileParser::XML_BASEDEVICE = "basedevice";
const char *cConfigFileParser::XML_SHUTDOWNONSTANDBY = "shutdownonstandby";
const char *cConfigFileParser::XML_POWEROFFONSTANDBY = "poweroffonstandby";
const char *cConfigFileParser::XML_ONCECCOMMAND = "onceccommand";
const char *cConfigFileParser::XML_EXECMENU = "execmenu";
const char *cConfigFileParser::XML_STOPMENU = "stopmenu";
const char *cConfigFileParser::XML_COMMANDLIST = "commandlist";
const char *cConfigFileParser::XML_COMMAND = "command";
const char *cConfigFileParser::XML_INITIATOR = "initiator";
const char *cConfigFileParser::XML_RTCDETECT = "rtcdetect";

/*
 * Parse <onceccommand>
 */
void cConfigFileParser::parseOnCecCommand(const xml_node node) {
    cCECCommandHandler h;

    string command = node.attribute(XML_COMMAND).as_string("");
    if (command.empty()) {
        string s = "Missing command";
        Esyslog(s.c_str());
        throw cCECConfigException(getLineNumber(node.offset_debug()), s);
    }

    if (!textToInt(command, h.mCecOpCode)) {
        if (!opcodeMap::getOpcode(command, h.mCecOpCode)) {
            string s = "CEC Command not an integer";
            Esyslog(s.c_str());
            throw cCECConfigException(getLineNumber(node.offset_debug()), s);
        }
    }

    const char *device = node.attribute(XML_INITIATOR).as_string("");
    getDevice(device, h.mDevice, getLineNumber(node.offset_debug()));
    Dsyslog("Handle Command %d Device %d %d\n", h.mCecOpCode,
            h.mDevice.mLogicalAddressDefined, h.mDevice.mLogicalAddressUsed);

    for (xml_node currentNode = node.first_child(); currentNode; currentNode =
            currentNode.next_sibling()) {

        if (currentNode.type() == node_element)  // is element
                {
            Dsyslog("   %s %s\n", currentNode.name(),
                    currentNode.text().as_string());

            if (strcasecmp(currentNode.name(), XML_COMMANDLIST) == 0) {
                parseList(currentNode, h.mCommands);
            } else if (strcasecmp(currentNode.name(), XML_EXECMENU) == 0) {
                h.mExecMenu = currentNode.text().as_string("");
            } else if (strcasecmp(currentNode.name(), XML_STOPMENU) == 0) {
                h.mStopMenu = currentNode.text().as_string("");
            } else {
                string s = "Invalid command ";
                s += currentNode.name();
                throw cCECConfigException(
                        getLineNumber(currentNode.offset_debug()), s);
            }
        }
    }

    mGlobalOptions.mCECCommandHandlers.insert(
            std::pair<cec_opcode, cCECCommandHandler>(h.mCecOpCode, h));
}

/*
 * Parse <player file="">
 */
void cConfigFileParser::parsePlayer(const xml_node node, cCECMenu &menu)
{
    menu.mStillPic = node.attribute(XML_FILE).as_string("");
    if (menu.mStillPic.empty()) {
        string s = "Missing file name";
        Esyslog(s.c_str());
        throw cCECConfigException(getLineNumber(node.offset_debug()), s);
    }
    Dsyslog("         Player StillPic = %s\n", menu.mStillPic.c_str());

    for (xml_node currentNode = node.first_child(); currentNode;
            currentNode = currentNode.next_sibling()) {

        if (currentNode.type() == node_element)  // is element
        {
            Dsyslog("          %s %s\n", currentNode.name(), currentNode.text().as_string());

            if (hasElements(currentNode)) {
                string s = "Too much arguments for ";
                s += currentNode.name();
                throw cCECConfigException(getLineNumber(currentNode.offset_debug()), s);
            }
            if (strcasecmp(currentNode.name(), XML_STOP) == 0) {
                eKeys k = cKey::FromString(currentNode.text().as_string());
                if (k == kNone) {
                    string s = "Invalid key ";
                    s += currentNode.text().as_string();
                    throw cCECConfigException(getLineNumber(currentNode.offset_debug()), s);
                }
                menu.mStopKeys.insert(k);
            }
            else if (strcasecmp(currentNode.name(), XML_KEYMAPS) == 0) {
                menu.mVDRKeymap = currentNode.attribute(XML_VDR).
                                        as_string(cKeyMaps::DEFAULTKEYMAP);
                menu.mCECKeymap = currentNode.attribute(XML_CEC).
                                                        as_string(cKeyMaps::DEFAULTKEYMAP);
                Dsyslog("              Keymap VDR %s CEC %s",
                        menu.mVDRKeymap.c_str(), menu.mCECKeymap.c_str());
            }
            else {
                string s = "Invalid command ";
                s += currentNode.name();
                throw cCECConfigException(getLineNumber(currentNode.offset_debug()), s);
            }
        }
    }
    if (menu.mStopKeys.size() < 1) {
        string s = "<player> requires at least one <stop>";
        throw cCECConfigException(getLineNumber(node.offset_debug()), s);
    }
}

/*
 * Check if a tag contains child elements.
 */
bool cConfigFileParser::hasElements(const xml_node node)
{
    for (xml_node currentNode = node.first_child(); currentNode;
         currentNode = currentNode.next_sibling()) {

        if (currentNode.type() == node_element)  // is element
        {
            return true;
        }
    }
    return false;
}

// Convert text to bool, returns false if conversion fails.
bool cConfigFileParser::textToBool(const char *text, bool &val)
{
    if (strcasecmp(text, "true") == 0) {
        val = true;
    }
    else if (strcasecmp(text, "false") == 0) {
        val = false;
    }
    else {
        return false;
    }
    return true;
}

/*
 * Helper function to get device address
 */
void cConfigFileParser::getDevice(const char *text, cCECDevice &device,
                                     ptrdiff_t linenumber)
{
    int val;
    // string starts with a digit, so interpret as logical address
    if (isdigit(text[0])) {
        if (!textToInt(text, val)) {
            string s = "Invalid device specification, not a logical address";
            throw cCECConfigException(linenumber, s);
        }
        if ((val <= CECDEVICE_UNKNOWN) || (val > CECDEVICE_BROADCAST)) {
            string s = "Logical address out of range";
            throw cCECConfigException(linenumber, s);
        }
        device.mPhysicalAddress = 0;
        device.mLogicalAddressDefined = (cec_logical_address)val;
    }
    else {
        try {
            device = mDeviceMap.at(text);
        }
        catch (const std::out_of_range& oor) {
            string s = "Device ";
            s += text;
            s += " not found";
            throw cCECConfigException(linenumber, s);
        }
    }
}
/*
 * parse <onstart> and <onstop>
 */
void cConfigFileParser::parseList(const xml_node node,
                                     cCmdQueue &cmdlist)
{
    cCmd cmd;

    for (xml_node currentNode = node.first_child(); currentNode;
            currentNode = currentNode.next_sibling()) {

        if (currentNode.type() == node_element)  // is element
        {
            Dsyslog("     %s %s\n", node.name(), currentNode.name());
            if (hasElements(currentNode)) {
                string s = "Too much arguments for ";
                s += currentNode.name();
                throw cCECConfigException(getLineNumber(currentNode.offset_debug()), s);
            }
            if (strcasecmp(currentNode.name(), XML_POWERON) == 0) {
                cmd.mCmd = CEC_POWERON;
                getDevice(currentNode.text().as_string(""), cmd.mDevice,
                        getLineNumber(currentNode.offset_debug()));
                cmd.mExec = "";
                Dsyslog("         POWERON %s\n", currentNode.text().as_string(""));
                cmdlist.push_back(cmd);
            }
            else if (strcasecmp(currentNode.name(), XML_POWEROFF) == 0) {
                cmd.mCmd = CEC_POWEROFF;
                getDevice(currentNode.text().as_string(""), cmd.mDevice,
                          getLineNumber(currentNode.offset_debug()));
                cmd.mExec = "";
                Dsyslog("         POWEROFF %s\n", currentNode.text().as_string(""));
                cmdlist.push_back(cmd);
            } else if (strcasecmp(currentNode.name(), XML_MAKEACTIVE) == 0) {
                cmd.mCmd = CEC_MAKEACTIVE;
                cmd.mExec = "";
                Dsyslog("         MAKEACTIVE\n");
                cmdlist.push_back(cmd);
            } else if (strcasecmp(currentNode.name(),XML_MAKEINACTIVE) == 0) {
                cmd.mCmd = CEC_MAKEINACTIVE;
                cmd.mExec = "";
                Dsyslog("         MAKEINACTIVE\n");
                cmdlist.push_back(cmd);
            } else if (strcasecmp(currentNode.name(),XML_TEXTVIEWON) == 0) {
                cmd.mCmd = CEC_TEXTVIEWON;
                getDevice(currentNode.text().as_string(""), cmd.mDevice,
                         getLineNumber(currentNode.offset_debug()));
                cmd.mExec = "";
                Dsyslog("         CEC_TEXTVIEWON %s\n", currentNode.text().as_string(""));
                cmdlist.push_back(cmd);
            } else if (strcasecmp(currentNode.name(), XML_EXEC) == 0) {
                cmd.mCmd = CEC_EXECSHELL;
                cmd.mExec = currentNode.text().as_string();
                Dsyslog("         EXEC %s\n", cmd.mExec.c_str());
                cmdlist.push_back(cmd);
            }
            else {
                string s = "Invalid command ";
                s += currentNode.name();
                throw cCECConfigException(0, s);
            }
        }
    }
}

/*
 * parse elements between <menu name="" address="">
 */
void cConfigFileParser::parseMenu(const xml_node node)
{
    cCECMenu menu;

    menu.mMenuTitle = node.attribute("name").as_string("");
    if (menu.mMenuTitle.empty()) {
        string s = "Missing menu name";
        Esyslog(s.c_str());
        throw cCECConfigException(getLineNumber(node.offset_debug()), s);
    }

    getDevice(node.attribute("address").as_string(""), menu.mDevice,
            getLineNumber(node.offset_debug()));
    Dsyslog ("  Menu %s (%s)\n", menu.mMenuTitle.c_str(),
            node.attribute("address").as_string(""));

    for (xml_node currentNode = node.first_child(); currentNode;
         currentNode = currentNode.next_sibling()) {

        if (currentNode.type() == node_element)  // is element
        {
            if (strcasecmp(currentNode.name(), "player") == 0) {
               parsePlayer(currentNode, menu);
            }
            else if (strcasecmp(currentNode.name(), XML_ONSTART) == 0) {
                if ((menu.mPowerToggle == cCECMenu::UNDEFINED) ||
                    (menu.mPowerToggle == cCECMenu::USE_ONSTART))
                {
                    menu.mPowerToggle = cCECMenu::USE_ONSTART;
                    parseList(currentNode, menu.mOnStart);
                }
                else
                {
                    string s = "Either <onstart> or <onpower..> is allowed";
                    throw cCECConfigException(getLineNumber(currentNode.offset_debug()), s);
                }
            }
            else if (strcasecmp(currentNode.name(), XML_ONSTOP) == 0) {
                if ((menu.mPowerToggle == cCECMenu::UNDEFINED) ||
                   (menu.mPowerToggle == cCECMenu::USE_ONSTART))
               {
                   menu.mPowerToggle = cCECMenu::USE_ONSTART;
                   parseList(currentNode, menu.mOnStop);
               }
               else
               {
                   string s = "Either <onstop> or <onpower..> is allowed";
                   throw cCECConfigException(getLineNumber(currentNode.offset_debug()), s);
               }
            }
            else if (strcasecmp(currentNode.name(), XML_ONPOWERON) == 0) {
                if ((menu.mPowerToggle == cCECMenu::UNDEFINED) ||
                    (menu.mPowerToggle == cCECMenu::USE_ONPOWER))
                {
                    menu.mPowerToggle = cCECMenu::USE_ONPOWER;
                    parseList(currentNode, menu.mOnPowerOn);
                }
                else
                {
                    string s = "Either <onstart>/<onstop> or <onpoweron> is allowed";
                    throw cCECConfigException(getLineNumber(currentNode.offset_debug()), s);
                }
            }
            else if (strcasecmp(currentNode.name(), XML_ONPOWEROFF) == 0) {
                if ((menu.mPowerToggle == cCECMenu::UNDEFINED) ||
                   (menu.mPowerToggle == cCECMenu::USE_ONPOWER))
               {
                   menu.mPowerToggle = cCECMenu::USE_ONPOWER;
                   parseList(currentNode, menu.mOnPowerOff);
               }
               else
               {
                   string s = "Either <onstart>/<onstop> or <onpoweroff> is allowed";
                   throw cCECConfigException(getLineNumber(currentNode.offset_debug()), s);
               }
            }
            else {
                string s = "Invalid Command ";
                s += currentNode.name();
                throw cCECConfigException(getLineNumber(currentNode.offset_debug()), s);
            }
        }
    }
    if (menu.mPowerToggle == cCECMenu::UNDEFINED) {
        string s = "At least one of the following tags are needed: <onstart> <onstop> <onpoweron> <onpoweroff>";
        throw cCECConfigException(getLineNumber(node.offset_debug()), s);
    }
    if ((menu.mPowerToggle == cCECMenu::USE_ONPOWER) &&
        (!menu.mStillPic.empty()))
    {
        string s = "<StillPic> not allowed for <onpoweron> or <onpoweroff>";
        throw cCECConfigException(getLineNumber(node.offset_debug()), s);
    }
    mMenuList.push_back(menu);
}

/*
 * Convert device type string to cec_device_type
 */
cec_device_type cConfigFileParser::getDeviceType(const string &s)
{
    if (strcasecmp(s.c_str(), "TV") == 0) {
        return CEC_DEVICE_TYPE_TV;
    }
    else if (strcasecmp(s.c_str(), "RECORDING_DEVICE") == 0) {
        return CEC_DEVICE_TYPE_RECORDING_DEVICE;
    }
    else if (strcasecmp(s.c_str(), "TUNER") == 0) {
        return CEC_DEVICE_TYPE_TUNER ;
    }
    else if (strcasecmp(s.c_str(), "PLAYBACK_DEVICE") == 0) {
        return CEC_DEVICE_TYPE_PLAYBACK_DEVICE ;
    }
    else if (strcasecmp(s.c_str(), "AUDIO_SYSTEM") == 0) {
        return CEC_DEVICE_TYPE_AUDIO_SYSTEM ;
    }
    return CEC_DEVICE_TYPE_RESERVED;
}
/*
 *  Parse elements between <global> nodes.
 */
void cConfigFileParser::parseGlobal(const pugi::xml_node node)
{
    for (xml_node currentNode = node.first_child(); currentNode;
         currentNode = currentNode.next_sibling()) {

        if (currentNode.type() == node_element)  // is element
        {
            Dsyslog("   Global Option %s\n", currentNode.name());
            // <cecdebug>
            if (strcasecmp(currentNode.name(), XML_CECDEBUG) == 0) {
                if (!currentNode.first_child()) {
                    string s = "No nodes allowed for cecdebug: ";
                    s += currentNode.first_child().name();
                    throw cCECConfigException(getLineNumber(currentNode.offset_debug()), s);
                }
                if (!textToInt(currentNode.text().as_string("7"),
                               mGlobalOptions.cec_debug)) {
                    string s = "Invalid numeric in cecdebug";
                    throw cCECConfigException(getLineNumber(currentNode.offset_debug()), s);
                }
                Dsyslog("CECDebug = %d \n", mGlobalOptions.cec_debug);
            }
            // <combokeytimeoutms>
            else if (strcasecmp(currentNode.name(), XML_COMBOKEYTIMEOUTMS) == 0) {
                if (!currentNode.first_child()) {
                    string s = "No nodes allowed for cecdebug: ";
                    s += currentNode.first_child().name();
                    throw cCECConfigException(getLineNumber(currentNode.offset_debug()), s);
                }
                if (!textToInt(currentNode.text().as_string("1000"),
                               mGlobalOptions.mComboKeyTimeoutMs)) {
                    string s = "Invalid numeric in combokeytimeoutms";
                    throw cCECConfigException(getLineNumber(currentNode.offset_debug()), s);
                }
                Dsyslog("ComboKeyTimeoutMs = %d \n", mGlobalOptions.mComboKeyTimeoutMs);
            }
            // <onStart>
            else if (strcasecmp(currentNode.name(), XML_ONSTART) == 0) {
                parseList(currentNode, mGlobalOptions.mOnStart);
            }
            // <onStop>
            else if (strcasecmp(currentNode.name(), XML_ONSTOP) == 0) {
                parseList(currentNode, mGlobalOptions.mOnStop);
            }
            // <onManualStart>
            else if (strcasecmp(currentNode.name(), XML_ONMANUALSTART) == 0) {
                parseList(currentNode, mGlobalOptions.mOnManualStart);
            }
            // <onSwitchToTV>
            else if (strcasecmp(currentNode.name(), XML_ONSWITCHTOTV) == 0) {
                parseList(currentNode, mGlobalOptions.mOnSwitchToTV);
            }
            // <onSwitchToRadio>
            else if (strcasecmp(currentNode.name(), XML_ONSWITCHTORADIO) == 0) {
                parseList(currentNode, mGlobalOptions.mOnSwitchToRadio);
            }
            // <onSwitchToRadio>
            else if (strcasecmp(currentNode.name(), XML_ONSWITCHTOREPLAY)
                    == 0) {
                parseList(currentNode, mGlobalOptions.mOnSwitchToReplay);
            } else if (strcasecmp(currentNode.name(), XML_CECDEVICETYPE) == 0) {
                if (!currentNode.first_child()) {
                    string s = "No nodes allowed for cecdebug: ";
                    s += currentNode.first_child().name();
                    throw cCECConfigException(
                            getLineNumber(currentNode.offset_debug()), s);
                }
                cec_device_type t = getDeviceType(
                        currentNode.text().as_string(""));
                if (t == CEC_DEVICE_TYPE_RESERVED) {
                    string s = "Invalid device type: ";
                    s += currentNode.text().as_string("");
                    throw cCECConfigException(
                            getLineNumber(currentNode.offset_debug()), s);
                }
                mGlobalOptions.mDeviceTypes.push_back(t);
                Dsyslog("CECDevicetype = %d \n", t);
            } else if (strcasecmp(currentNode.name(), XML_KEYMAPS) == 0) {
                mGlobalOptions.mVDRKeymap =
                        currentNode.attribute(XML_VDR).as_string(
                                cKeyMaps::DEFAULTKEYMAP);
                mGlobalOptions.mCECKeymap =
                        currentNode.attribute(XML_CEC).as_string(
                                cKeyMaps::DEFAULTKEYMAP);
                Dsyslog("Keymap VDR %s CEC %s",
                        mGlobalOptions.mVDRKeymap.c_str(),
                        mGlobalOptions.mCECKeymap.c_str());
            } else if (strcasecmp(currentNode.name(), XML_HDMIPORT) == 0) {
                if (!textToInt(currentNode.text().as_string("1000"),
                        mGlobalOptions.mHDMIPort)) {
                    string s = "Invalid numeric in hdmiport";
                    throw cCECConfigException(
                            getLineNumber(currentNode.offset_debug()), s);
                }
                if ((mGlobalOptions.mHDMIPort < CEC_HDMI_PORTNUMBER_NONE)
                        || (mGlobalOptions.mHDMIPort) > CEC_MAX_HDMI_PORTNUMBER) {
                    string s = "Allowed value for hdmiport 0-15";
                    throw cCECConfigException(
                            getLineNumber(currentNode.offset_debug()), s);
                }
            } else if (strcasecmp(currentNode.name(), XML_BASEDEVICE) == 0) {
                if (!textToInt(currentNode.text().as_string("0"),
                        mGlobalOptions.mBaseDevice)) {
                    string s = "Invalid numeric in basedevice";
                    throw cCECConfigException(
                            getLineNumber(currentNode.offset_debug()), s);
                }
                if ((mGlobalOptions.mBaseDevice < CEC_HDMI_PORTNUMBER_NONE)
                        || (mGlobalOptions.mBaseDevice) > CECDEVICE_BROADCAST) {
                    string s = "Allowed value for basedevice 0-15";
                    throw cCECConfigException(
                            getLineNumber(currentNode.offset_debug()), s);
                }
            } else if (strcasecmp(currentNode.name(), XML_SHUTDOWNONSTANDBY)
                    == 0) {
                if (!textToBool(currentNode.text().as_string(""),
                        mGlobalOptions.mShutdownOnStandby)) {
                    string s = "Only true or false allowed";
                    throw cCECConfigException(
                            getLineNumber(currentNode.offset_debug()), s);
                }
            } else if (strcasecmp(currentNode.name(), XML_POWEROFFONSTANDBY)
                    == 0) {
                if (!textToBool(currentNode.text().as_string(""),
                        mGlobalOptions.mPowerOffOnStandby)) {
                    string s = "Only true or false allowed";
                    throw cCECConfigException(
                            getLineNumber(currentNode.offset_debug()), s);
                }
            } else if (strcasecmp(currentNode.name(), XML_RTCDETECT) == 0) {
                if (!textToBool(currentNode.text().as_string(""),
                        mGlobalOptions.mRTCDetect)) {
                    string s = "Only true or false allowed";
                    throw cCECConfigException(
                            getLineNumber(currentNode.offset_debug()), s);
                }
            } else {
                string s = "Invalid Node ";
                s += currentNode.name();
                throw cCECConfigException(
                        getLineNumber(currentNode.offset_debug()), s);
            }
        }
    }
}

/*
 * parse elements between <vdrkeymap>
 */
void cConfigFileParser::parseVDRKeymap(const xml_node node, cKeyMaps &keymaps)
{
    string id = node.attribute(XML_ID).as_string("");
    if (id.empty()) {
        string s = "Missing id for vdr keymap";
        Esyslog(s.c_str());
        throw cCECConfigException(getLineNumber(node.offset_debug()), s);
    }

    Dsyslog ("VDRKEYMAP %s\n", id.c_str());

    keymaps.InitVDRKeyFromDefault(id);
    for (xml_node currentNode = node.first_child(); currentNode;
         currentNode = currentNode.next_sibling()) {

        if (currentNode.type() == node_element)  // is element
        {
            if (strcasecmp(currentNode.name(), XML_KEY) != 0) {
                string s = "Invalid node ";
                s += currentNode.name();
                Esyslog(s.c_str());
                throw cCECConfigException(getLineNumber(node.offset_debug()), s);
            }
            string code = currentNode.attribute(XML_CODE).as_string("");
            if (code.empty()) {
                string s = "Missing code in vdr keymap";
                Esyslog(s.c_str());
                throw cCECConfigException(getLineNumber(currentNode.offset_debug()), s);
            }
            eKeys k = cKey::FromString(code.c_str());
            if (k == kNone) {
                string s = "Unknown VDR key code " + code;
                Esyslog(s.c_str());
                throw cCECConfigException(getLineNumber(currentNode.offset_debug()), s);
            }
            keymaps.ClearVDRKey(id, k);

            // Parse cec key values
            for (xml_node ceckeynode = currentNode.first_child(); ceckeynode;
                    ceckeynode = ceckeynode.next_sibling()) {
                if (ceckeynode.type() == node_element)  // is element
                        {
                    if (strcasecmp(ceckeynode.name(), XML_VALUE) != 0) {
                        string s = "Invalid node ";
                        s += ceckeynode.name();
                        Esyslog(s.c_str());
                        throw cCECConfigException(
                                getLineNumber(ceckeynode.offset_debug()), s);
                    }
                    string ceckey = ceckeynode.text().as_string();
                    cec_user_control_code c = keymaps.StringToCEC(ceckey);
                    if (c == CEC_USER_CONTROL_CODE_UNKNOWN) {
                        string s = "Unknown CEC key code " + ceckey;
                        Esyslog(s.c_str());
                        throw cCECConfigException(
                                getLineNumber(ceckeynode.offset_debug()), s);
                    }
                    keymaps.AddVDRKey(id, k, c);
                }
            }
        }
    }
}

/*
 * parse elements between <ceckeymap>
 */
void cConfigFileParser::parseCECKeymap(const xml_node node, cKeyMaps &keymaps)
{
    string id = node.attribute(XML_ID).as_string("");
    if (id.empty()) {
        string s = "Missing id for vdr keymap";
        Esyslog(s.c_str());
        throw cCECConfigException(getLineNumber(node.offset_debug()), s);
    }

    Dsyslog ("CECKEYMAP %s\n", id.c_str());

    keymaps.InitCECKeyFromDefault(id);
    for (xml_node currentNode = node.first_child(); currentNode;
            currentNode = currentNode.next_sibling()) {

        if (currentNode.type() == node_element)  // is element
        {
            if (strcasecmp(currentNode.name(), XML_KEY) != 0) {
                string s = "Invalid node ";
                s += currentNode.name();
                Esyslog(s.c_str());
                throw cCECConfigException(getLineNumber(node.offset_debug()), s);
            }
            string code = currentNode.attribute(XML_CODE).as_string("");
            if (code.empty()) {
                string s = "Missing code in vdr keymap";
                Esyslog(s.c_str());
                throw cCECConfigException(getLineNumber(currentNode.offset_debug()), s);
            }
            cec_user_control_code c = keymaps.StringToCEC(code);
            if (c == CEC_USER_CONTROL_CODE_UNKNOWN) {
                string s = "Unknown CEC key code " + code;
                Esyslog(s.c_str());
                throw cCECConfigException(getLineNumber(currentNode.offset_debug()), s);
            }
            keymaps.ClearCECKey(id, c);

            // Parse vdr key values
            for (xml_node vdrkeynode = currentNode.first_child(); vdrkeynode;
                    vdrkeynode = vdrkeynode.next_sibling()) {
                if (vdrkeynode.type() == node_element)  // is element
                {
                    if (strcasecmp(vdrkeynode.name(), XML_VALUE) != 0) {
                        string s = "Invalid node ";
                        s += vdrkeynode.name();
                        Esyslog(s.c_str());
                        throw cCECConfigException(getLineNumber(vdrkeynode.offset_debug()), s);
                    }
                    string vdrkey = vdrkeynode.text().as_string();
                    eKeys k = cKey::FromString(vdrkey.c_str());
                    if (k == kNone) {
                        string s = "Unknown VDR key code " + vdrkey;
                        Esyslog(s.c_str());
                        throw cCECConfigException(getLineNumber(vdrkeynode.offset_debug()), s);
                    }
                    keymaps.AddCECKey(id, c, k);
                }
            }
        }
    }
}

/*
 * parse elements between <device id="">
 */
void cConfigFileParser::parseDevice(const xml_node node)
{
    cCECDevice device;
    string id = node.attribute(XML_ID).as_string("");
    if (id.empty()) {
        string s = "Missing id for vdr keymap";
        Esyslog(s.c_str());
        throw cCECConfigException(getLineNumber(node.offset_debug()), s);
    }

    Dsyslog ("DEVICE %s\n", id.c_str());
    //device.id = id;
    for (xml_node currentNode = node.first_child(); currentNode;
         currentNode = currentNode.next_sibling()) {

        if (currentNode.type() == node_element)  // is element
        {
            if (strcasecmp(currentNode.name(), XML_PHYSICAL) == 0) {
                if (!textToInt(currentNode.text().as_string("x"),
                               device.mPhysicalAddress, 16)) {
                    string s = "Invalid physical address ";
                    s += currentNode.text().as_string();
                    Esyslog(s.c_str());
                    throw cCECConfigException(getLineNumber(node.offset_debug()), s);
                }
                if (device.mPhysicalAddress == -1) {
                    string s = "Invalid physical address ";
                    s += currentNode.text().as_string();
                    Esyslog(s.c_str());
                    throw cCECConfigException(getLineNumber(node.offset_debug()), s);
                }
                Dsyslog ("   Physical Address = %04x", device.mPhysicalAddress);
            }
            else if (strcasecmp(currentNode.name(), XML_LOGICAL) == 0) {
                if (!textToInt(currentNode.text().as_string("x"),
                               device.mLogicalAddressDefined)) {
                    string s = "Invalid logical address ";
                    s += currentNode.text().as_string();
                    Esyslog(s.c_str());
                    throw cCECConfigException(getLineNumber(node.offset_debug()), s);
                }

                if ((device.mLogicalAddressDefined < 0) &&
                        (device.mLogicalAddressDefined > CECDEVICE_BROADCAST)) {
                    string s = "Invalid logical address ";
                    s += currentNode.text().as_string();
                    Esyslog(s.c_str());
                    throw cCECConfigException(getLineNumber(node.offset_debug()), s);
                }
                Dsyslog ("   Logical Address = %d", device.mLogicalAddressDefined);
            }
            else {
                string s = "Invalid node ";
                s += currentNode.name();
                Esyslog(s.c_str());
                throw cCECConfigException(getLineNumber(node.offset_debug()), s);
            }
        }
    }
    if ((device.mPhysicalAddress == -1) &&
        (device.mLogicalAddressDefined == CECDEVICE_UNKNOWN)) {
        string s = "Nothing defined for device";
        s += id;
        Esyslog(s.c_str());
        throw cCECConfigException(getLineNumber(node.offset_debug()), s);
    }
    mDeviceMap.insert(std::pair<string, cCECDevice>(id, device));
}
/*
 * Helper function to get the line number from the byte offset in the XML
 * error.
 */
int cConfigFileParser::getLineNumber(long offset)
{
    int line = 1;
    FILE *fp = fopen (mXmlFile, "r");
    if (fp == NULL) {
        Esyslog("Can not open file %s for getting line number", mXmlFile);
        return -1;
    }
    while (!feof(fp) && (offset > 0)) {
        if (fgetc(fp) == '\n') {
            line++;
        }
        offset--;
    }
    return line;
}

/*
 * Find a menu by name
 */
bool cConfigFileParser::FindMenu(const string &menuname, cCECMenu &menu) {
    bool found = false;
    for (cCECMenuListIterator i = mMenuList.begin(); i != mMenuList.end();
            i++) {
        cCECMenu m = *i;
        if (m.mMenuTitle == menuname) {
            menu = m;
            found = true;
        }
    }
    return found;
}

/*
 * Parse the file, fill mGlobalOptions and mMenuList and return the
 * parsed keymaps.
 * Returns false when a syntax error occurred during parsing.
 */
bool cConfigFileParser::Parse(const string &filename, cKeyMaps &keymaps) {
    bool ret = true;
    xml_document xmlDoc;
    xml_node currentNode;
    mXmlFile = filename.c_str();
    xml_parse_result res = xmlDoc.load_file(mXmlFile);
    if (res.status != status_ok) {
        Esyslog("Error parsing file %s: %s\nAt line: %d",
                mXmlFile, res.description(), getLineNumber(res.offset));
        return false;
    }

    // Get the top-level element: NAme is "root". No attributes for "root"
    xml_node elementRoot = xmlDoc.document_element();
    if (elementRoot.empty()) {
        Esyslog("Document contains no data\n");
        return false;
    }

    if (strcasecmp(elementRoot.name(), "config") != 0) {
        Esyslog("Not a config file\n");
        return false;
    }

    // Check for all childe nodes if they contains a known element
    for (currentNode = elementRoot.first_child(); currentNode;
         currentNode = currentNode.next_sibling()) {

        if (currentNode.type() == node_element)  // is element
        {
            Dsyslog("Node Name %s\n", currentNode.name());

            if (!(
                    (strcasecmp(currentNode.name(), XML_GLOBAL) != 0) ||
                    (strcasecmp(currentNode.name(), XML_MENU) != 0) ||
                    (strcasecmp(currentNode.name(), XML_CECKEYMAP) != 0) ||
                    (strcasecmp(currentNode.name(), XML_VDRKEYMAP) != 0)
               )) {
                Esyslog("Invalid Node %s", currentNode.name());

                return false;
            }
        }
    }

    // Create the default device for TV
    cCECDevice device;
    device.mLogicalAddressDefined = CECDEVICE_TV;
    device.mLogicalAddressUsed = CECDEVICE_TV;
    device.mPhysicalAddress = 0x0000;
    string id = "TV";
    mDeviceMap.insert(std::pair<string, cCECDevice>(id, device));
    try {
        currentNode = currentNode.next_sibling(XML_GLOBAL);
        if (currentNode) {
            Esyslog("Only one global node is allowed");
            return false;
        }

        // Parse ceckeymaps
        for (currentNode = elementRoot.child(XML_CECKEYMAP); currentNode;
             currentNode = currentNode.next_sibling(XML_CECKEYMAP)) {
            parseCECKeymap(currentNode, keymaps);
        }
        // Parse vdrkeymaps
        for (currentNode = elementRoot.child(XML_VDRKEYMAP); currentNode;
                currentNode = currentNode.next_sibling(XML_VDRKEYMAP)) {
            parseVDRKeymap(currentNode, keymaps);
        }
        // Parse device
        for (currentNode = elementRoot.child(XML_DEVICE); currentNode;
                currentNode = currentNode.next_sibling(XML_DEVICE)) {
            parseDevice(currentNode);
        }

        // parse global node
        currentNode = elementRoot.child(XML_GLOBAL);
        parseGlobal(currentNode);

        // Parse all menus
        for (currentNode = elementRoot.child(XML_MENU); currentNode;
                currentNode = currentNode.next_sibling(XML_MENU)) {
            parseMenu(currentNode);
        }

        // Parse all onceccommand definitions
        for (currentNode = elementRoot.child(XML_ONCECCOMMAND); currentNode;
                currentNode = currentNode.next_sibling(XML_ONCECCOMMAND)) {
            parseOnCecCommand(currentNode);
        }

    } catch (const cCECConfigException &e) {
        Esyslog ("cCECConfigException %s\n", e.what());
        ret = false;
    } catch (const exception& e) {
        Esyslog ("Unexpected Exception %s", e.what());
        ret = false;
    }

    // Check that referenced menu entries in onceccommand/execmenu,startmenu
    // are defined
    if (ret) {
        cCECMenu m;
        for (mapCommandHandlerIterator i =
                mGlobalOptions.mCECCommandHandlers.begin();
                i != mGlobalOptions.mCECCommandHandlers.end(); i++) {
            cCECCommandHandler h = i->second;
            if (!h.mExecMenu.empty()) {
                if (!FindMenu(h.mExecMenu, m)) {
                    Esyslog("Menu %s in execmenu not found",
                            h.mExecMenu.c_str());
                    ret = false;
                }
            }

            if (!h.mStopMenu.empty()) {
                if (!FindMenu(h.mStopMenu, m)) {
                    Esyslog("Menu %s in stopmenu not found",
                            h.mStopMenu.c_str());
                    ret = false;
                }
            }
        }
    }
    return ret;
}

} // namespace cecplugin
