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
#include "ceclog.h"

cCECStatusMonitor::cCECStatusMonitor(cPluginCecremote *plugin) : mMonitorStatus(UNKNOWN) {
    mPlugin = plugin;
}

cCECStatusMonitor::~cCECStatusMonitor() {

}

void cCECStatusMonitor::ChannelSwitch(const cDevice *Device, int ChannelNumber,
                                      bool LiveView)
{
    char l = 'f';
    if (LiveView) {
        l = 't';
    }
    if (Device->IsPrimaryDevice()) {
        Dsyslog("Primary device, Channel Switch %d %c", ChannelNumber,l);
        const cChannel* channel = Channels.GetByNumber(ChannelNumber);
        if (channel != NULL) {
            if (channel->Vpid() == 0) {
                Dsyslog("  Radio : %s", channel->Name());
                if (mMonitorStatus != RADIO) {
                    mMonitorStatus = RADIO;
                    mPlugin->PushCmdQueue(mPlugin->mConfigFileParser.mGlobalOptions.mOnSwitchToRadio);
                }
            }
            else {
                Dsyslog("  TV    : %s", channel->Name());
                if (mMonitorStatus != TV) {
                    mMonitorStatus = TV;
                    mPlugin->PushCmdQueue(mPlugin->mConfigFileParser.mGlobalOptions.mOnSwitchToTV);
                }
            }
        }
    }
    else {
        Dsyslog("Not primary device, Channel Switch %d %c", ChannelNumber, l);
    }
}

void cCECStatusMonitor::Replaying(const cControl *Control, const char *Name,
                                  const char *FileName, bool On)
{
    Dsyslog("Replaying");
    if (On) {
        if (mMonitorStatus != REPLAYING) {
            mMonitorStatus = REPLAYING;
            mPlugin->PushCmdQueue(mPlugin->mConfigFileParser.mGlobalOptions.mOnSwitchToReplay);
        }
    }
}
