/*
 * CECRemote PlugIn for VDR
 *
 * Copyright (C) 2015 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 * ceckeymaps.cc: Class for handling the keymaps.
 */

#include <stdexcept>
#include "ceckeymaps.h"
#include "ceclog.h"

using namespace std;

const char *cCECkeymaps::DEFAULTKEYMAP = "default";

cCECkeymaps::cCECkeymaps() {
    for (int i = 0; i <= CEC_USER_CONTROL_CODE_MAX; i++) {
        mDefaultKeyMap[i][0] = kNone;
        mDefaultKeyMap[i][1] = kNone;
    }

    mDefaultKeyMap[CEC_USER_CONTROL_CODE_SELECT             ][0] = kOk;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_UP                 ][0] = kUp;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_DOWN               ][0] = kDown;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_LEFT               ][0] = kLeft;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_RIGHT              ][0] = kRight;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_RIGHT_UP           ][0] = kRight;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_RIGHT_UP           ][1] = kUp;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_RIGHT_DOWN         ][0] = kRight;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_RIGHT_DOWN         ][1] = kDown;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_LEFT_UP            ][0] = kLeft;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_LEFT_UP            ][1] = kUp;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_LEFT_DOWN          ][0] = kRight;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_LEFT_DOWN          ][1] = kDown;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_ROOT_MENU          ][0] = kMenu;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_SETUP_MENU         ][0] = kSetup;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_CONTENTS_MENU      ][0] = kMenu;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_NUMBER0            ][0] = k0;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_NUMBER1            ][0] = k1;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_NUMBER2            ][0] = k2;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_NUMBER3            ][0] = k3;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_NUMBER4            ][0] = k4;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_NUMBER5            ][0] = k5;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_NUMBER6            ][0] = k6;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_NUMBER7            ][0] = k7;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_NUMBER8            ][0] = k8;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_NUMBER9            ][0] = k9;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_ENTER              ][0] = kOk;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_CLEAR              ][0] = kBack;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_CHANNEL_UP         ][0] = kChanUp;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_CHANNEL_DOWN       ][0] = kChanDn;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_PREVIOUS_CHANNEL   ][0] = kChanPrev;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_SOUND_SELECT       ][0] = kAudio;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_DISPLAY_INFORMATION][0] = kInfo;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_PAGE_UP            ][0] = kNext;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_PAGE_DOWN          ][0] = kPrev;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_POWER              ][0] = kPower;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_VOLUME_UP          ][0] = kVolUp;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_VOLUME_DOWN        ][0] = kVolDn;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_MUTE               ][0] = kMute;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_PLAY               ][0] = kPlay;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_STOP               ][0] = kStop;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_PAUSE              ][0] = kPause;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_RECORD             ][0] = kRecord;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_REWIND             ][0] = kFastRew;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_FAST_FORWARD       ][0] = kFastFwd;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_FORWARD            ][0] = kFastFwd;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_BACKWARD           ][0] = kFastRew;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_SUB_PICTURE        ][0] = kSubtitles;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_F1_BLUE            ][0] = kBlue;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_F2_RED             ][0] = kRed;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_F3_GREEN           ][0] = kGreen;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_F4_YELLOW          ][0] = kYellow;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_AN_RETURN          ][0] = kBack;
    mDefaultKeyMap[CEC_USER_CONTROL_CODE_EXIT               ][0] = kBack;

    memset(mCECKeyNames, 0, sizeof(mCECKeyNames));
    mCECKeyNames[CEC_USER_CONTROL_CODE_SELECT             ] = "SELECT";
    mCECKeyNames[CEC_USER_CONTROL_CODE_UP                 ] = "UP";
    mCECKeyNames[CEC_USER_CONTROL_CODE_DOWN               ] = "DOWN";
    mCECKeyNames[CEC_USER_CONTROL_CODE_LEFT               ] = "LEFT";
    mCECKeyNames[CEC_USER_CONTROL_CODE_RIGHT              ] = "RIGHT";
    mCECKeyNames[CEC_USER_CONTROL_CODE_RIGHT_UP           ] = "RIGHT_UP";
    mCECKeyNames[CEC_USER_CONTROL_CODE_RIGHT_DOWN         ] = "RIGHT_DOWN";
    mCECKeyNames[CEC_USER_CONTROL_CODE_LEFT_UP            ] = "LEFT_UP";
    mCECKeyNames[CEC_USER_CONTROL_CODE_LEFT_DOWN          ] = "LEFT_DOWN";
    mCECKeyNames[CEC_USER_CONTROL_CODE_ROOT_MENU          ] = "ROOT_MENU";
    mCECKeyNames[CEC_USER_CONTROL_CODE_SETUP_MENU         ] = "SETUP_MENU";
    mCECKeyNames[CEC_USER_CONTROL_CODE_CONTENTS_MENU      ] = "CONTENTS_MENU";
    mCECKeyNames[CEC_USER_CONTROL_CODE_NUMBER0            ] = "NUMBER0";
    mCECKeyNames[CEC_USER_CONTROL_CODE_NUMBER1            ] = "NUMBER1";
    mCECKeyNames[CEC_USER_CONTROL_CODE_NUMBER2            ] = "NUMBER2";
    mCECKeyNames[CEC_USER_CONTROL_CODE_NUMBER3            ] = "NUMBER3";
    mCECKeyNames[CEC_USER_CONTROL_CODE_NUMBER4            ] = "NUMBER4";
    mCECKeyNames[CEC_USER_CONTROL_CODE_NUMBER5            ] = "NUMBER5";
    mCECKeyNames[CEC_USER_CONTROL_CODE_NUMBER6            ] = "NUMBER6";
    mCECKeyNames[CEC_USER_CONTROL_CODE_NUMBER7            ] = "NUMBER7";
    mCECKeyNames[CEC_USER_CONTROL_CODE_NUMBER8            ] = "NUMBER8";
    mCECKeyNames[CEC_USER_CONTROL_CODE_NUMBER9            ] = "NUMBER9";
    mCECKeyNames[CEC_USER_CONTROL_CODE_ENTER              ] = "ENTER";
    mCECKeyNames[CEC_USER_CONTROL_CODE_CLEAR              ] = "CLEAR";
    mCECKeyNames[CEC_USER_CONTROL_CODE_CHANNEL_UP         ] = "CHANNEL_UP";
    mCECKeyNames[CEC_USER_CONTROL_CODE_CHANNEL_DOWN       ] = "CHANNEL_DOWN";
    mCECKeyNames[CEC_USER_CONTROL_CODE_PREVIOUS_CHANNEL   ] = "PREVIOUS_CHANNEL";
    mCECKeyNames[CEC_USER_CONTROL_CODE_SOUND_SELECT       ] = "SOUND_SELECT";
    mCECKeyNames[CEC_USER_CONTROL_CODE_DISPLAY_INFORMATION] = "DISPLAY_INFORMATION";
    mCECKeyNames[CEC_USER_CONTROL_CODE_PAGE_UP            ] = "PAGE_UP";
    mCECKeyNames[CEC_USER_CONTROL_CODE_PAGE_DOWN          ] = "PAGE_DOWN";
    mCECKeyNames[CEC_USER_CONTROL_CODE_POWER              ] = "POWER";
    mCECKeyNames[CEC_USER_CONTROL_CODE_VOLUME_UP          ] = "VOLUME_UP";
    mCECKeyNames[CEC_USER_CONTROL_CODE_VOLUME_DOWN        ] = "VOLUME_DOWN";
    mCECKeyNames[CEC_USER_CONTROL_CODE_MUTE               ] = "MUTE";
    mCECKeyNames[CEC_USER_CONTROL_CODE_PLAY               ] = "PLAY";
    mCECKeyNames[CEC_USER_CONTROL_CODE_STOP               ] = "STOP";
    mCECKeyNames[CEC_USER_CONTROL_CODE_PAUSE              ] = "PAUSE";
    mCECKeyNames[CEC_USER_CONTROL_CODE_RECORD             ] = "RECORD";
    mCECKeyNames[CEC_USER_CONTROL_CODE_REWIND             ] = "REWIND";
    mCECKeyNames[CEC_USER_CONTROL_CODE_FAST_FORWARD       ] = "FAST_FORWARD";
    mCECKeyNames[CEC_USER_CONTROL_CODE_FORWARD            ] = "FORWARD";
    mCECKeyNames[CEC_USER_CONTROL_CODE_BACKWARD           ] = "BACKWARD";
    mCECKeyNames[CEC_USER_CONTROL_CODE_SUB_PICTURE        ] = "SUB_PICTURE";
    mCECKeyNames[CEC_USER_CONTROL_CODE_F1_BLUE            ] = "F1_BLUE";
    mCECKeyNames[CEC_USER_CONTROL_CODE_F2_RED             ] = "F2_RED";
    mCECKeyNames[CEC_USER_CONTROL_CODE_F3_GREEN           ] = "F3_GREEN";
    mCECKeyNames[CEC_USER_CONTROL_CODE_F4_YELLOW          ] = "F4_YELLOW";
    mCECKeyNames[CEC_USER_CONTROL_CODE_AN_RETURN          ] = "AN_RETURN";
    mCECKeyNames[CEC_USER_CONTROL_CODE_EXIT               ] = "EXIT";
    mCECKeyNames[CEC_USER_CONTROL_CODE_VIDEO_ON_DEMAND             ] = "VIDEO_ON_DEMAND";
    mCECKeyNames[CEC_USER_CONTROL_CODE_ELECTRONIC_PROGRAM_GUIDE    ] = "ELECTRONIC_PROGRAM_GUIDE";
    mCECKeyNames[CEC_USER_CONTROL_CODE_TIMER_PROGRAMMING           ] = "TIMER_PROGRAMMING";
    mCECKeyNames[CEC_USER_CONTROL_CODE_INITIAL_CONFIGURATION       ] = "INITIAL_CONFIGURATION";
    mCECKeyNames[CEC_USER_CONTROL_CODE_PLAY_FUNCTION               ] = "PLAY_FUNCTION";
    mCECKeyNames[CEC_USER_CONTROL_CODE_PAUSE_PLAY_FUNCTION         ] = "PAUSE_PLAY_FUNCTION";
    mCECKeyNames[CEC_USER_CONTROL_CODE_RECORD_FUNCTION             ] = "RECORD_FUNCTION";
    mCECKeyNames[CEC_USER_CONTROL_CODE_PAUSE_RECORD_FUNCTION       ] = "PAUSE_RECORD_FUNCTION";
    mCECKeyNames[CEC_USER_CONTROL_CODE_STOP_FUNCTION               ] = "STOP_FUNCTION";
    mCECKeyNames[CEC_USER_CONTROL_CODE_MUTE_FUNCTION               ] = "MUTE_FUNCTION";
    mCECKeyNames[CEC_USER_CONTROL_CODE_RESTORE_VOLUME_FUNCTION     ] = "RESTORE_VOLUME_FUNCTION";
    mCECKeyNames[CEC_USER_CONTROL_CODE_TUNE_FUNCTION               ] = "TUNE_FUNCTION";
    mCECKeyNames[CEC_USER_CONTROL_CODE_SELECT_MEDIA_FUNCTION       ] = "SELECT_MEDIA_FUNCTION";
    mCECKeyNames[CEC_USER_CONTROL_CODE_SELECT_AV_INPUT_FUNCTION    ] = "SELECT_AV_INPUT_FUNCTION";
    mCECKeyNames[CEC_USER_CONTROL_CODE_SELECT_AUDIO_INPUT_FUNCTION ] = "SELECT_AUDIO_INPUT_FUNCTION";
    mCECKeyNames[CEC_USER_CONTROL_CODE_POWER_TOGGLE_FUNCTION       ] = "POWER_TOGGLE_FUNCTION";
    mCECKeyNames[CEC_USER_CONTROL_CODE_POWER_OFF_FUNCTION          ] = "POWER_OFF_FUNCTION";
    mCECKeyNames[CEC_USER_CONTROL_CODE_POWER_ON_FUNCTION           ] = "POWER_ON_FUNCTION";
    mCECKeyNames[CEC_USER_CONTROL_CODE_F5                          ] = "F5";
    mCECKeyNames[CEC_USER_CONTROL_CODE_DATA                        ] = "DATA";
    mCECKeyNames[CEC_USER_CONTROL_CODE_STOP_RECORD                 ] = "STOP_RECORD";
    mCECKeyNames[CEC_USER_CONTROL_CODE_PAUSE_RECORD                ] = "PAUSE_RECORD";
    mCECKeyNames[CEC_USER_CONTROL_CODE_ANGLE                       ] = "ANGLE";
    mCECKeyNames[CEC_USER_CONTROL_CODE_EJECT                       ] = "EJECT";
    mCECKeyNames[CEC_USER_CONTROL_CODE_FAVORITE_MENU               ] = "FAVORITE_MENU";
    mCECKeyNames[CEC_USER_CONTROL_CODE_DOT                         ] = "DOT";
    mCECKeyNames[CEC_USER_CONTROL_CODE_NEXT_FAVORITE               ] = "NEXT_FAVORITE";
    mCECKeyNames[CEC_USER_CONTROL_CODE_INPUT_SELECT                ] = "INPUT_SELECT";
    mCECKeyNames[CEC_USER_CONTROL_CODE_HELP                        ] = "HELP";
    mCECKeyNames[CEC_USER_CONTROL_CODE_AN_CHANNELS_LIST            ] = "AN_CHANNELS_LIST";
    Dsyslog("Load keymap");
    InitCECKeyFromDefault(DEFAULTKEYMAP);
    InitVDRKeyFromDefault(DEFAULTKEYMAP);
    SetActiveKeymaps(DEFAULTKEYMAP, DEFAULTKEYMAP);
}

