/*
 *
 * CECRemote PlugIn for VDR
 *
 * Copyright (C) 2015-2016 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 * This class implements a data storage for CEC commands.
 */

#ifndef _CCECCMD_H_
#define _CCECCMD_H_

using namespace CEC;

namespace cecplugin {
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
    CEC_EXECTOGGLE,
    CEC_TEXTVIEWON,
    CEC_RECONNECT,
    CEC_CONNECT,
    CEC_DISCONNECT,
    CEC_COMMAND
} CECCommand;

class cCmd;

typedef std::list<cCmd> cCmdQueue;
typedef cCmdQueue::const_iterator cCmdQueueIterator;

class cCmd {
private:
    static int serial;
    static cMutex mSerialMutex;

public:
    CECCommand mCmd;
    int mVal;
    cCECDevice mDevice;
    std::string mExec;
    int mSerial;
    cCmdQueue mPoweron;
    cCmdQueue mPoweroff;
    cec_opcode mCecOpcode;
    cec_logical_address mCecLogicalAddress;

    cCmd() : mCmd(CEC_INVALID), mVal(0), mSerial(-1),
            mCecOpcode(CEC_OPCODE_NONE),
            mCecLogicalAddress(CECDEVICE_UNKNOWN) {};
    cCmd(CECCommand cmd, int val = -1,
            cCECDevice *dev = NULL, std::string exec="");
    cCmd(CECCommand cmd, const cCECDevice dev,
            const cCmdQueue poweron, const cCmdQueue poweroff);
    cCmd(CECCommand cmd, cec_opcode opcode, cec_logical_address logicaladdress);

    int getSerial(void);

    cCmd& operator=(const cCmd &c) {
        mCmd = c.mCmd;
        mVal = c.mVal;
        mDevice = c.mDevice;
        mExec = c.mExec;
        mSerial = c.mSerial;
        mPoweron = c.mPoweron;
        mPoweroff = c.mPoweroff;
        mCecOpcode = c.mCecOpcode;
        mCecLogicalAddress = c.mCecLogicalAddress;
        return *this;
    }
};

} // namespace cecplugin

#endif /* PLUGINS_SRC_VDR_PLUGIN_CECREMOTE_CCECCMD_H_ */
