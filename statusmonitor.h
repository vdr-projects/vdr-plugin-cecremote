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


#ifndef _CECSTATUSMONITOR_H_
#define _CECSTATUSMONITOR_H_

#include <vdr/plugin.h>
#include <vdr/status.h>

#include "cecremoteplugin.h"

namespace cecplugin {

class cStatusMonitor : public cStatus {
protected:
    typedef enum {
        UNKNOWN,
        RADIO,
        TV,
        REPLAYING
    } MonitorStatus;

    virtual void TimerChange(const cTimer *Timer, eTimerChange Change) {};
    virtual void ChannelSwitch(const cDevice *Device, int ChannelNumber, bool LiveView);

    virtual void Recording(const cDevice *Device, const char *Name, const char *FileName, bool On) {};
    virtual void Replaying(const cControl *Control, const char *Name,
                           const char *FileName, bool On);
    virtual void SetVolume(int Volume, bool Absolute) {};
    virtual void SetAudioTrack(int Index, const char * const *Tracks) {};
    virtual void SetAudioChannel(int AudioChannel) {};
    virtual void SetSubtitleTrack(int Index, const char * const *Tracks) {};
    virtual void OsdClear(void) {};
    virtual void OsdTitle(const char *Title) {};
    virtual void OsdStatusMessage(const char *Message) {};
    virtual void OsdHelpKeys(const char *Red, const char *Green,
                             const char *Yellow, const char *Blue) {};
    virtual void OsdItem(const char *Text, int Index) {};
    virtual void OsdCurrentItem(const char *Text) {};
    virtual void OsdTextItem(const char *Text, bool Scroll) {};
    virtual void OsdChannel(const char *Text) {};
    virtual void OsdProgramme(time_t PresentTime, const char *PresentTitle,
                              const char *PresentSubtitle, time_t FollowingTime,
                              const char *FollowingTitle, const char *FollowingSubtitle) {};

    MonitorStatus mMonitorStatus;
    cPluginCecremote *mPlugin;
public:
    cStatusMonitor(cPluginCecremote *plugin);
    virtual ~cStatusMonitor();
};

} // namespace cecplugin

#endif /*_CECSTATUSMONITOR_H_ */