/*
 * List known keymaps. Output is suitable for using with SVDRP
 */
cString cCECkeymaps::ListKeymaps()
{
    cString s = "Keymaps CEC->VDR";
    for (map<string, cVDRKeyMap>::iterator i = mVDRKeyMap.begin();
         i != mVDRKeyMap.end(); ++i) {
        s = cString::sprintf("%s\n  %s", *s, i->first.c_str());
    }
    s = cString::sprintf("%s\nKeymaps VDR->CEC", *s);
    for (map<string, cCECKeyMap>::iterator i = mCECKeyMap.begin();
         i != mCECKeyMap.end(); ++i) {
        s = cString::sprintf("%s\n  %s", *s, i->first.c_str());
    }
    return s;
}

/*
 * List known CEC keycodes. Output is suitable for using with SVDRP
 */
cString cCECkeymaps::ListKeycodes()
{
    cString s = "CEC Keycodes";
    for (int i = 0; i <= CEC_USER_CONTROL_CODE_MAX; i++) {
        if (mCECKeyNames[i] != NULL) {
            s = cString::sprintf("%s\n%02x %s", *s, i, mCECKeyNames[i]);
        }
    }
    return s;
}

/*
 * List CEC Keymap. Output is suitable for using with SVDRP
 */
cString cCECkeymaps::ListCECKeyMap(const string &id)
{
    cCECKeyMap m;
    cString s = "CEC KEYMAP ";
    s = cString::sprintf("%s %s", *s, id.c_str());
    try {
        m = mCECKeyMap.at(id);
    }
    catch (const std::out_of_range& oor) {
        s = cString::sprintf("%s\n   Keymap not found", *s);
        return s;
    }
    for (int i = 0; i <= CEC_USER_CONTROL_CODE_MAX; i++) {
        if (mCECKeyNames[i] != NULL) {
            s = cString::sprintf("%s\n<key code=\"%s\">", *s, mCECKeyNames[i]);
            cKeyList l = m.at(i);
            for (cKeyListIterator it = l.begin(); it != l.end(); it++) {
                s = cString::sprintf("%s\n  <value>%s</value>", *s, cKey::ToString(*it));
            }
            s = cString::sprintf("%s\n</key>", *s);
        }
    }
    return s;
}

