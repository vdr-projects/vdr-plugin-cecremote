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

int cCECCmd::getSerial(void) {
    int ret;
    mSerialMutex.Lock();
    serial++;
    ret = serial;
    mSerialMutex.Unlock();
    return ret;
}

