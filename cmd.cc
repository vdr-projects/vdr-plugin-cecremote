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

#include "cecremote.h"
#include "cecremoteplugin.h"
#include "cmd.h"

namespace cecplugin {
int cCmd::serial = 1;
cMutex cCmd::mSerialMutex;

cCmd::cCmd(CECCommand cmd, int val, cCECDevice *dev, std::string exec) {
    mCmd = cmd;
    mVal = val;
    if (dev != NULL) {
        mDevice = *dev;
    }
    mExec = exec;
    mSerial =-1;
    mCecOpcode = CEC_OPCODE_NONE;
    mCecLogicalAddress = CECDEVICE_UNKNOWN;
}

cCmd::cCmd(CECCommand cmd, const cCECDevice dev,
        const cCmdQueue poweron, const cCmdQueue poweroff) {
    mCmd = cmd;
    mVal = -1;
    mDevice = dev;
    mSerial = -1;
    mPoweron = poweron;
    mPoweroff = poweroff;
    mCecOpcode = CEC_OPCODE_NONE;
    mCecLogicalAddress = CECDEVICE_UNKNOWN;
}

cCmd::cCmd(CECCommand cmd, cec_opcode opcode,
        cec_logical_address logicaladdress) {
    mCmd = cmd;
    mVal = -1;
    mSerial = -1;
    mCecOpcode = opcode;
    mCecLogicalAddress = logicaladdress;
}

int cCmd::getSerial(void) {
    int ret;
    mSerialMutex.Lock();
    serial++;
    if (serial > 10000) {
        serial = 1;
    }
    ret = serial;
    mSerialMutex.Unlock();
    return ret;
}

} // namespace cecplugin
