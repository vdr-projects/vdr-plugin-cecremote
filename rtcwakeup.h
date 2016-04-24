/*
 * CECRemote PlugIn for VDR
 *
 * Copyright (C) 2016 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 * rtcwakeup.h: Static helper class to detect if the VDR was started via
 *              the RTC.
 */

#ifndef PLUGINS_SRC_VDR_PLUGIN_CECREMOTE_RTCWAKEUP_H_
#define PLUGINS_SRC_VDR_PLUGIN_CECREMOTE_RTCWAKEUP_H_

namespace cecplugin {

class rtcwakeup {
protected:

    static const char *RESET_RTCALARM;
    static const char *RTC_DEVICE;
    static const char *ALARM_KEY;
    static void trim(char *str);
    static void reset_alarm(void);

public:
    typedef enum {
        RTC_WAKEUP, OTHER_WAKEUP, RTC_ERROR
    } RTC_WAKEUP_TYPE;

    /*
     * Check if the VDR was started via the RTC.
     * Returns :  rtcwakeup::RTC_WAKEUP   if wakeup from RTC was detected.
     *            rtcwakeup::OTHER_WAKEUP if no wakeup from the RTC was detected.
     *            rtcwakeup::RTC_ERROR    it was not possible to detect the startup
     *                                    reason, e.g. by problems accessing the
     *                                    /proc or /sys filesystems.
     */
    static RTC_WAKEUP_TYPE check(void);
};

} /* namespace cecplugin */

#endif /* PLUGINS_SRC_VDR_PLUGIN_CECREMOTE_RTCWAKEUP_H_ */
