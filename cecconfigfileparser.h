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
typedef std::map<std::string, stringList> Key;
typedef std::map<std::string, Key> Section;

class cCECConfigFileParser {
private:
    static std::string mWhiteSpace;
    Section mSections;
    std::string mCurrSection;

    void AddKey (Key &, const std::string &, const std::string &);
    void AddSection (std::string , std::string , const std::string);
    bool isSection (const std::string &token, std::string &section);
    bool isComment (const std::string &token) {return (token.at(0) == ';');}
    bool ParseLine (const std::string line);
    stringQueue TokenizeLine (const std::string line);

public:
    cCECConfigFileParser() : mCurrSection("") {};
    bool Parse (const std::string filename);
    bool GetKeys (const std::string sectionname, stringList &values);
    bool GetFirstSection (Section::iterator &iter, std::string &sectionname);
    bool GetNextSection (Section::iterator &iter, std::string &sectionname);
    bool GetValues (const std::string sectionname, const std::string key,
                      stringList &values);
    bool GetSingleValue (const std::string sectionname, const std::string key,
                           std::string &values);
    bool CheckSection (const std::string sectionname,
                         const stringSet required,
                         const stringSet optional);
};

#endif /* CONFIGFILEPARSER_H_ */