/*
 * List VDR Keymap. Output is suitable for using with SVDRP
 */
cString cCECkeymaps::ListVDRKeyMap(const string &id)
{
    cVDRKeyMap m;
    cString s = "VDR KEYMAP ";
    s = cString::sprintf("%s %s", *s, id.c_str());
    try {
        m = mVDRKeyMap.at(id);
    }
    catch (const std::out_of_range& oor) {
        s = cString::sprintf("%s\n   Keymap not found", *s);
        return s;
    }
    for (int i = 0; i < kNone; i++) {
        s = cString::sprintf("%s\n<key code=\"%s\">", *s, cKey::ToString((eKeys)i));
        cCECList  l = m.at((eKeys)i);
        for (cCECListIterator it = l.begin(); it != l.end(); it++) {
            s = cString::sprintf("%s\n  <value>%s</value>", *s, mCECKeyNames[*it]);
        }
        s = cString::sprintf("%s\n</key>", *s);
    }
    return s;
}

/*
 * Convert a String containing a CEC key name to a cec_user_control_code
 */
cec_user_control_code cCECkeymaps::StringToCEC(const string &s)
{
    const char *str = s.c_str();
    for (int i = 0; i <= CEC_USER_CONTROL_CODE_MAX; i++) {
        if (mCECKeyNames[i] != NULL) {
            if (strcasecmp(str, mCECKeyNames[i]) == 0) {
                return (cec_user_control_code)i;
            }
        }
    }
    return CEC_USER_CONTROL_CODE_UNKNOWN;
}

