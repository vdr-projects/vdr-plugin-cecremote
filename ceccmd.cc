/*
 *
 * CECRemote PlugIn for VDR
 *
 * Copyright (C) 2015 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 * This class implements the remote receiving and processing the CEC commands.
 */

#include "cecremote.h"
#include "cecremoteplugin.h"
#include "ceccmd.h"

int cCECCmd::serial = 1;
cMutex cCECCmd::mSerialMutex;

cCECCmd::cCECCmd(CECCommand cmd, int val, cCECDevice *dev, std::string exec) {
    mCmd = cmd;
    mVal = val;
    if (dev != NULL) {
        mDevice = *dev;
    }
    mExec = exec;
    mSerial =-1;
}
cCECCmd::cCECCmd(CECCommand cmd, const cCECDevice dev,
        const cCmdQueue poweron, const cCmdQueue poweroff) {
    mCmd = cmd;
    mVal = -1;
    mDevice = dev;
    mSerial = -1;
    mPoweron = poweron;
    mPoweroff = poweroff;
}

int cCECCmd::getSerial(void) {
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

