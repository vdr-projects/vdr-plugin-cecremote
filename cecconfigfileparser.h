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

// Class for storing information on <global> tags.

typedef std::list<cec_device_type> deviceTypeList;
typedef deviceTypeList::const_iterator deviceTypeListIterator;

class cCECGlobalOptions {
public:
    int cec_debug;
    uint32_t mComboKeyTimeoutMs;
    cCmdQueue mOnStart;
    cCmdQueue mOnStop;
    cCmdQueue mOnManualStart;
    cCmdQueue mOnSwitchToTV;
    cCmdQueue mOnSwitchToRadio;
    cCmdQueue mOnSwitchToReplay;
    deviceTypeList mDeviceTypes;

    cCECGlobalOptions() : cec_debug(7), mComboKeyTimeoutMs(1000) {};
};

typedef std::map<std::string, cCECDevice> mCECDeviceMap;

// Class for storing information on <menu> tags.
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
    cCECDevice mDevice;
    cCmdQueue mOnStart;
    cCmdQueue mOnStop;
    cCmdQueue mOnPowerOn;
    cCmdQueue mOnPowerOff;
    std::string mCECKeymap;
    std::string mVDRKeymap;

    cCECMenu() : mCECKeymap(cCECkeymaps::DEFAULTKEYMAP),
                 mVDRKeymap(cCECkeymaps::DEFAULTKEYMAP),
                 mPowerToggle(UNDEFINED) {};

    bool isMenuPowerToggle() const { return (mPowerToggle == USE_ONPOWER); };
    bool isStopKey(eKeys key) { return (mStopKeys.find(key) != mStopKeys.end()); };
private:
    PowerToggleState mPowerToggle;
};

typedef std::list<cCECMenu> cCECMenuList;
typedef cCECMenuList::const_iterator cCECMenuListIterator;

// Exception thrown when a systax error occurred.
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

    // Returns an error text.
    const char *what() const throw() {
        char buf[10];
        sprintf(buf,"%d\n", mLineNr);
        static std::string s = "Syntax error in line ";
        s += buf + mTxt;
        return s.c_str();
    }
};

// Configuration file parser
class cCECConfigFileParser {
private:
    // Helper function to get the line number from the byte offset in the XML
    // error.
    int getLineNumber(long offset);
    // Helper function to convert string to cec_device_type.
    cec_device_type getDeviceType(const std::string &s);
    // Check if a tag contains child elements.
    bool hasElements(const pugi::xml_node node);
    // Helper function to get device address
    void getDevice(const char *text, cCECDevice &device, ptrdiff_t linenr);
    // Convert text to int, returns false if conversion fails.
    bool textToInt(const char *text, int &val, int base = 10);
    bool textToInt(const char *text, uint16_t &val, int base = 10) {
        int v;
        bool ret = textToInt(text, v, base);
        val = v;
        return ret;
    };
    bool textToInt(const char *text, uint32_t &val, int base = 10) {
        int v;
        bool ret = textToInt(text, v, base);
        val = v;
        return ret;
    };
    bool textToInt(const char *text, cec_logical_address &val, int base = 10) {
        int v;
        bool ret = textToInt(text, v, base);
        val = (cec_logical_address)v;
        return ret;
    };
    // parse elements between <vdrkeymap>
    void parseVDRKeymap(const pugi::xml_node node, cCECkeymaps &keymaps);
    // parse elements between <ceckeymap>
    void parseCECKeymap(const pugi::xml_node node, cCECkeymaps &keymaps);
    // parse elements between <global>
    void parseGlobal(const pugi::xml_node node);
    // parse elements between <menu>
    void parseMenu(const pugi::xml_node node);
    // parse <onstart> and <onstop>
    void parseList(const pugi::xml_node node, cCmdQueue &cmdlist);
    void parsePlayer(const pugi::xml_node node, cCECMenu &menu);
    // parse elements between <device id="">
    void parseDevice(const pugi::xml_node node);

    // Keywords used in the XML config file
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
    static const char *XML_POWERON;
    static const char *XML_POWEROFF;
    static const char *XML_MAKEACTIVE;
    static const char *XML_MAKEINACTIVE;
    static const char *XML_EXEC;
    static const char *XML_TEXTVIEWON;
    static const char *XML_COMBOKEYTIMEOUTMS;
    static const char *XML_CECDEBUG;
    static const char *XML_CECDEVICETYPE;
    static const char *XML_DEVICE;
    static const char *XML_PHYSICAL;
    static const char *XML_LOGICAL;
    static const char *XML_ONMANUALSTART;
    static const char *XML_ONSWITCHTOTV;
    static const char *XML_ONSWITCHTORADIO;
    static const char *XML_ONSWITCHTOREPLAY;

    // Filename of the configuration file.
    const char* mXmlFile;

public:
    // Storage of the parsed global options.
    cCECGlobalOptions mGlobalOptions;
    // List of the parsed menu items.
    cCECMenuList mMenuList;
    // List of devices
    mCECDeviceMap mDeviceMap;

    cCECConfigFileParser() : mXmlFile(NULL) {};

    // Parse the file, fill mGlobalOptions and mMenuList and return the
    // parsed keymaps.
    // Returns false when a syntax error occurred during parsing.
    bool Parse(const std::string &filename, cCECkeymaps &keymaps);
};

#endif /* CONFIGFILEPARSER_H_ */
