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

#include "cecremote.h"

typedef std::queue<std::string> stringQueue;
typedef std::list<std::string> stringList;
typedef std::set<std::string> stringSet;

class cCECGlobalOptions {
public:
    int cec_debug;
    cCmdQueue onStart;
    cCmdQueue onStop;
public:
    cCECGlobalOptions() : cec_debug(7) {};
};

class cCECMenu {
public:
    std::string mMenuTitle;
    std::string mStillPic;
    cec_logical_address mAddress;
    cCmdQueue onStart;
    cCmdQueue onStop;

    cCECMenu() {};
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
        static std::string s = "Syntax error: ";
       // s += std::to_string( mLineNr );
        s += " " + mTxt;
        return s.c_str();
    }
};

class cCECConfigFileParser {
private:
    void parseGlobal(const xercesc::DOMNodeList *list);
    void parseMenu(const xercesc::DOMNodeList *list, xercesc::DOMElement *menuElem);
    void parseList(const xercesc::DOMNodeList *nodelist, cCmdQueue &cmdlist);

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
    XMLCh *mAddress;

public:
    cCECGlobalOptions mGlobalOptions;
    cCECMenuList mMenuList;

    cCECConfigFileParser();
    ~cCECConfigFileParser();
    bool Parse (const std::string &filename);
};

#endif /* CONFIGFILEPARSER_H_ */
