/*
 * CECRemote PlugIn for VDR
 *
 * Copyright (C) 2015-2016 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 * This class implements the keymap handling class
 *
 */


#ifndef _CECKEYMAPS_H_
#define _CECKEYMAPS_H_

#include <vdr/plugin.h>
#include <map>
#include <string>
#include <vector>
#include <list>
#include <cectypes.h>
#include <cec.h>

using namespace CEC;
namespace cecplugin {

// Key map CEC key->VDR keys
typedef std::list<eKeys> cKeyList;
typedef std::vector<cKeyList> cKeyMap;
typedef cKeyList::const_iterator cKeyListIterator;

// Key map VDR key->CEC keys
typedef std::list<cec_user_control_code>cCECList;
typedef std::vector<cCECList> cVDRKeyMap;
typedef cCECList::const_iterator cCECListIterator;

class cKeyMaps {
private:
    eKeys mDefaultKeyMap[CEC_USER_CONTROL_CODE_MAX+1][2];
    const char *mCECKeyNames[CEC_USER_CONTROL_CODE_MAX+1];

    std::map<std::string, cVDRKeyMap> mVDRKeyMap;
    std::map<std::string, cKeyMap> mCECKeyMap;
    cVDRKeyMap mActiveVdrKeyMap;
    cKeyMap mActiveCecKeyMap;

    cec_user_control_code getFirstCEC(eKeys key);
public:
    cKeyMaps();
    void InitVDRKeyFromDefault(std::string id);
    void InitCECKeyFromDefault(std::string id);
    void ClearCECKey(std::string id, cec_user_control_code k);
    void ClearVDRKey(std::string id, eKeys k);
    void AddCECKey(std::string id, cec_user_control_code k, eKeys c);
    void AddVDRKey(std::string id, eKeys k, cec_user_control_code c);
    cKeyList CECtoVDRKey(cec_user_control_code code);
    cCECList VDRtoCECKey(eKeys key);
    cec_user_control_code StringToCEC(const std::string &s);
    void SetActiveKeymaps(const std::string &vdrkeymapid,
                          const std::string &ceckeymapid);

    // Functions to dump information via SVDRP.
    cString ListKeymaps();
    cString ListKeycodes();
    cString ListCECKeyMap(const std::string &id);
    cString ListVDRKeyMap(const std::string &id);

    static const char *DEFAULTKEYMAP;
};

} // namespace cecplugin

#endif /* _CECKEYMAPS_H_ */
