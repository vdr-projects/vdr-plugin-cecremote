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

#ifndef CECCONFIGFILEPARSER_H_
#define CECCONFIGFILEPARSER_H_

#include <pugixml.hpp>
#include <fstream>
#include <iostream>
#include <sstream>

#include <string>
#include <list>
#include <map>
#include <queue>
#include <set>

#include "cecremote.h"

typedef std::set<eKeys> keySet;

class cCECGlobalOptions {
public:
    int cec_debug;
    cCmdQueue onStart;
    cCmdQueue onStop;
public:
    cCECGlobalOptions() : cec_debug(7) {};
};

class cCECMenu {
    friend class cCECConfigFileParser;
public:
    typedef enum {
        UNDEFINED,
        USE_ONSTART,
        USE_ONPOWER
    } PowerToggleState;
    std::string mMenuTitle;
    std::string mStillPic;
    keySet mStopKeys;
    cec_logical_address mAddress;
    cCmdQueue onStart;
    cCmdQueue onStop;
    cCmdQueue onPowerOn;
    cCmdQueue onPowerOff;
    std::string mCECKeymap;
    std::string mVDRKeymap;

    cCECMenu() : mAddress(CECDEVICE_UNKNOWN),
                 mCECKeymap(cCECkeymaps::DEFAULTKEYMAP),
                 mVDRKeymap(cCECkeymaps::DEFAULTKEYMAP),
                 mPowerToggle(UNDEFINED) {};

    bool isMenuPowerToggle() const { return (mPowerToggle == USE_ONPOWER); };
    bool isStopKey(eKeys key) { return (mStopKeys.find(key) != mStopKeys.end()); };
private:
    PowerToggleState mPowerToggle;
};

typedef std::list<cCECMenu> cCECMenuList;
typedef cCECMenuList::const_iterator cCECMenuListIterator;

class cCECConfigException : public std::exception {
private:
    int mLineNr;
    std::string mTxt;
public:
    cCECConfigException(int linenr, const std::string &txt) {
        mLineNr = linenr;
        mTxt = txt;
    }
    ~cCECConfigException() throw() {};

    const char *what() const throw() {
        char buf[10];
        sprintf(buf,"%d\n", mLineNr);
        static std::string s = "Syntax error in line ";
        s += buf + mTxt;
        return s.c_str();
    }
};

class cCECConfigFileParser {
private:
    int getLineNumber(long offset);
    bool hasElements(const pugi::xml_node node);
    void parseVDRKeymap(const pugi::xml_node node, cCECkeymaps &keymaps);
    void parseCECKeymap(const pugi::xml_node node, cCECkeymaps &keymaps);
    void parseGlobal(const pugi::xml_node node);
    void parseMenu(const pugi::xml_node node);
    void parseList(const pugi::xml_node node, cCmdQueue &cmdlist);
    void parsePlayer(const pugi::xml_node node, cCECMenu &menu);

    static const char *XML_GLOBAL;
    static const char *XML_MENU;
    static const char *XML_CECKEYMAP;
    static const char *XML_VDRKEYMAP;
    static const char *XML_ONSTART;
    static const char *XML_ONSTOP;
    static const char *XML_ONPOWERON;
    static const char *XML_ONPOWEROFF;
    static const char *XML_ID;
    static const char *XML_KEY;
    static const char *XML_CODE;
    static const char *XML_VALUE;
    static const char *XML_STOP;
    static const char *XML_KEYMAPS;
    static const char *XML_FILE;
    static const char *XML_CEC;
    static const char *XML_VDR;

    const char* mXmlFile;

public:
    cCECGlobalOptions mGlobalOptions;
    cCECMenuList mMenuList;

    cCECConfigFileParser() : mXmlFile(NULL) {};

    bool Parse(const std::string &filename, cCECkeymaps &keymaps);
};

#endif /* CONFIGFILEPARSER_H_ */
