/*
 * CECRemote PlugIn for VDR
 *
 * Copyright (C) 2015-2016 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
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

#include "keymaps.h"
#include "cmd.h"

namespace cecplugin {

#define MAX_CEC_ADAPTERS 10

class cPluginCecremote;
class cCECGlobalOptions;

class cCECRemote : public cRemote, private cThread {
public:
    cCECRemote(const cCECGlobalOptions &options, cPluginCecremote *plugin);
    ~cCECRemote();
    bool Initialize(void) {return false;};
    void PushCmd(const cCmd &cmd);
    void PushCmdQueue(const cCmdQueue &cmdList);
    void PushWaitCmd(cCmd &cmd, int timeout = 5000);
    int getCECLogLevel(void) {return mCECLogLevel;}
    cString ListDevices(void);
    void Reconnect(void);
    void Stop(void);
    void Startup(void);

    ICECAdapter            *mCECAdapter;
private:
    static const char      *VDRNAME;
    int                    mCECLogLevel;
    int                    mProcessedSerial;
    uint8_t                mDevicesFound;
    uint8_t                mHDMIPort;
    cec_logical_address    mBaseDevice;
    uint32_t               mComboKeyTimeoutMs;
    libcec_configuration   mCECConfig;
    ICECCallbacks          mCECCallbacks;
    cec_adapter_descriptor mCECAdapterDescription[MAX_CEC_ADAPTERS];

    // Queue for normal worker thread
    cMutex                 mWorkerQueueMutex;
    cCondWait              mWorkerQueueWait;
    cCmdQueue              mWorkerQueue;

    // Queue for special commands when shell script is executed
    cMutex                 mExecQueueMutex;
    cCondWait              mExecQueueWait;
    cCmdQueue              mExecQueue;

    cCondWait              mCmdReady;
    deviceTypeList         mDeviceTypes;
    bool                   mShutdownOnStandby;
    bool                   mPowerOffOnStandby;
    bool                   mInExec;
    cPluginCecremote       *mPlugin;

    void Connect(void);
    void Disconnect(void);
    void ActionKeyPress(cCmd &cmd);
    void Action(void);
    void CECCommand(const cCmd &cmd);
    cCmd WaitCmd(int timeout = 5000);
    cCmd WaitExec(pid_t pid);
    void Exec(cCmd &cmd);
    void ExecToggle(cCECDevice dev, const cCmdQueue &poweron,
                    const cCmdQueue &poweroff);
    void WaitForPowerStatus(cec_logical_address addr, cec_power_status newstatus);
    bool TextViewOn(cec_logical_address address);
    cec_logical_address getLogical(cCECDevice &dev);

    cCmdQueue mOnStart;
    cCmdQueue mOnStop;
    cCmdQueue mOnManualStart;
};

} // namespace cecplugin

#endif /* CECREMOTE_H_ */