/*
 * Convert a CEC Key to a key list of VDR keys.
 */
cKeyList cCECkeymaps::CECtoVDRKey(cec_user_control_code code)
{
    try {
        return mActiveCecKeyMap.at(code);
    }
    catch (const std::out_of_range& oor) { }
    cKeyList empty;
    return empty; // Empty list
}

/*
 * Convert a VDR Key to a list of CEC keys.
 */
cCECList cCECkeymaps::VDRtoCECKey(eKeys key)
{
    try {
         return mActiveVdrKeyMap.at(key);
    }
    catch (const std::out_of_range& oor) { }
    cCECList empty;
    return empty;
}

/*
 * Helper function which retrieves the CEC key which
 * exactly matches the vdr key.
 */
cec_user_control_code cCECkeymaps::getFirstCEC(eKeys key)
{
    for (int i = 0; i <= CEC_USER_CONTROL_CODE_MAX; i++) {
        if ((mDefaultKeyMap[i][0] == key) &&
            (mDefaultKeyMap[i][1] == kNone)) {
            return (cec_user_control_code)i;
        }
    }
    return CEC_USER_CONTROL_CODE_UNKNOWN;
}

/*
 * Initialize the default CEC Keymap.
 */
void cCECkeymaps::InitCECKeyFromDefault(string id)
{
    cCECKeyMap map;
    map.resize(CEC_USER_CONTROL_CODE_MAX + 2);
    for (int i = 0; i <= CEC_USER_CONTROL_CODE_MAX; i++) {
        map[i].clear();
        if (mDefaultKeyMap[i][0] != kNone) {
            map[i].push_back(mDefaultKeyMap[i][0]);
        }
        if (mDefaultKeyMap[i][1] != kNone) {
            map[i].push_back(mDefaultKeyMap[i][1]);
        }
    }
    // Empty list
    map[CEC_USER_CONTROL_CODE_MAX+1].clear();
    mCECKeyMap.insert(std::pair<string, cCECKeyMap>(id, map));
}

