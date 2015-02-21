/*
 * CECRemote PlugIn for VDR
 *
 * Copyright (C) 2015 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 * This class implements the remote receiving and processing the CEC commands.
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
#include <map>
#include <string>

#include "ceckeymaps.h"

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
    CEC_EXECSHELL,
    CEC_TEXTVIEWON
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

class cPluginCecremote;
class cCECGlobalOptions;

class cCECRemote : public cRemote, private cThread {
public:
    cCECRemote(const cCECGlobalOptions &options, cPluginCecremote *plugin);
    ~cCECRemote();
    virtual bool Initialize(void) {return false;};
    void PushCmd(const cCECCmd &cmd);
    void ExecCmd(const cCmdQueue &cmdList);
    void ExecToggle(cec_logical_address addr, const cCmdQueue &poweron,
                    const cCmdQueue &poweroff);
    int getCECLogLevel() {return mCECLogLevel;}
    cString ListDevices();

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
    cPluginCecremote       *mPlugin;

    void Action(void);
    cCECCmd WaitCmd();
    bool TextViewOn(cec_logical_address address);

    cCmdQueue mOnStart;
    cCmdQueue mOnStop;
};

#endif /* CECREMOTE_H_ */
