/*
 * configfileparser.cc: Class for parsing a configuration file.
 *
 * The config file has the following syntax:
 * [SECTION NAME]
 * <KEY> = <VALUE> <VALUE> <VALUE>
 *
 * Copyright (C) 2014 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 */
#include <vdr/plugin.h>
#include "cecconfigfileparser.h"
#include "ceclog.h"
#include "stringtools.h"

using namespace std;
using namespace xercesc;

cCECConfigFileParser::cCECConfigFileParser()
{
    try {
        XMLPlatformUtils::Initialize();
    } catch (const XMLException& toCatch) {
        char* message = XMLString::transcode(toCatch.getMessage());
        cout << "Error during initialization! :\n" << message << "\n";
        XMLString::release(&message);
        exit(-1);
    }
    mWildcard = XMLString::transcode("*");
    mGlobal = XMLString::transcode("global");
    mMenu = XMLString::transcode("menu");
    mConfig = XMLString::transcode("config");
    mCecDebug = XMLString::transcode("cecdebug");
    mOnStart = XMLString::transcode("onstart");
    mOnStop = XMLString::transcode("onstop");
    mPowerOn = XMLString::transcode("poweron");
    mPowerOff = XMLString::transcode("poweroff");
    mExec = XMLString::transcode("exec");
    mName = XMLString::transcode("name");
    mStillPic = XMLString::transcode("stillpic");
}

cCECConfigFileParser::~cCECConfigFileParser()
{
    XMLString::release(&mWildcard);
    XMLString::release(&mGlobal);
    XMLString::release(&mMenu);
    XMLString::release(&mConfig);
    XMLString::release(&mCecDebug);
    XMLString::release(&mOnStart);
    XMLString::release(&mOnStop);
    XMLString::release(&mPowerOn);
    XMLString::release(&mPowerOff);
    XMLString::release(&mExec);
    XMLString::release(&mName);
    XMLString::release(&mStillPic);
}

void cCECConfigFileParser::parseList(const DOMNodeList *nodelist,
                                     cCECCommandList &cmdlist)
{
    cCECCommand cmd;
    const XMLSize_t nodeCount = nodelist->getLength();

    for(XMLSize_t i = 0; i < nodeCount; ++i)
    {
        DOMNode* currentNode = nodelist->item(i);

        if (currentNode->getNodeType() == DOMNode::ELEMENT_NODE)  // is element
        {
            char *str = XMLString::transcode(currentNode->getNodeName());
            Dsyslog("     OnXXX %s\n", str);
            XMLString::release(&str);

            // Found node which is an Element. Re-cast node as element
            DOMElement* curElem =
                    dynamic_cast<xercesc::DOMElement*>(currentNode);

            DOMNodeList *li = curElem->getChildNodes();
            if (li->getLength() > 1) {
                str = XMLString::transcode(currentNode->getNodeName());
                string s = "Too much arguments for ";
                s += str;
                XMLString::release(&str);
                throw cCECConfigException(0, s);
            }
            if (XMLString::compareIString(curElem->getNodeName(),
                                            mPowerOn) == 0) {
                cmd.mCommandType = cCECCommand::POWER_ON;
                cmd.mCECAddress = XMLString::parseInt(curElem->getTextContent());
                cmd.mExec = "";
                Dsyslog("         POWERON %d\n", cmd.mCECAddress);
                cmdlist.push_back(cmd);
            }
            else if (XMLString::compareIString(curElem->getNodeName(),
                                               mPowerOff) == 0) {
                cmd.mCommandType = cCECCommand::POWER_OFF;
                cmd.mCECAddress = XMLString::parseInt(
                        curElem->getTextContent());
                cmd.mExec = "";
                Dsyslog("         POWEROFF %d\n", cmd.mCECAddress);
                cmdlist.push_back(cmd);
            } else if (XMLString::compareIString(curElem->getNodeName(),
                                                 mExec) == 0) {
                str = XMLString::transcode(curElem->getTextContent());
                cmd.mCommandType = cCECCommand::EXEC;
                cmd.mCECAddress = 0;
                cmd.mExec = str;
                Dsyslog("         EXEC %s\n", cmd.mExec.c_str());
                cmdlist.push_back(cmd);
                XMLString::release(&str);
            }
            else {
                str = XMLString::transcode(currentNode->getNodeName());
                string s = "Invalid command ";
                s += str;
                XMLString::release(&str);
                throw cCECConfigException(0, s);
            }
        }
    }
}

void cCECConfigFileParser::parseMenu(const DOMNodeList *list, DOMElement *menuElem)
{
    cCECMenu menu;
    char *str = XMLString::transcode(menuElem->getAttribute(mName));
    Dsyslog ("  Menu %s\n", str);
    menu.mMenuTitle = str;
    XMLString::release(&str);
    if (menu.mMenuTitle.empty()) {
        string s = "Missing menu name";
        throw cCECConfigException(0, s);
    }

    const  XMLSize_t nodeCount = list->getLength();

    for(XMLSize_t i = 0; i < nodeCount; ++i)
    {
        DOMNode* currentNode = list->item(i);

        if (currentNode->getNodeType() == DOMNode::ELEMENT_NODE)  // is element
        {
            DOMElement* curElem =
                                dynamic_cast<xercesc::DOMElement*>(currentNode);
            DOMNodeList *li = curElem->getChildNodes();

            if (XMLString::compareIString(curElem->getNodeName(),
                    mStillPic) == 0) {
                if (li->getLength() > 1) {
                    string s = "Too much arguments";
                    s += std::to_string( li->getLength());
                    throw cCECConfigException(0, s);
                }
                char *str = XMLString::transcode(curElem->getTextContent());
                menu.mStillPic = str;
                XMLString::release(&str);

                Dsyslog("StillPic = %s\n", menu.mStillPic.c_str());
            }
            else if (XMLString::compareIString(curElem->getNodeName(),
                    mOnStart) == 0) {
                parseList(li, menu.onStart);
            }
            else if (XMLString::compareIString(curElem->getNodeName(),
                    mOnStop) == 0) {
                parseList(li, menu.onStop);
            }
            else {
                char *str = XMLString::transcode(curElem->getNodeName());
                string s = "Invalid Command ";
                s += str;
                XMLString::release(&str);
                throw cCECConfigException(0, s);
            }
        }
    }
    mMenuList.push_back(menu);
}

