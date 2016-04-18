/*
 * CECRemote PlugIn for VDR
 *
 * Copyright (C) 2014, 2015 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 * This class implements logging functions
 *
 */

#ifndef CECLOG_H_
#define CECLOG_H_

namespace cecplugin {

#define MAXSYSLOGBUF 256

static void ceclogmsg (int severity, const char *format, ...)
{
    if (SysLogLevel > severity) {
        char fmt[MAXSYSLOGBUF];

        int facility_priority = LOG_ERR;
        if (severity == 1) {
            facility_priority = LOG_WARNING;
        }
        else if (severity == 2) {
            facility_priority = LOG_DEBUG;
        }

        snprintf(fmt, sizeof(fmt), "[cecremote] %s", format);
        va_list ap;
        va_start(ap, format);
        vsyslog(facility_priority ,fmt, ap);
        va_end(ap);
    }
}

#define Esyslog(a...) ceclogmsg(0, a)
#define Isyslog(a...) ceclogmsg(1, a)
#define Dsyslog(a...) ceclogmsg(2, a)

#ifdef VERBOSEDEBUG
#define Csyslog(a...) ceclogmsg(2, a)
#else
#define Csyslog(a...)
#endif

} // namespace cecplugin

#endif /* CECLOG_H_ */
