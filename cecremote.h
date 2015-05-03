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

// Class for storing information of devices (<device> tag)
class cCECDevice {
public:
    uint16_t mPhysicalAddress;
    cec_logical_address mLogicalAddressDefined;
    cec_logical_address mLogicalAddressUsed;

    cCECDevice() : mPhysicalAddress(0),
                   mLogicalAddressDefined(CECDEVICE_UNKNOWN),
                   mLogicalAddressUsed(CECDEVICE_UNKNOWN) {};

    cCECDevice &operator=(const cCECDevice &c) {
        mPhysicalAddress = c.mPhysicalAddress;
        mLogicalAddressDefined = c.mLogicalAddressDefined;
        mLogicalAddressUsed = c.mLogicalAddressUsed;
        return *this;
    }
};

typedef std::list<cec_device_type> deviceTypeList;
typedef deviceTypeList::const_iterator deviceTypeListIterator;

typedef enum {
    CEC_INVALID = -1,
    CEC_EXIT = 0,
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
    cCECCmd() : mCmd(CEC_INVALID), mVal(0) {};
    cCECCmd(CECCommand cmd, int val = -1,
            cCECDevice *dev = NULL, std::string exec="") {
        mCmd = cmd;
        mVal = val;
        if (dev != NULL) {
            mDevice = *dev;
        }
        mExec = exec;
    }

    CECCommand mCmd;
    int mVal;
    cCECDevice mDevice;
    std::string mExec;

    cCECCmd &operator=(const cCECCmd &c) {
        mCmd = c.mCmd;
        mVal = c.mVal;
        mDevice = c.mDevice;
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
    bool Initialize(void) {return false;};
    void PushCmd(const cCECCmd &cmd);
    void PushCmdQueue(const cCmdQueue &cmdList);
    void ExecToggle(cCECDevice dev, const cCmdQueue &poweron,
                    const cCmdQueue &poweroff);
    int getCECLogLevel() {return mCECLogLevel;}
    cString ListDevices();
    void Connect();
    void Disconnect();
    void Stop();
    ICECAdapter            *mCECAdapter;
private:
    static const char      *VDRNAME;
    int                    mCECLogLevel;
    uint8_t                mDevicesFound;
    uint8_t                mHDMIPort;
    uint32_t               mComboKeyTimeoutMs;
    libcec_configuration   mCECConfig;
    ICECCallbacks          mCECCallbacks;
    cec_adapter_descriptor mCECAdapterDescription[MAX_CEC_ADAPTERS];
    cMutex                 mWorkerQueueMutex;
    cCondWait              mWorkerQueueWait;
    cCmdQueue              mWorkerQueue;
    deviceTypeList         mDeviceTypes;
    cPluginCecremote       *mPlugin;

    void ActionKeyPress(cCECCmd &cmd);
    void Action(void);
    cCECCmd WaitCmd();
    void WaitForPowerStatus(cec_logical_address addr, cec_power_status newstatus);
    bool TextViewOn(cec_logical_address address);
    cec_logical_address getLogical(cCECDevice &dev);

    cCmdQueue mOnStart;
    cCmdQueue mOnStop;
    cCmdQueue mOnManualStart;
};

#endif /* CECREMOTE_H_ */
