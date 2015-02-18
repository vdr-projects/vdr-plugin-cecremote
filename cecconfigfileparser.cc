/*
 * CECRemote PlugIn for VDR
 *
 * Copyright (C) 2015 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 * cecconfigfileparser.h: Class for parsing the plugin configuration file.
 */

#include <vdr/plugin.h>
#include "ceclog.h"
#include "cecconfigfileparser.h"
#include "stringtools.h"

using namespace std;
using namespace pugi;

const char *cCECConfigFileParser::ONSTART = "onstart";
const char *cCECConfigFileParser::ONSTOP  = "onstop";
const char *cCECConfigFileParser::ONPOWERON = "onpoweron";
const char *cCECConfigFileParser::ONPOWEROFF = "onpoweroff";

cCECConfigFileParser::cCECConfigFileParser() : mXmlFile(NULL)
{

}

cCECConfigFileParser::~cCECConfigFileParser()
{

}

void cCECConfigFileParser::parsePlayer(const xml_node node, cCECMenu &menu)
{
    menu.mStillPic = node.attribute("file").as_string("");
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
            if (strcasecmp(currentNode.name(), "stop") == 0) {
                eKeys k = cKey::FromString(currentNode.text().as_string());
                if (k == kNone) {
                    string s = "Invalid key ";
                    s += currentNode.text().as_string();
                    throw cCECConfigException(getLineNumber(currentNode.offset_debug()), s);
                }
                menu.mStopKeys.insert(k);
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

bool cCECConfigFileParser::hasElements(const xml_node node)
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

void cCECConfigFileParser::parseList(const xml_node node,
                                     cCmdQueue &cmdlist)
{
    cCECCmd cmd;

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
            if (strcasecmp(currentNode.name(), "poweron") == 0) {
                cmd.mCmd = CEC_POWERON;
                cmd.mAddress = (cec_logical_address)
                                                currentNode.text().as_int(-1);
                cmd.mExec = "";
                Dsyslog("         POWERON %d\n", cmd.mAddress);
                cmdlist.push_back(cmd);
            }
            else if (strcasecmp(currentNode.name(), "poweroff") == 0) {
                cmd.mCmd = CEC_POWEROFF;
                cmd.mAddress = (cec_logical_address)
                                                currentNode.text().as_int(-1);
                cmd.mExec = "";
                Dsyslog("         POWEROFF %d\n", cmd.mAddress);
                cmdlist.push_back(cmd);
            } else if (strcasecmp(currentNode.name(), "makeactive") == 0) {
                cmd.mCmd = CEC_MAKEACTIVE;
                cmd.mAddress = CECDEVICE_UNKNOWN;
                cmd.mExec = "";
                Dsyslog("         MAKEACTIVE %d\n", cmd.mAddress);
                cmdlist.push_back(cmd);
            } else if (strcasecmp(currentNode.name(),"makeinactive") == 0) {
                cmd.mCmd = CEC_MAKEINACTIVE;
                cmd.mAddress = CECDEVICE_UNKNOWN;
                cmd.mExec = "";
                Dsyslog("         MAKEINACTIVE %d\n", cmd.mAddress);
                cmdlist.push_back(cmd);
            } else if (strcasecmp(currentNode.name(), "exec") == 0) {
                cmd.mCmd = CEC_EXECSHELL;
                cmd.mAddress = CECDEVICE_UNKNOWN;
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

void cCECConfigFileParser::parseMenu(const xml_node node)
{
    cCECMenu menu;

    menu.mMenuTitle = node.attribute("name").as_string("");
    if (menu.mMenuTitle.empty()) {
        string s = "Missing menu name";
        Esyslog(s.c_str());
        throw cCECConfigException(0, s);
    }
    menu.mAddress = (cec_logical_address)node.attribute("address").as_int(CECDEVICE_UNKNOWN);
    if (menu.mAddress == CECDEVICE_UNKNOWN) {
        string s = "Missing address";
        Esyslog(s.c_str());
        throw cCECConfigException(0, s);
    }
    Dsyslog ("  Menu %s (%d)\n", menu.mMenuTitle.c_str(), menu.mAddress);

    for (xml_node currentNode = node.first_child(); currentNode;
         currentNode = currentNode.next_sibling()) {

        if (currentNode.type() == node_element)  // is element
        {
            if (strcasecmp(currentNode.name(), "player") == 0) {
               parsePlayer(currentNode, menu);
            }
            else if (strcasecmp(currentNode.name(), ONSTART) == 0) {
                if ((menu.mPowerToggle == cCECMenu::UNDEFINED) ||
                    (menu.mPowerToggle == cCECMenu::USE_ONSTART))
                {
                    menu.mPowerToggle = cCECMenu::USE_ONSTART;
                    parseList(currentNode, menu.onStart);
                }
                else
                {
                    string s = "Either <onstart> or <onpower..> is allowed";
                    throw cCECConfigException(getLineNumber(currentNode.offset_debug()), s);
                }
            }
            else if (strcasecmp(currentNode.name(), ONSTOP) == 0) {
                if ((menu.mPowerToggle == cCECMenu::UNDEFINED) ||
                   (menu.mPowerToggle == cCECMenu::USE_ONSTART))
               {
                   menu.mPowerToggle = cCECMenu::USE_ONSTART;
                   parseList(currentNode, menu.onStop);
               }
               else
               {
                   string s = "Either <onstop> or <onpower..> is allowed";
                   throw cCECConfigException(getLineNumber(currentNode.offset_debug()), s);
               }
            }
            else if (strcasecmp(currentNode.name(), ONPOWERON) == 0) {
                if ((menu.mPowerToggle == cCECMenu::UNDEFINED) ||
                    (menu.mPowerToggle == cCECMenu::USE_ONPOWER))
                {
                    menu.mPowerToggle = cCECMenu::USE_ONPOWER;
                    parseList(currentNode, menu.onPowerOn);
                }
                else
                {
                    string s = "Either <onstart>/<onstop> or <onpoweron> is allowed";
                    throw cCECConfigException(getLineNumber(currentNode.offset_debug()), s);
                }
            }
            else if (strcasecmp(currentNode.name(), ONPOWEROFF) == 0) {
                if ((menu.mPowerToggle == cCECMenu::UNDEFINED) ||
                   (menu.mPowerToggle == cCECMenu::USE_ONPOWER))
               {
                   menu.mPowerToggle = cCECMenu::USE_ONPOWER;
                   parseList(currentNode, menu.onPowerOff);
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

void cCECConfigFileParser::parseGlobal(const pugi::xml_node node)
{
    for (xml_node currentNode = node.first_child(); currentNode;
         currentNode = currentNode.next_sibling()) {

        if (currentNode.type() == node_element)  // is element
        {
            Dsyslog("   Global Option %s\n", currentNode.name());

            if (strcasecmp(currentNode.name(), "cecdebug") == 0) {
                if (!currentNode.first_child()) {
                    string s = "No nodes allowed for cecdebug: ";
                    s += currentNode.first_child().name();
                    throw cCECConfigException(getLineNumber(currentNode.offset_debug()), s);
                }
                mGlobalOptions.cec_debug = currentNode.text().as_int(-1);
                Dsyslog("CECDebug = %d \n", mGlobalOptions.cec_debug);
            }
            else if (strcasecmp(currentNode.name(), ONSTART) == 0) {
                parseList(currentNode, mGlobalOptions.onStart);
            }
            else if (strcasecmp(currentNode.name(), ONSTOP) == 0) {
                parseList(currentNode, mGlobalOptions.onStop);
            }
            else {
                string s = "Invalid Node ";
                s += currentNode.name();
                throw cCECConfigException(getLineNumber(currentNode.offset_debug()), s);
            }
        }
    }
}

int cCECConfigFileParser::getLineNumber(long offset) {
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

bool cCECConfigFileParser::Parse(const string &filename) {
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
                    (strcasecmp(currentNode.name(), "global") != 0) ||
                    (strcasecmp(currentNode.name(), "menu") != 0)
               )) {
                Esyslog("Invalid Node %s", currentNode.name());

                return false;
            }
        }
    }

    try {
        // First parse global node
        currentNode = elementRoot.child("global");
        parseGlobal(currentNode);

        currentNode = currentNode.next_sibling("global");
        if (currentNode) {
            Esyslog("Only one global node is allowed");
            return false;
        }

        // Parse all menus
        for (currentNode = elementRoot.child("menu"); currentNode;
             currentNode = currentNode.next_sibling("menu")) {
            parseMenu(currentNode);
        }

    } catch (const cCECConfigException &e) {
        Esyslog ("cCECConfigException %s\n", e.what());
        ret = false;
    } catch (const exception& e) {
        Esyslog ("Unexpected Exception %s", e.what());
        ret = false;
    }

    return ret;
}
