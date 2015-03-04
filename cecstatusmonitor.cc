/*
 * CECRemote PlugIn for VDR
 *
 * Copyright (C) 2015 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 * This class implements the status monitor for channel switch information.
 */

#include "cecstatusmonitor.h"

cCECStatusMonitor::cCECStatusMonitor() {
    // TODO Auto-generated constructor stub

}

cCECStatusMonitor::~cCECStatusMonitor() {
    // TODO Auto-generated destructor stub
}

void cCECStatusMonitor::ChannelSwitch(const cDevice *Device, int ChannelNumber,
                                      bool LiveView)
{
    char l = 'f';
    if (LiveView) {
        l = 't';
    }
    if (Device->IsPrimaryDevice()) {
        Dsyslog("Primary device Channel Switch %d %c", ChannelNumber,l);
        const cChannel* channel = Channels.GetByNumber(ChannelNumber);
        if (channel != NULL) {
            if (channel->Vpid() == 0) {
                Dsyslog("  Radio : %s", channel->Name());
            }
            else {
                Dsyslog("  TV    : %s", channel->Name());
            }
        }
    }
    else {
        Dsyslog("Not primary device Channel Switch %d %c", ChannelNumber,l);
    }
}

