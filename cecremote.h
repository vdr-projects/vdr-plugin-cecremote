/*
 * cecremote.h
 *
 *  Created on: 25.12.2014
 *      Author: uli
 */

#ifndef CECREMOTE_H_
#define CECREMOTE_H_

#include <vdr/plugin.h>
#include <vdr/remote.h>
#include <vdr/thread.h>
#include <vdr/keys.h>
#include <cectypes.h>
#include <cec.h>
#include <stdint.h>
#include <queue>
#include <list>
#include <vector>

#define MAX_CEC_ADAPTERS 10

using namespace CEC;

typedef enum {
    CEC_INVALID = -1,
    CEC_TIMEOUT = 0,
    CEC_KEYRPRESS,
    CEC_MAKEACTIVE,
    CEC_MAKEINACTIVE,
    CEC_POWERON,
    CEC_POWEROFF,
    CEC_VDRKEYPRESS
} CECCommand;

class cCECCmd {
public:
    cCECCmd() : mCmd(CEC_INVALID), mVal(0) {};
    cCECCmd(CECCommand cmd, int val = -1, cec_logical_address adress = CECDEVICE_UNKNOWN) {
        mCmd = cmd;
        mVal = val;
        mAddress = adress;
    }

    CECCommand mCmd;
    int mVal;
    cec_logical_address mAddress;

    cCECCmd &operator=(const cCECCmd &c) {
        mCmd = c.mCmd;
        mVal = c.mVal;
        mAddress = c.mAddress;
        return *this;
    }
};

typedef std::queue<cCECCmd> cCmdQueue;
typedef std::list<eKeys> cKeyList;
typedef cKeyList::const_iterator cKeyListIterator;

typedef std::vector<cKeyList> cVdrKeyMap;

class cCECRemote : public cRemote, private cThread {
public:
    cCECRemote(int loglevel);
    ~cCECRemote();
    bool Initialize(void);
    void PushCmd(const cCECCmd &cmd);
    int getCECLogLevel() {return mCECLogLevel;}
    ICECAdapter            *mCECAdapter;
private:
    int                    mCECLogLevel;
    uint8_t                mDevicesFound;
    libcec_configuration   mCECConfig;
    ICECCallbacks          mCECCallbacks;
    cec_adapter_descriptor mCECAdapterDescription[MAX_CEC_ADAPTERS];
    cMutex                 mQueueMutex;
    cCondWait              mQueueWait;
    cCmdQueue              mQueue;
    cVdrKeyMap             mKeyMap;

    void Action(void);

    cCECCmd WaitCmd();
    cKeyList &CECtoVDRKey(cec_user_control_code code);
    cec_user_control_code VDRtoCECKey(eKeys key);
    bool TextViewOn(cec_logical_address address);
};

#endif /* CECREMOTE_H_ */
