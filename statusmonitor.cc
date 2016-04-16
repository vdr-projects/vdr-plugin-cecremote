/*
 * CECRemote PlugIn for VDR
 *
 * Copyright (C) 2015-2016 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 * This class implements the status monitor for channel switch information.
 */

#include "statusmonitor.h"
#include "ceclog.h"

namespace cecplugin {

cStatusMonitor::cStatusMonitor(cPluginCecremote *plugin) : mMonitorStatus(UNKNOWN) {
    mPlugin = plugin;
}

cStatusMonitor::~cStatusMonitor() {

}

void cStatusMonitor::ChannelSwitch(const cDevice *Device, int ChannelNumber,
                                      bool LiveView)
{
    char l = 'f';
    if (LiveView) {
        l = 't';
    }
    if (Device->IsPrimaryDevice()) {
        Dsyslog("Primary device, Channel Switch %d %c", ChannelNumber,l);
#if (APIVERSNUM >= 20301)
        LOCK_CHANNELS_READ;
        const cChannel* channel = Channels->GetByNumber(ChannelNumber);
#else
        const cChannel* channel = Channels.GetByNumber(ChannelNumber);
#endif
        if (channel != NULL) {
            if (channel->Vpid() == 0) {
                Dsyslog("  Radio : %s", channel->Name());
                if (mMonitorStatus != RADIO) {
                    // Ignore first switch, this is covered by <onstart>
                    if (mMonitorStatus != UNKNOWN) {
                        mPlugin->PushCmdQueue(mPlugin->mConfigFileParser.
                                                mGlobalOptions.mOnSwitchToRadio);
                    }
                    mMonitorStatus = RADIO;
                }
            }
            else {
                Dsyslog("  TV    : %s", channel->Name());
                if (mMonitorStatus != TV) {
                    // Ignore first switch, this is covered by <onstart>
                    if (mMonitorStatus != UNKNOWN) {
                        mPlugin->PushCmdQueue(mPlugin->mConfigFileParser.
                                                mGlobalOptions.mOnSwitchToTV);
                    }
                    mMonitorStatus = TV;
                }
            }
        }
    }
    else {
        Dsyslog("Not primary device, Channel Switch %d %c", ChannelNumber, l);
    }
}

void cStatusMonitor::Replaying(const cControl *Control, const char *Name,
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

} // namespace cecplugin