void cCECConfigFileParser::parseGlobal(const DOMNodeList *list)
{
    const  XMLSize_t nodeCount = list->getLength();

    for(XMLSize_t i = 0; i < nodeCount; ++i)
    {
        DOMNode* currentNode = list->item(i);

        if (currentNode->getNodeType() == DOMNode::ELEMENT_NODE)  // is element
        {
            char *str = XMLString::transcode(currentNode->getNodeName());
            Dsyslog("   Global Option %s\n", str);
            XMLString::release(&str);

            // Found node which is an Element. Re-cast node as element
            DOMElement* curElem =
                    dynamic_cast<xercesc::DOMElement*>(currentNode);

            DOMNodeList *li = curElem->getChildNodes();

            if (XMLString::compareIString(curElem->getNodeName(),
                                          mCecDebug) == 0) {
                if (li->getLength() > 1) {
                    string s = "Too much arguments";
                    s += std::to_string(li->getLength());
                    throw cCECConfigException(0, s);
                }
                mGlobalOptions.cec_debug =
                                XMLString::parseInt(curElem->getTextContent());
                Dsyslog("CECDebug = %d\n", mGlobalOptions.cec_debug);
            }
            else if (XMLString::compareIString(curElem->getNodeName(),
                                               mOnStart) == 0) {
                parseList(li, mGlobalOptions.onStart);
            }
            else if (XMLString::compareIString(curElem->getNodeName(),
                                               mOnStop) == 0) {
                parseList(li, mGlobalOptions.onStop);
            }
            else {
                char *str = XMLString::transcode(curElem->getNodeName());
                string s = "Invalid Command ";
                s += str;
                XMLString::release(&str);
                throw cCECConfigException(0, s);
            }
        }
    }
}

bool cCECConfigFileParser::Parse(const string &filename) {
    bool ret = true;

    XercesDOMParser *parser = new XercesDOMParser;
    parser->setValidationScheme(XercesDOMParser::Val_Never);
    parser->setDoNamespaces(false);
    parser->setDoSchema(false);
    parser->setLoadExternalDTD(false);
    ErrorHandler* errHandler = (ErrorHandler*)new HandlerBase();
    parser->setErrorHandler(errHandler);

    const char* xmlFile = filename.c_str();

    try {
        parser->parse(xmlFile);

        DOMDocument* xmlDoc = parser->getDocument();
        // Get the top-level element: NAme is "root". No attributes for "root"
        DOMElement* elementRoot = xmlDoc->getDocumentElement();
        if (elementRoot == NULL) {
            Esyslog("Element Root NULL\n");
            return false;
        }

        if (XMLString::compareIString(elementRoot->getTagName(), mConfig) != 0) {
            Esyslog("Not a config file\n");
            return false;
        }

        const DOMNodeList *list = elementRoot->getChildNodes();
        const XMLSize_t nodeCount = list->getLength();

        // For all nodes, children of "root" in the XML tree.

        for (XMLSize_t i = 0; i < nodeCount; ++i) {
            DOMNode* currentNode = list->item(i);

            if (currentNode->getNodeType() == DOMNode::ELEMENT_NODE)  // is element
            {
                char *str = XMLString::transcode(currentNode->getNodeName());
                Dsyslog("Node Name %s\n", str);
                XMLString::release(&str);

                // Found node which is an Element. Re-cast node as element
                DOMElement* curElem =
                        dynamic_cast<xercesc::DOMElement*>(currentNode);
                DOMNodeList *li = curElem->getChildNodes();

                if (XMLString::compareIString(currentNode->getNodeName(), mGlobal) == 0) {
                    parseGlobal(li);
                }
                else if (XMLString::compareIString(currentNode->getNodeName(), mMenu) == 0) {
                    parseMenu(li, curElem);
                }
                else {
                    char *str = XMLString::transcode(currentNode->getNodeName());
                    Esyslog("Invalid Command %s", str);
                    XMLString::release(&str);
                    return false;
                }
            }
        }
    } catch (xercesc::XMLException &e) {
        char* message = xercesc::XMLString::transcode(e.getMessage());
        Esyslog("Error parsing file %s: %s", xmlFile, message);
        XMLString::release(&message);
        ret = false;
    } catch (const DOMException& toCatch) {
        char* message = XMLString::transcode(toCatch.msg);
        Esyslog("DOMException message is: %s", message);
        XMLString::release(&message);
        ret = false;
    } catch (const SAXParseException &e) {
        char* message = XMLString::transcode(e.getMessage());
        Esyslog("SAXParseException: %d %s\n, ", e.getLineNumber(), message);
        XMLString::release(&message);
        ret = false;
    } catch (const cCECConfigException &e) {
        Esyslog ("%s\n", e.what());
        ret = false;
    } catch (const exception& e) {
        Esyslog ("Unexpected Exception %s", e.what());
        ret = false;
    }

    delete parser;
    delete errHandler;
    return ret;
}
