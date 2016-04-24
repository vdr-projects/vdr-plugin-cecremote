/*
 * CECRemote PlugIn for VDR
 *
 * Copyright (C) 2016 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 * rtcwakeup.cc: Static helper class to detect if the VDR was started via
 *               the RTC.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "cecremote.h"
#include "rtcwakeup.h"
#include "ceclog.h"

namespace cecplugin {

using namespace std;

const char *rtcwakeup::RESET_RTCALARM = "/sys/class/rtc/rtc0/wakealarm";
const char *rtcwakeup::RTC_DEVICE = "/proc/driver/rtc";
const char *rtcwakeup::ALARM_KEY = "alarm_IRQ";

void rtcwakeup::trim(char *str) {
    char *end = str + strlen(str) - 1;
    while (isspace(*end) && (end > str)) {
        *end = '\0';
        end--;
    }
}

/*
 * Reset the alarm state
 */
void rtcwakeup::reset_alarm(void) {
    FILE *fp = fopen(RESET_RTCALARM, "w");
    if (fp == NULL) {
        Esyslog("Can not open %s: %s\n", RESET_RTCALARM, strerror(errno));
        return;
    }
    fputc('0', fp);
    fclose(fp);
}

/*
 * Check if the VDR was started via the RTC.
 * Returns :  rtcwakeup::RTC_WAKEUP   if wakeup from RTC was detected.
 *            rtcwakeup::OTHER_WAKEUP if no wakeup from the RTC was detected.
 *            rtcwakeup::RTC_ERROR    it was not possible to detect the startup
 *                                    reason, e.g. by problems accessing the
 *                                    /proc or /sys filesystems.
 */
rtcwakeup::RTC_WAKEUP_TYPE rtcwakeup::check(void) {
    FILE *fp = fopen(RTC_DEVICE, "r");
    char key[128];
    char val[128];

    if (fp == NULL) {
        Esyslog("Can not open %s: %s\n", RTC_DEVICE, strerror(errno));
        return(RTC_ERROR);
    }
    key[0] = 0;
    val[0] = 0;
    while (fscanf(fp, "%127[^:] : %127s\n", key, val) == 2) {
        trim(key);
        Csyslog ("Key = '%s' val = '%s'\n", key, val);
        if (strcmp(key, ALARM_KEY) == 0) {
            fclose(fp);
            if (strcmp(val, "no") == 0) {
                Dsyslog("Not started by the rtc\n");
                return(OTHER_WAKEUP);
            }
            Dsyslog("Started by rtc\n");
            reset_alarm();
            return(RTC_WAKEUP);
        }
    }
    fclose(fp);
    Esyslog("Alarm key %s not found in %s\n", ALARM_KEY, RTC_DEVICE);
    return(RTC_ERROR);
}

} /* namespace cecplugin */
