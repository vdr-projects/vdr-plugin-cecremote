/*
 * ceclog.h
 *
 *  Created on: 27.12.2014
 *      Author: uli
 */

#ifndef CECLOG_H_
#define CECLOG_H_

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

#endif /* CECLOG_H_ */
