/*
 * configfileparser.h: Class for parsing a configuration file.
 *
 * Copyright (C) 2010 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 */

#ifndef CECCONFIGFILEPARSER_H_
#define CECCONFIGFILEPARSER_H_

#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>

#include <fstream>
#include <iostream>
#include <sstream>

#include <string>
#include <list>
#include <map>
#include <queue>
#include <set>

typedef std::queue<std::string> stringQueue;
typedef std::list<std::string> stringList;
typedef std::set<std::string> stringSet;
//typedef std::map<std::string, stringList> Key;
//typedef std::map<std::string, Key> Section;

class cCECCommand {
public:
    typedef enum {
        UNDEFINED = 0,
        POWER_ON = 1,
        POWER_OFF,
        EXEC
    } CEC_COMMAND_TYPE;

    cCECCommand() : mCommandType(UNDEFINED), mCECAddress(0) {}
    cCECCommand(CEC_COMMAND_TYPE type, int addr) : mCommandType(type), mCECAddress(addr) {}
    cCECCommand(CEC_COMMAND_TYPE type, std::string exec) :
        mCommandType(type), mCECAddress(0), mExec(exec) {}

    CEC_COMMAND_TYPE mCommandType;
    int mCECAddress;
    std::string mExec;
};

typedef std::list<cCECCommand> cCECCommandList;

class cCECGlobalOptions {
public:
    int cec_debug;
    cCECCommandList onStart;
    cCECCommandList onStop;
public:
    cCECGlobalOptions() : cec_debug(7) {};
};

class cCECMenu {
public:
    std::string mMenuTitle;
    std::string mStillPic;
    cCECCommandList onStart;
    cCECCommandList onStop;

    cCECMenu() {};
};

typedef std::list<cCECMenu> cCECMenuList;

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
        static std::string s = "Syntax error: ";
       // s += std::to_string( mLineNr );
        s += " " + mTxt;
        return s.c_str();
    }
};

class cCECConfigFileParser {
private:
    cCECGlobalOptions mGlobalOptions;
    cCECMenuList mMenuList;

    void parseGlobal(const xercesc::DOMNodeList *list);
    void parseMenu(const xercesc::DOMNodeList *list, xercesc::DOMElement *menuElem);
    void parseList(const xercesc::DOMNodeList *nodelist, cCECCommandList &cmdlist);

    XMLCh *mWildcard;
    XMLCh *mGlobal;
    XMLCh *mMenu;
    XMLCh *mConfig;
    XMLCh *mCecDebug;
    XMLCh *mOnStart;
    XMLCh *mOnStop;
    XMLCh *mPowerOn;
    XMLCh *mPowerOff;
    XMLCh *mExec;
    XMLCh *mName;
    XMLCh *mStillPic;

public:
    cCECConfigFileParser();
    ~cCECConfigFileParser();
    bool Parse (const std::string &filename);
};

#endif /* CONFIGFILEPARSER_H_ */
