/*
 * stringtools: string functions
 *
 *
 * Copyright (C) 2010 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 */

#ifndef STRINGTOOLS_H_
#define STRINGTOOLS_H_

#include <string>
#include <algorithm>
#include <stdlib.h>

namespace cecplugin {

class StringTools
{
public:
    static std::string ToUpper (std::string str) {
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
        return str;
    }
    static std::string IntToStr (int val) {
        char buf[20];
        std::string s;
        sprintf(buf,"%d", val);
        s = buf;
        return s;
    }

    // Trim trailing spaces and tabs.
    static void StrTrimTrail(std::string &s) {
        size_t endpos = s.find_last_not_of(" \t"); // Trim space and tabs
        if (std::string::npos != endpos) {
            s = s.substr(0, endpos + 1);
        }
    }

    static bool TextToInt(std::string s, int &val, int base = 0) {
        char *endp = NULL;

        StrTrimTrail(s);
        val = strtol(s.c_str(), &endp, 0);

        return (*endp == '\0');
    }
};

} // namespace cecplugin

#endif /* STRINGTOOLS_H_ */
