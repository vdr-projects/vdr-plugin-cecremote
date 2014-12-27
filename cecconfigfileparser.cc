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

string cCECConfigFileParser::mWhiteSpace = " \t\n";

void cCECConfigFileParser::AddKey (Key &kl, const string &key, const string &val)
{
    Key::iterator iter;
    stringList vl;
    iter = kl.find(key);
    if (iter != kl.end()) {
        vl = iter->second;
    }

    vl.push_back(val);
    kl[key] = vl;

}

void cCECConfigFileParser::AddSection (string sectionname,
                                         string key,
                                         const string val)
{
    Section::iterator iter;
    Key kl;
    sectionname = StringTools::ToUpper(sectionname);
    key = StringTools::ToUpper(key);
    iter = mSections.find(sectionname);
    if (iter != mSections.end()) {
        kl = iter->second;
    }

    AddKey(kl, key, val);
    mSections[sectionname] = kl;
}

bool cCECConfigFileParser::isSection (const string &token, string &section)
{
    size_t last = token.length()-1;
    if ((token.at(0) == '[') && (token.at(last) == ']')) {
        section = token.substr (1, last-1);
        return true;
    }
    return false;
}

stringQueue cCECConfigFileParser::TokenizeLine (const string line)
{
    stringQueue q;
    string newstr;
    size_t i;
    char c;
    bool apo_active = false;

    for (i = 0; i < line.size(); i++ )
    {
        c = line.at(i);
        if (isspace(c) && (!apo_active)) {
            if (!newstr.empty()) {
                q.push(newstr);
                newstr.clear();
            }
        }
        else if (c == '"') {
            if (apo_active) {
                q.push(newstr);
                newstr.clear();
                apo_active = false;
            }
            else
            {
                apo_active = true;
            }
        }
        else {
            newstr += c;
        }
    }
    if (!newstr.empty()) {
        q.push(newstr);
        newstr.clear();
    }
    return q;
}

bool cCECConfigFileParser::ParseLine (const string line)
{
    string token;
    string key;
    bool keysep = false;
    stringQueue q = TokenizeLine (line);

    while (!q.empty()) {
        token = q.front();
        q.pop();
        /* Comment found ? */
        if (isComment(token)) {
            return true;
        }
        /* Is a section ? */
        if (isSection(token, mCurrSection)) {
            if (!q.empty()) {
                token = q.front();
                q.pop();

                if (isComment(token)) {
                    return true;
                } else {
                    Esyslog( "Syntax error on section: %s", line.c_str());
                    return false;
                }
            }
            return true;
        }
        /* Read key */
        if ((!keysep) && (token != "=")) {
            if (key.empty()) {
                key = token;
            }
            else {
                Esyslog("Missing =: %s", line.c_str());
                return false;
            }
        }
        else if (token == "=") {
            if (keysep) {
                Esyslog("Duplicate = : %s", line.c_str());
                return false;
            }
            keysep = true;
        }
        /* Read Values of key */
        else {
            AddSection (mCurrSection, key, token);
        }
    }
    return true;
}

// Parse the config file
bool cCECConfigFileParser::Parse(const string fname)
{
    ifstream file;
    string line;

    mCurrSection.clear();
    file.open(fname.c_str());
    if (!file.is_open()) {
        string err = "Can not open file " + fname;
        Esyslog(err.c_str());
        return false;
    }

    while (getline (file, line)) {
        if (!ParseLine (line)) {
            return false;
        }
    }
    if (!file.eof()) {
        string err = "Read error on file " + fname;
        Esyslog(err.c_str());
        return false;
    }
    return true;
}

// Find a key in a given section and return the values as list.
// Returns true if the key was found.
bool cCECConfigFileParser::GetValues (const string sectionname,
                                       const string key,
                                       stringList &values)
{
    Section::iterator seciter;
    Key::iterator keyiter;
    Key kl;

    string section = StringTools::ToUpper(sectionname);
    string k = StringTools::ToUpper(key);
    values.clear();
    seciter = mSections.find(section);
    if (seciter == mSections.end()) {
        string err = "Can not find section " + section;
        Esyslog(err.c_str());
        return false;
    }
    kl = seciter->second;
    keyiter = kl.find(k);
    if (keyiter == kl.end()) {
        Esyslog("Can not find key %s\n",k.c_str());
        return false;
    }
    values = keyiter->second;
    return true;
}

// Return all keys in a given section and return the values as list.
// Returns true if the section was found.
bool cCECConfigFileParser::GetKeys (const string sectionname,
                                     stringList &values)
{
    Section::iterator seciter;
    Key::iterator keyiter;
    Key kl;

    string section = StringTools::ToUpper(sectionname);

    values.clear();
    seciter = mSections.find(section);
    if (seciter == mSections.end()) {
        string err = "Can not find section " + section;
        Esyslog(err.c_str());
        return false;
    }
    values.clear();
    kl = seciter->second;
    for (keyiter = kl.begin(); keyiter != kl.end(); keyiter++) {
        values.push_back(keyiter->first);
    }

    return true;
}
// Return a a key in a given section as single value (not split into a key list)

bool cCECConfigFileParser::GetSingleValue (const string sectionname,
                                             const string key,
                                             string &value)
{
    stringList vals;

    string plugin;
    if (!GetValues(sectionname, key, vals)) {
        return false;
    }
    if (vals.size() != 1) {
        string err = "More than one argument to " + sectionname +  " key " + key;
        Esyslog(err.c_str());
        return false;
    }
    value = vals.front();
    return true;
}

// Get first section of a config file, returning the section name
bool cCECConfigFileParser::GetFirstSection (Section::iterator &iter,
                                              string &sectionname)
{
    if (mSections.empty()) {
        return false;
    }
    iter = mSections.begin();
    sectionname = iter->first;
    return true;
}

// Get next section, returning the section name
// Returns false if no next section exist
bool cCECConfigFileParser::GetNextSection (Section::iterator &iter,
                                             string &sectionname)
{
    iter++;
    if (iter == mSections.end()) {
        return false;
    }
    sectionname = iter->first;
    return true;
}

// Helper function to validate that a section contains only required and
// optional keywords and that all required keywords are present.

bool cCECConfigFileParser::CheckSection (const string sectionname,
                                      const stringSet required,
                                      const stringSet optional)
{
    stringList::iterator it;
    stringList keys;

    map <string,bool> reqmap;
    map <string,bool>::iterator reqit;

// Check that required keywords are available
    stringSet::iterator req;
    for (req = required.begin(); req != required.end(); req++) {
        string s = *req;
        reqmap[s] = false;
    }
    // Check for required and optional keywords.
    if (!GetKeys(sectionname, keys)) {
        Esyslog( "Invalid section %s", sectionname.c_str());
        return false;
    }

    for (it = keys.begin(); it != keys.end(); it++) {
        string s = *it;
        bool inreq = (reqmap.find(s) != reqmap.end());
        bool inopt = (optional.find(s) != optional.end());
        if ((!inreq) && (!inopt)) {
            Esyslog( "Invalid keyword %s in section %s",
                            s.c_str(),
                            sectionname.c_str());
            return false;
        }
        if (inreq) {
            reqmap[s] = true;
        }
    }

    for (reqit = reqmap.begin(); reqit != reqmap.end(); reqit++) {
        if (!reqit->second) {
             Esyslog( "Required keyword %s missing in section %s",
                            reqit->first.c_str(),
                            sectionname.c_str());
            return false;
        }
    }
    return true;
}
