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
#include <string>

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
    CEC_VDRKEYPRESS,
    CEC_EXECSHELL
} CECCommand;

class cCECCmd {
public:
    cCECCmd() : mCmd(CEC_INVALID), mVal(0), mAddress(CECDEVICE_UNKNOWN) {};
    cCECCmd(CECCommand cmd, int val = -1,
            cec_logical_address adress = CECDEVICE_UNKNOWN, std::string exec="") {
        mCmd = cmd;
        mVal = val;
        mAddress = adress;
        mExec = exec;
    }

    CECCommand mCmd;
    int mVal;
    cec_logical_address mAddress;
    std::string mExec;

    cCECCmd &operator=(const cCECCmd &c) {
        mCmd = c.mCmd;
        mVal = c.mVal;
        mAddress = c.mAddress;
        mExec = c.mExec;
        return *this;
    }
};

typedef std::list<cCECCmd> cCmdQueue;
typedef cCmdQueue::const_iterator cCmdQueueIterator;
typedef std::list<eKeys> cKeyList;
typedef cKeyList::const_iterator cKeyListIterator;

typedef std::vector<cKeyList> cVdrKeyMap;

class cCECRemote : public cRemote, private cThread {
public:
    cCECRemote(int loglevel, const cCmdQueue &onStart,
               const cCmdQueue &onStop);
    ~cCECRemote();
    bool Initialize(void);
    void PushCmd(const cCECCmd &cmd);
    void ExecCmd(const cCmdQueue &cmdList);
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

    cCmdQueue mOnStart;
    cCmdQueue mOnStop;
};

#endif /* CECREMOTE_H_ */
