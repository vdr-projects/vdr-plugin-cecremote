/*
 * CECRemote PlugIn for VDR
 *
 * Copyright (C) 2015 Ulrich Eckhardt <uli-vdr@uli-eckhardt.de>
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 * This class implements a VDR Player which displays a still-picture.
 */

#include "stillpicplayer.h"
#include "ceclog.h"

using namespace std;
using namespace cecplugin;

namespace cecplugin {

cStillPicPlayer::cStillPicPlayer(const cCECMenu &config) :
                pStillBuf(NULL), mStillBufLen(0)
{
    mStillPic = config.mStillPic;
}

cStillPicPlayer::~cStillPicPlayer() {
    if (pStillBuf != NULL) {
        free (pStillBuf);
        pStillBuf = NULL;
    }
}

void cStillPicPlayer::DisplayStillPicture (void)
{
    if (pStillBuf != NULL) {
        DeviceStillPicture((const uchar *)pStillBuf, mStillBufLen);
    }
}

void cStillPicPlayer::LoadStillPicture (const string &FileName)
{
    int fd;
    ssize_t len;
    int size = CDMAXFRAMESIZE;
    cMutexLock MutexLock(&mPlayerMutex);

    if (pStillBuf != NULL) {
        free (pStillBuf);
    }
    pStillBuf = NULL;
    mStillBufLen = 0;
    fd = open(FileName.c_str(), O_RDONLY);
    if (fd < 0) {
        string errtxt = tr("Can not open still picture: ");
        errtxt += FileName;
        Skins.QueueMessage(mtError, errtxt.c_str());
        Esyslog("%s %d Can not open still picture %s",
                __FILE__, __LINE__, FileName.c_str());
        return;
    }

    pStillBuf = (uchar *)malloc (CDMAXFRAMESIZE * 2);
    if (pStillBuf == NULL) {
        Esyslog("%s %d Out of memory", __FILE__, __LINE__);
        close(fd);
        return;
    }
    do {
        len = read(fd, &pStillBuf[mStillBufLen], CDMAXFRAMESIZE);
        if (len < 0) {
            Esyslog ("%s %d read error %d", __FILE__, __LINE__, errno);
            close(fd);
            free (pStillBuf);
            pStillBuf = NULL;
            mStillBufLen = 0;
            return;
        }
        if (len > 0) {
            mStillBufLen += len;
            if (mStillBufLen >= size) {
                size += CDMAXFRAMESIZE;
                pStillBuf = (uchar *) realloc(pStillBuf, size + CDMAXFRAMESIZE);
                if (pStillBuf == NULL) {
                    close(fd);
                    mStillBufLen = 0;
                    Esyslog("%s %d Out of memory", __FILE__, __LINE__);
                    return;
                }
            }
        }
    } while (len > 0);
    close(fd);
    DisplayStillPicture();
}

void cStillPicPlayer::Activate(bool On) {
    if (On) {
        LoadStillPicture(mStillPic);
    }
}

} // namespace cecplugin

