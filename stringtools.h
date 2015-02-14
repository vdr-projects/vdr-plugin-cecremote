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
};

#endif /* STRINGTOOLS_H_ */
