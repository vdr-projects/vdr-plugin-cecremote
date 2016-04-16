/*
 * cCECCmd.h
 *
 *  Created on: 09.01.2016
 *      Author: uli
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

class cCECCmd;

typedef std::list<cCECCmd> cCmdQueue;
typedef cCmdQueue::const_iterator cCmdQueueIterator;

class cCECCmd {
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

    cCECCmd() : mCmd(CEC_INVALID), mVal(0), mSerial(-1),
            mCecOpcode(CEC_OPCODE_NONE),
            mCecLogicalAddress(CECDEVICE_UNKNOWN) {};
    cCECCmd(CECCommand cmd, int val = -1,
            cCECDevice *dev = NULL, std::string exec="");
    cCECCmd(CECCommand cmd, const cCECDevice dev,
            const cCmdQueue poweron, const cCmdQueue poweroff);
    cCECCmd(CECCommand cmd, cec_opcode opcode, cec_logical_address logicaladdress);

    int getSerial(void);

    cCECCmd& operator=(const cCECCmd &c) {
        mCmd = c.mCmd;
        mVal = c.mVal;
        mDevice = c.mDevice;
        mExec = c.mExec;
        mSerial = c.mSerial;
        mPoweron = c.mPoweron;
        mPoweroff = c.mPoweroff;
        return *this;
    }
};

} // namespace cecplugin

#endif /* PLUGINS_SRC_VDR_PLUGIN_CECREMOTE_CCECCMD_H_ */