void cCECkeymaps::ClearCECKey(string id, cec_user_control_code k)
{
    mCECKeyMap.at(id).at(k).clear();
}

void cCECkeymaps::AddCECKey(string id, cec_user_control_code k, eKeys c)
{
    mCECKeyMap.at(id).at(k).push_back(c);
}

/*
 * Initialize the default VDR Keymap.
 */
void cCECkeymaps::InitVDRKeyFromDefault(string id)
{
    cVDRKeyMap map;
    cec_user_control_code ceckey;
    map.resize(kNone);
    for (int i = 0; i < kNone; i++) {
        map[i].clear();
        ceckey = getFirstCEC((eKeys)i);
        if (ceckey != CEC_USER_CONTROL_CODE_UNKNOWN) {
            map[i].push_back(ceckey);
        }
    }
    mVDRKeyMap.insert(std::pair<string, cVDRKeyMap>(id, map));
}

void cCECkeymaps::ClearVDRKey(string id, eKeys k)
{
    mVDRKeyMap.at(id).at(k).clear();
}

void cCECkeymaps::AddVDRKey(string id, eKeys k, cec_user_control_code c)
{
    mVDRKeyMap.at(id).at(k).push_back(c);
}

void cCECkeymaps::SetActiveKeymaps(const string &vdrkeymapid,
                                   const string &ceckeymapid)
{
    mActiveVdrKeyMap = mVDRKeyMap.at(vdrkeymapid);
    mActiveCecKeyMap = mCECKeyMap.at(ceckeymapid);
}
